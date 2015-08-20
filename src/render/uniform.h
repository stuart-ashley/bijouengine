#pragma once

#include "../scripting/scriptObject.h"

#include <array>
#include <memory>
#include <string>

class Animation;
class Color;
class Mat3;
class Mat4;
class Normal;
class Vec2;
class Vec3;

namespace render {
	class Texture;

	class Uniform final: public ScriptObject {
	public:

		/**
		 * construct uniform from animation
		 *
		 * @param animation  animation
		 * @param frame      frame
		 */
		Uniform(const Animation & animation, float frame);

		/**
		 * construct uniform from color
		 *
		 * @param uid    uniform id
		 * @param color  color
		 */
		Uniform(size_t uid, const Color & color);

		/**
		 * construct uniform from float
		 *
		 * @param uid  uniform id
		 * @param f    float value
		 */
		Uniform(size_t uid, float f);

		/**
		 * construct uniform from two float
		 *
		 * @param uid  uniform id
		 * @param f0   1st float value
		 * @param f1   2nd float value
		 */
		Uniform(size_t uid, float f0, float f1);

		/**
		 * construct uniform from three floats
		 *
		 * @param uid  uniform id
		 * @param f0   1st float value
		 * @param f1   2nd float value
		 * @param f2   3rd float value
		 */
		Uniform(size_t uid, float f0, float f1, float f2);

		/**
		 * construct uniform from four floats
		 *
		 * @param uid  uniform id
		 * @param f0   1st float value
		 * @param f1   2nd float value
		 * @param f2   3rd float value
		 * @param f3   4th float value
		 */
		Uniform(size_t uid, float f0, float f1, float f2, float f3);

		/**
		 * construct uniform from 3x3 matrix
		 *
		 * @param uid     uniform id
		 * @param matrix  matrix to create uniform from
		 */
		Uniform(size_t uid, const Mat3 & matrix);

		/**
		 * construct uniform from 4x4 matrix
		 *
		 * @param uid     uniform id
		 * @param matrix  matrix to create uniform from
		 */
		Uniform(size_t uid, const Mat4 & matrix);

		/**
		 * construct uniform from normal
		 *
		 * @param uid  uniform id
		 * @param n    normal
		 */
		Uniform(size_t uid, const Normal & n);

		/**
		 *
		 * @param uid
		 * @param texture
		 */
		Uniform(size_t uid, const std::shared_ptr<Texture> & texture);

		/**
		 * construct uniform from vec2
		 *
		 * @param uid  uniform id
		 * @param v    vec2
		 */
		Uniform(size_t uid, const Vec2 & v);

		/**
		 * construct uniform from vec3
		 *
		 * @param uid  uniform id
		 * @param v    vec3
		 */
		Uniform(size_t uid, const Vec3 & v);

		/**
		 * construct uniform from spherical harmonic coefficients
		 *
		 * @param uid     uniform id
		 * @param coeffs  spherical harmonic coefficients
		 */
		Uniform(size_t uid, const std::array<float, 27> & coeffs);

		/**
		 * copy constructor
		 *
		 * @param other  uniform to copy
		 */
		inline Uniform(const Uniform &) = default;

		/**
		 * default destructor
		 */
		inline virtual ~Uniform() = default;

		/**
		 * bind uniform to location
		 *
		 * @param location  bind location
		 */
		void bind(int location) const;

		/**
		 * bind uniform texture
		 *
		 * @param texid  texture id to bind to
		 */
		void bindTexture(size_t texid) const;

		/**
		 * dump contents of this uniform
		 */
		void dump() const;

		/**
		 * get texture of this uniform, presupposes a texture uniform
		 *
		 * @return  texture for this uniform
		 */
		const std::shared_ptr<Texture> & getTexture() const;

		/**
		 * get type of uniform
		 *
		 * @return  uniform type
		 */
		int getType() const;

		/**
		 * get unique id associated with uniform name
		 *
		 * @return  unique id for uniform
		 */
		size_t getUID() const;

		/**
		 * check if uniform is a texture
		 *
		 * @return  true if uniform is a texture, false otherwise
		 */
		bool isTexture() const;

		/**
		 * is this uniform equal to an other uniform
		 *
		 * @param other  uniform to check against
		 *
		 * @return       true if uniforms are equal, false otherwise
		 */
		bool operator==(const Uniform & other) const;

		/**
		 * get script object factory for Uniform
		 *
		 * @return  Uniform factory
		 */
		static const ScriptObjectPtr & getFactory();

		/**
		 * get name from unique id
		 *
		 * @param uid  id to get name for
		 *
		 * @return     name associated with id
		 */
		static const std::string & getName(size_t uid);

		/**
		 * get unique id for given name
		 *
		 * @param name  name to get unique id for
		 *
		 * @return      unique id for name
		 *
		 */
		static size_t getUID(const std::string & name);

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

