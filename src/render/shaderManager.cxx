#include "shaderManager.h"

#include "shader.h"
#include "program.h"

#include "../core/config.h"
#include "../core/loadManager.h"

#include "../render/shaderTag.h"
#include "../render/shaderFlag.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <GL/glew.h>

using namespace render;

namespace {
	struct ProgramCache {
		std::mutex lock;
		std::unordered_map<std::string, std::shared_ptr<Program>> cache;

		std::shared_ptr<Program> get(const std::string & currentDir,
				const std::string & name) {
			std::lock_guard<std::mutex> locker(lock);
			auto canonicalFilename = LoadManager::getInstance()->getName(
					currentDir, name);
			auto it = cache.find(canonicalFilename);
			if (it == cache.end()) {
				auto program = std::make_shared<Program>(currentDir, name);
				cache.emplace(canonicalFilename, program);
				return program;
			} else {
				return it->second;
			}
		}
	};

	ProgramCache & getProgramCache() {
		static ProgramCache cache;
		return cache;
	}

	ShaderTag genShaderTag(const std::string & vf, const std::string & gf,
			const std::string & ff) {
		auto & programCache = getProgramCache();
		const auto & vp = programCache.get("shaders/", vf);
		const auto & gp =
				(gf == "") ? nullptr : programCache.get("shaders/", gf);
		const auto & fp = programCache.get("shaders/", ff);
		ShaderFlags flags;
		return ShaderTag(vp, gp, fp, flags);
	}

	struct Conf {
		bool instancing;
		bool floatTextures;
		GLint maxVertexUniforms;
		bool flipGeneratedFaceNormals;
		bool geometryShaders;

		Conf() :
						instancing(false),
						floatTextures(true),
						maxVertexUniforms(0),
						flipGeneratedFaceNormals(false),
						geometryShaders(true) {
		}

		void init() {
			// set fp texture config
			auto vendor = (const char *) glGetString(GL_VENDOR);
			auto renderer = (const char *) glGetString(GL_RENDERER);
			auto extensions = (const char *) glGetString(GL_EXTENSIONS);

			// float textures
			if (strstr(extensions, "GL_ARB_texture_float") == nullptr) {
				floatTextures = false;
			}
			if (strstr(vendor, "NVIDIA") != nullptr
					&& strstr(renderer, "7950") != nullptr) {
				floatTextures = false;
			}

			// max vertex uniforms
			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniforms);

			// generated face normals
			auto tag = genShaderTag("checkFaceNormal.vp", "",
					"checkFaceNormal.fp");
			auto shader = std::make_shared<Shader>(0, "",
					tag.getVertexProgram(), tag.getGeometryProgram(),
					tag.getFragmentProgram(), 1);

			shader->build();
			shader->bind();

			int posIdx = shader->getAttributeIndex("a_position");
			assert(posIdx >= 0);

			float clipSpaceVerts[] = { -1, -1, 0, -1, 1, 0, 1, 1, 0, 1, -1, 0 };

			glVertexAttribPointer(posIdx, 3, GL_FLOAT, false, 0,
					clipSpaceVerts);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

			shader->unbind();

			float rgba[4];
			glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, rgba);
			glFlush();

			float z = rgba[2];

			flipGeneratedFaceNormals = (z == 0);

			// geometry shaders
			if (strstr(extensions, "GL_EXT_geometry_shader4") == nullptr) {
				geometryShaders = false;
			}
		}

		/**
		 * Turn a set of flags into a string of #define statements
		 *
		 * @param flags
		 *            flags to turn to #define statements
		 * @return String of #define statement
		 */
		std::string getDefs(const ShaderFlags & flags) {
			std::string defs = "#version 120\n";

			if (instancing) {
				defs += "#extension GL_EXT_draw_instanced : enable\n";
			} else {
				defs += "#define gl_InstanceID (0)\n";
			}
			// defs.append("#define DEFERRED\n");

			for (size_t i = 0, n = flags.size(); i < n; ++i) {
				if (flags[i]) {
					defs += "#define " + ShaderFlag::toString(i) + "\n";
				}
			}

			if (floatTextures) {
				defs += "#define FLOAT_TEXTURES\n";
			}

			const auto & depthFormat = Config::getInstance().getString(
					"depthFormat");
			if (depthFormat == "RGBA_DEPTH") {
				defs += "#define RGBA_DEPTH\n";
			}

			if (flipGeneratedFaceNormals) {
				defs += "#define FLIP_GENERATED_FACE_NORMALS\n";
			}

			defs += "#define MAX_INSTANCES ("
					+ std::to_string(getMaxInstances(flags)) + ")\n";

			return defs;
		}

		int getMaxInstances(const ShaderFlags & flags) {
			static auto skinning = ShaderFlag::valueOf("SKINNING");
			if (flags[skinning]) {
				return 1;
			}
			if (instancing == false) {
				return 1;
			}
			int otherUniforms = 150;
			int floatsPerMatrix = 16;
			int numInstanceMatrices = 3;
			return (maxVertexUniforms - otherUniforms)
					/ (floatsPerMatrix * numInstanceMatrices);
		}
	} conf;
}

struct ShaderManager::impl {
	ShaderTag uber;
	ShaderTag shadow;
	ShaderTag irradianceSampler;
	ShaderTag defaultImage;

	/** debug drawing shader */
	std::shared_ptr<Shader> debugShader;
	std::shared_ptr<Shader> debugLinesShader;
	std::shared_ptr<Shader> blurH;
	std::shared_ptr<Shader> blurV;

	std::mutex lock;
	std::unordered_map<ShaderTag, std::shared_ptr<Shader>> shaderMap;
	std::vector<std::shared_ptr<Shader>> shaderList;

	std::unordered_set<ShaderTag> awaitingBuild;

	impl() :
					uber(genShaderTag("uber.vp", "", "uber.fp")),
					shadow(genShaderTag("shadow.vp", "", "shadow.fp")),
					irradianceSampler(
							genShaderTag("uber.vp", "irradianceSampler.gp",
									"uber.fp")),
					defaultImage(genShaderTag("colmap.vp", "", "colmap.fp")) {
		auto debugTag = genShaderTag("debug.vp", "", "debug.fp");
		auto linesTag = genShaderTag("debug.vp", "debugLines.gp",
				"debugLines.fp");
		auto blurHTag = genShaderTag("colmap.vp", "", "blurH.fp");
		auto blurVTag = genShaderTag("colmap.vp", "", "blurV.fp");

		getShader(debugTag);
		if (conf.geometryShaders) {
			getShader(linesTag);
		}
		getShader(blurHTag);
		getShader(blurVTag);

		buildShaders();

		debugShader = getShader(debugTag);
		if (conf.geometryShaders) {
			debugLinesShader = getShader(linesTag);
		}
		blurH = getShader(blurHTag);
		blurV = getShader(blurVTag);
	}

	void buildShaders() {
		std::lock_guard<std::mutex> locker(lock);

		for (const auto & tag : awaitingBuild) {
			auto id = shaderList.size();
			const auto & flags = tag.getActiveFlags();
			auto shader = std::make_shared<Shader>(id, conf.getDefs(flags),
					tag.getVertexProgram(), tag.getGeometryProgram(),
					tag.getFragmentProgram(), conf.getMaxInstances(flags));
			shader->build();
			shaderMap.emplace(tag, shader);
			shaderList.emplace_back(shader);
		}
		awaitingBuild.clear();
	}

	std::shared_ptr<Shader> getShader(const ShaderTag & tag) {
		auto it = shaderMap.find(tag);
		if (it == shaderMap.end()) {
			std::lock_guard<std::mutex> locker(lock);

			if (tag.validate()) {
				awaitingBuild.emplace(tag);
			}

			return nullptr;
		}
		return it->second;
	}

	std::shared_ptr<Shader> getShader(const std::string & vf,
			const std::string & gf, const std::string & ff) {
		auto tag = genShaderTag(vf, gf, ff);
		return getShader(tag);
	}
};

ShaderManager::ShaderManager() :
		pimpl(new impl()) {
}

void ShaderManager::buildShaders() {
	pimpl->buildShaders();
}

const std::shared_ptr<Shader> & ShaderManager::getDebugLinesShader() const {
	if (pimpl->debugLinesShader != nullptr) {
		return pimpl->debugLinesShader;
	} else {
		return pimpl->debugShader;
	}
}

const std::shared_ptr<Shader> & ShaderManager::getDebugShader() const {
	return pimpl->debugShader;
}

const std::shared_ptr<Shader> & ShaderManager::getHorizontalBlurShader() const {
	return pimpl->blurH;
}

const ShaderTag & ShaderManager::getImageTag() const {
	return pimpl->defaultImage;
}

/*
 *
 */
std::shared_ptr<Program> ShaderManager::getProgram(
		const std::string & currentDir, const std::string & name) {
	return getProgramCache().get(currentDir, name);
}

std::shared_ptr<Shader> ShaderManager::getShader(unsigned id) {
	if (id >= pimpl->shaderList.size()) {
		return nullptr;
	}
	return pimpl->shaderList[id];
}

std::shared_ptr<Shader> ShaderManager::getShader(const ShaderTag & tag) {
	return pimpl->getShader(tag);
}

const ShaderTag & ShaderManager::getShadowTag() const {
	return pimpl->shadow;
}

const ShaderTag & ShaderManager::getUberTag() const {
	return pimpl->uber;
}

const std::shared_ptr<Shader> & ShaderManager::getVerticalBlurShader() const {
	return pimpl->blurV;
}

STATIC ShaderManager & ShaderManager::getInstance() {
	// ensure init has been done
	assert(conf.maxVertexUniforms > 0);

	static ShaderManager instance;
	return instance;
}

/*
 *
 */
STATIC void ShaderManager::init() {
	conf.init();
	ShaderManager::getInstance();
}

