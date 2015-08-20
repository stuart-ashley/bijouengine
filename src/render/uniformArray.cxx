#include "uniformArray.h"

#include "shader.h"
#include "texture.h"

#include <iostream>

using namespace render;

/*
 *
 */
UniformArray::UniformArray(const UniformArray & other) :
		uniforms(other.uniforms) {
}

/*
 *
 */
void UniformArray::add(const Uniform & uniform) {
	auto result = uniforms.emplace(uniform.getUID(), uniform);
	if (result.second == false) {
		result.first->second = uniform;
	}
}

/*
 *
 */
void UniformArray::add(const UniformArray & other) {
	for (const auto & entry : other.uniforms) {
		uniforms.insert(entry);
	}
}

/*
 *
 */
bool UniformArray::contains(size_t uid) const {
	return uniforms.find(uid) != uniforms.end();
}

/*
 *
 */
const Uniform & UniformArray::get(size_t uid) const {
	return uniforms.at(uid);
}

/*
 *
 */
void UniformArray::bind(const std::shared_ptr<Shader> & shader) {
	int nBound = 0;
	int nUniforms = shader->getNumUserUniforms();

	for (const auto & e : uniforms) {
		const auto & uniform = e.second;
		auto loc = shader->getUniformLocation(uniform.getUID(),
				uniform.getType());

		if (loc == -1) {
			continue;
		}

		nBound++;

		if (uniform.getTexture()) {
			auto texId = shader->getTextureId(uniform.getUID());
			uniform.bindTexture(texId);
			continue;
		}

		uniform.bind(loc);
	}

	if (nBound < nUniforms) {
		shader->dump();
		dump();
		std::unordered_set<std::string> names(
				shader->getUserUniformNames().begin(),
				shader->getUserUniformNames().end());
		for (const auto & e : uniforms) {
			names.erase(Uniform::getName(e.first));
		}
		std::cerr << "ERROR: Missing " << std::to_string(nUniforms - nBound)
				<< " of " << std::to_string(shader->getNumUniforms())
				<< " Uniforms [";
		std::string separator = " ";
		for (const auto & name : names) {
			std::cerr << separator << name;
			separator = ", ";
		}
		std::cerr << " ]";
	}
}

/**
 * Get the names of textures in the array that are used by the given shader.
 *
 * @param shader  shader to check usage against
 *
 * @return        names of textures used by shader
 */
std::unordered_set<int> UniformArray::getTextureDependencies(
		const std::shared_ptr<Shader> & shader) const {
	// using a map so that duplicate uniform can overwrite previous one
	std::unordered_map<int, int> map;

	for (const auto & e : uniforms) {
		const auto & uniform = e.second;
		if (uniform.isTexture() == false) {
			continue;
		}
		if (shader->getUniformLocation(uniform.getUID(), uniform.getType())
				== -1) {
			continue;
		}
		auto result = map.emplace(uniform.getUID(),
				uniform.getTexture()->getId());
		if (result.second == false) {
			result.first->second = uniform.getTexture()->getId();
		}
	}

	// copy dependencies from map
	std::unordered_set<int> deps;
	for (const auto & e : map) {
		deps.emplace(e.second);
	}
	return deps;
}

/**
 * check if the contents of this array is the same as that of an other array
 *
 * @param other  array to check against
 *
 * @return       false if array is equal, true otherwise
 */
bool UniformArray::operator!=(const UniformArray & other) const {
	return uniforms != other.uniforms;
}

/**
 * check if the contents of this array is the same as that of an other array
 *
 * @param other  array to check against
 *
 * @return       true if array is equal, false otherwise
 */
bool UniformArray::operator==(const UniformArray & other) const {
	return uniforms == other.uniforms;
}

/**
 * dump uniform array
 */
void UniformArray::dump() const {
	for (const auto & entry : uniforms) {
		entry.second.dump();
	}
}
