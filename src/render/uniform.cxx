#include "uniform.h"

#include "glDebug.h"
#include "texture.h"
#include "textureManager.h"

#include "../core/animation.h"
#include "../core/bezier.h"
#include "../core/bone.h"
#include "../core/color.h"
#include "../core/mat3.h"
#include "../core/mat4.h"
#include "../core/nameToIdMap.h"
#include "../core/vec2.h"
#include "../core/vec3.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"
#include "../scripting/scriptExecutionException.h"

#include <array>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>

using namespace render;

namespace {
	NameToIdMap & getNameToIdMap() {
		static NameToIdMap instance;
		return instance;
	}

	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			scriptExecutionAssert(nArgs >= 2, "Require at least 2 arguments");

			auto name = getArg<String>("string", stack, 1);

			auto id = Uniform::getUID(name.getValue());

			if (nArgs == 2) {
				auto arg = stack.top();
				stack.pop();

				if (typeid(*arg) == typeid(Mat3)) {
					auto mat = std::static_pointer_cast<Mat3>(arg);
					stack.push(std::make_shared<Uniform>(id, *mat));
				} else if (typeid(*arg) == typeid(Texture)) {
					auto tex = std::static_pointer_cast<Texture>(arg);
					stack.push(std::make_shared<Uniform>(id, tex));
				} else if (typeid(*arg) == typeid(Vec3)) {
					auto vec = std::static_pointer_cast<Vec3>(arg);
					stack.push(std::make_shared<Uniform>(id, *vec));
				} else if (typeid(*arg) == typeid(Real)) {
					auto num = std::static_pointer_cast<Real>(arg);
					stack.push(std::make_shared<Uniform>(id, num->getFloat()));
				} else {
					scriptExecutionAssert(false,
							"Require Mat3, Texture, Vec3 or number for argument 2");
				}
			} else if (nArgs == 4) {
				float x = static_cast<float>(getNumericArg(stack, 2));
				float y = static_cast<float>(getNumericArg(stack, 3));
				float z = static_cast<float>(getNumericArg(stack, 4));
				stack.push(std::make_shared<Uniform>(id, x, y, z));
			} else if (nArgs == 5) {
				float x = static_cast<float>(getNumericArg(stack, 2));
				float y = static_cast<float>(getNumericArg(stack, 3));
				float z = static_cast<float>(getNumericArg(stack, 4));
				float w = static_cast<float>(getNumericArg(stack, 5));
				stack.push(std::make_shared<Uniform>(id, x, y, z, w));
			} else if (nArgs == 28) {
				std::array<float, 27> coeffs;
				for (int i = 0; i < 27; ++i) {
					coeffs[i] = static_cast<float>(getNumericArg(stack, i + 2));
				}
				stack.push(std::make_shared<Uniform>(id, coeffs));
			} else {
				scriptExecutionAssert(false, "Unsupported number of arguments");
			}
		}
	};

	int texType(const std::shared_ptr<Texture> & texture) {
		if (texture->isCubeMap()) {
			return GL_SAMPLER_CUBE;
		} else if (texture->isDepth()) {
			return GL_SAMPLER_2D_SHADOW;
		} else if (texture->is3d()) {
			return GL_SAMPLER_3D;
		} else {
			return GL_SAMPLER_2D;
		}
	}
}

struct Uniform::impl {
	size_t uid;
	int type;
	std::vector<int> ints;
	std::vector<float> floats;
	std::shared_ptr<Texture> texture;

	impl(const Animation & animation, float frame) :
			uid(Uniform::getUID(animation.getName())) {

		const auto & curves = animation.getBone();
		switch (curves.size()) {
		case 1: {
			floats.emplace_back(curves.get(0).getY(frame));
			type = GL_FLOAT;
			break;
		}
		case 2: {
			floats.emplace_back(curves.get(0).getY(frame));
			floats.emplace_back(curves.get(1).getY(frame));
			type = GL_FLOAT_VEC2;
			break;
		}
		case 3: {
			floats.emplace_back(curves.get(0).getY(frame));
			floats.emplace_back(curves.get(1).getY(frame));
			floats.emplace_back(curves.get(2).getY(frame));
			type = GL_FLOAT_VEC3;
			break;
		}
		case 4: {
			floats.emplace_back(curves.get(0).getY(frame));
			floats.emplace_back(curves.get(1).getY(frame));
			floats.emplace_back(curves.get(2).getY(frame));
			floats.emplace_back(curves.get(3).getY(frame));
			type = GL_FLOAT_VEC4;
			break;
		}
		default:
			assert(false);
		}
	}

	impl(size_t uid, const Color & color) :
			uid(uid), type(GL_FLOAT_VEC4), floats(std::vector<float>{
					color.getR(), color.getG(),	color.getB(), color.getA() }) {
	}

	impl(size_t uid, float f) :
			uid(uid), type(GL_FLOAT), floats(std::vector<float>{ f }) {
	}

	impl(size_t uid, float f0, float f1) :
			uid(uid), type(GL_FLOAT_VEC2), floats(std::vector<float>{ f0, f1 }) {
	}

	impl(size_t uid, float f0, float f1, float f2) :
			uid(uid), type(GL_FLOAT_VEC3), floats(std::vector<float>{ f0, f1, f2 }) {
	}

	impl(size_t uid, float f0, float f1, float f2, float f3) :
			uid(uid), type(GL_FLOAT_VEC4), floats(std::vector<float>{ f0, f1, f2, f3 }) {
	}

	impl(size_t uid, const Mat3 & matrix) :
					uid(uid),
					type(GL_FLOAT_MAT3),
					floats(std::vector<float>{
							matrix.get(0, 0), matrix.get(1, 0), matrix.get(2, 0),
							matrix.get(0, 1), matrix.get(1, 1), matrix.get(2, 1),
							matrix.get(0, 2), matrix.get(1, 2),	matrix.get(2, 2) }) {
	}

	impl(size_t uid, const Mat4 & matrix) :
					uid(uid),
					type(GL_FLOAT_MAT4),
					floats(std::vector<float>{
							matrix.get(0, 0), matrix.get(1, 0), matrix.get(2, 0), matrix.get(3, 0),
							matrix.get(0, 1), matrix.get(1, 1), matrix.get(2, 1), matrix.get(3, 1),
							matrix.get(0, 2), matrix.get(1, 2), matrix.get(2, 2), matrix.get(3, 2),
							matrix.get(0, 3), matrix.get(1, 3), matrix.get(2, 3), matrix.get(3, 3) }) {
	}

	impl(size_t uid, const Normal & n) :
					uid(uid),
					type(GL_FLOAT_VEC3),
					floats(std::vector<float>{ n.getX(), n.getY(), n.getZ() }) {
	}

	impl(size_t uid, const std::shared_ptr<Texture> & texture) :
			uid(uid), type(texType(texture)), texture(texture) {
	}

	impl(size_t uid, const Vec2 & v) :
			uid(uid), type(GL_FLOAT_VEC2), floats(std::vector<float>{
					static_cast<float>(v.getX()), static_cast<float>(v.getY()) }) {
	}

	impl(size_t uid, const Vec3 & v) :
			uid(uid), type(GL_FLOAT_VEC3), floats(std::vector<float>{
					static_cast<float>(v.getX()), static_cast<float>(v.getY()),
					static_cast<float>(v.getZ()) }) {
	}

	impl(size_t uid, const std::array<float, 27> & coeffs) :
			uid(uid), type(GL_FLOAT_VEC3), floats(coeffs.begin(), coeffs.end()) {
	}
};

/**
 * construct uniform from animation
 *
 * @param animation  animation
 * @param frame      frame
 */
Uniform::Uniform(const Animation & animation, float frame) :
		pimpl(std::make_shared<impl>(animation, frame)) {
}

/**
 * construct uniform from color
 *
 * @param uid    uniform id
 * @param color  color
 */
Uniform::Uniform(size_t uid, const Color & color) :
		pimpl(std::make_shared<impl>(uid, color)) {
}

/**
 * construct uniform from float
 *
 * @param uid  uniform id
 * @param f    float value
 */
Uniform::Uniform(size_t uid, float f) :
		pimpl(std::make_shared<impl>(uid, f)) {
}

/**
 * construct uniform from two float
 *
 * @param uid  uniform id
 * @param f0   1st float value
 * @param f1   2nd float value
 */
Uniform::Uniform(size_t uid, float f0, float f1) :
		pimpl(std::make_shared<impl>(uid, f0, f1)) {
}

/**
 * construct uniform from three floats
 *
 * @param uid  uniform id
 * @param f0   1st float value
 * @param f1   2nd float value
 * @param f2   3rd float value
 */
Uniform::Uniform(size_t uid, float f0, float f1, float f2) :
		pimpl(std::make_shared<impl>(uid,f0, f1, f2)) {
}

/**
 * construct uniform from four floats
 *
 * @param uid  uniform id
 * @param f0   1st float value
 * @param f1   2nd float value
 * @param f2   3rd float value
 * @param f3   4th float value
 */
Uniform::Uniform(size_t uid, float f0, float f1, float f2, float f3) :
		pimpl(std::make_shared<impl>(uid,f0, f1, f2, f3)) {
}

/**
 * construct uniform from 3x3 matrix
 *
 * @param uid     uniform id
 * @param matrix  matrix to create uniform from
 */
Uniform::Uniform(size_t uid, const Mat3 & matrix) :
		pimpl(std::make_shared<impl>(uid, matrix)) {
}

/**
 * construct uniform from 4x4 matrix
 *
 * @param uid     uniform id
 * @param matrix  matrix to create uniform from
 */
Uniform::Uniform(size_t uid, const Mat4 & matrix) :
		pimpl(std::make_shared<impl>(uid, matrix)) {
}

/**
 * construct uniform from normal
 *
 * @param uid  uniform id
 * @param n    normal
 */
Uniform::Uniform(size_t uid, const Normal & n) :
		pimpl(std::make_shared<impl>(uid, n)) {
}

/**
 *
 * @param uid
 * @param texture
 */
Uniform::Uniform(size_t uid, const std::shared_ptr<Texture> & texture) :
		pimpl(std::make_shared<impl>(uid, texture)) {
}

/**
 * construct uniform from vec2
 *
 * @param uid  uniform id
 * @param v    vec2
 */
Uniform::Uniform(size_t uid, const Vec2 & v) :
		pimpl(std::make_shared<impl>(uid, v)) {
}

/**
 * construct uniform from vec3
 *
 * @param uid  uniform id
 * @param v    vec3
 */
Uniform::Uniform(size_t uid, const Vec3 & v) :
		pimpl(std::make_shared<impl>(uid, v)) {
}

/**
 * construct uniform from spherical harmonic coefficients
 *
 * @param uid     uniform id
 * @param coeffs  spherical harmonic coefficients
 */
Uniform::Uniform(size_t uid, const std::array<float, 27> & coeffs) :
		pimpl(std::make_shared<impl>(uid, coeffs)) {
}

/**
 * bind uniform to location
 *
 * @param location  bind location
 */
void Uniform::bind(int location) const {
	switch (pimpl->type) {
	case GL_INT:
		glUniform1i(location, pimpl->ints[0]);
		break;
	case GL_FLOAT:
		glUniform1f(location, pimpl->floats[0]);
		break;
	case GL_FLOAT_VEC2:
		glUniform2f(location, pimpl->floats[0], pimpl->floats[1]);
		break;
	case GL_FLOAT_VEC3:
		glUniform3fv(location, static_cast<GLsizei>(pimpl->floats.size() / 3),
				pimpl->floats.data());
		break;
	case GL_FLOAT_VEC4:
		glUniform4f(location, pimpl->floats[0], pimpl->floats[1],
				pimpl->floats[2], pimpl->floats[3]);
		break;
	case GL_FLOAT_MAT3:
		glUniformMatrix3fv(location,
				static_cast<GLsizei>(pimpl->floats.size() / 9), false,
				pimpl->floats.data());
		break;
	case GL_FLOAT_MAT4:
		glUniformMatrix4fv(location,
				static_cast<GLsizei>(pimpl->floats.size() / 16), false,
				pimpl->floats.data());
		break;
	default:
		assert(false);
	}
	GlDebug::printOpenGLError();
}
/**
 * bind uniform texture
 *
 * @param texid  texture id to bind to
 */
void Uniform::bindTexture(size_t texid) const {
	if (pimpl->texture->validate() == false) {
		return;
	}
	glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + texid));
	pimpl->texture->bind();
	GlDebug::printOpenGLError();
}

/**
 * dump contents of this uniform
 */
void Uniform::dump() const {
	std::cout << "Uniform( " << getName(pimpl->uid)
			<< ", " + GlDebug::getType(pimpl->type);
	switch (pimpl->type) {
	case GL_INT:
		for (auto i : pimpl->ints) {
			std::cout << ", " << i;
		}
		break;
	case GL_FLOAT:
	case GL_FLOAT_VEC2:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC4:
	case GL_FLOAT_MAT3:
	case GL_FLOAT_MAT4:
		for (auto f : pimpl->floats) {
			std::cout << ", " << f;
		}
		break;
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_2D_SHADOW:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_2D:
		std::cout << ","
				<< TextureManager::getInstance().getName(
						pimpl->texture->getId());
		break;
	default:
		assert(false);
	}
	std::cout << " )" << std::endl;
}

/**
 * get texture of this uniform, presupposes a texture uniform
 *
 * @return  texture for this uniform
 */
const std::shared_ptr<Texture> & Uniform::getTexture() const {
	return pimpl->texture;
}

/**
 * get type of uniform
 *
 * @return  uniform type
 */
int Uniform::getType() const {
	return pimpl->type;
}

/**
 * get unique id associated with uniform name
 *
 * @return  unique id for uniform
 */
size_t Uniform::getUID() const {
	return pimpl->uid;
}

/**
 * check if uniform is a texture
 *
 * @return  true if uniform is a texture, false otherwise
 */
bool Uniform::isTexture() const {
	return pimpl->type == GL_SAMPLER_CUBE || pimpl->type == GL_SAMPLER_2D_SHADOW
			|| pimpl->type == GL_SAMPLER_3D || pimpl->type == GL_SAMPLER_2D;
}

/**
 * is this uniform equal to an other uniform
 *
 * @param other  uniform to check against
 *
 * @return       true if uniforms are equal, false otherwise
 */
bool Uniform::operator==(const Uniform & other) const {
	if (pimpl->uid != other.pimpl->uid || pimpl->type != other.pimpl->type) {
		return false;
	}

	if (pimpl->type == GL_INT) {
		return pimpl->ints == other.pimpl->ints;
	}

	if (isTexture()) {
		return pimpl->texture == other.pimpl->texture;
	}

	return pimpl->floats == other.pimpl->floats;
}

/**
 * get script object factory for Uniform
 *
 * @return  Uniform factory
 */
STATIC const ScriptObjectPtr & Uniform::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}

/**
 * get name from unique id
 *
 * @param uid  id to get name for
 *
 * @return     name associated with id
 */
STATIC const std::string & Uniform::getName(size_t uid) {
	return getNameToIdMap().getName(uid);
}

/**
 * get unique id for given name
 *
 * @param name  name to get unique id for
 *
 * @return      unique id for name
 *
 */
STATIC size_t Uniform::getUID(const std::string & name) {
	if (name.compare(name.length() - 3, name.length(), "[0]") == 0) {
		return getNameToIdMap().getId(name.substr(0, name.length() - 3));
	}
	return getNameToIdMap().getId(name);
}
