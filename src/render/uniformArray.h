#pragma once

#include "uniform.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace render {
	class Shader;

	class UniformArray final {
	public:

		/**
		 *
		 */
		inline UniformArray() = default;

		UniformArray(const UniformArray & other);

		/**
		 * default destructor
		 */
		inline ~UniformArray() = default;

		void add(const Uniform & uniform);

		void add(const UniformArray & other);

		void bind(const std::shared_ptr<Shader> & shader);

		bool contains(size_t uid) const;

		/**
		 * dump uniform array
		 */
		void dump() const;

		const Uniform & get(size_t uid) const;

		/**
		 * Get the names of textures in the array that are used by the given shader.
		 *
		 * @param shader  shader to check usage against
		 *
		 * @return        names of textures used by shader
		 */
		std::unordered_set<int> getTextureDependencies(
				const std::shared_ptr<Shader> & shader) const;

		/**
		 * check if the contents of this array is the same as that of an other array
		 *
		 * @param other  array to check against
		 *
		 * @return       false if array is equal, true otherwise
		 */
		bool operator!=(const UniformArray & other) const;

		/**
		 * check if the contents of this array is the same as that of an other array
		 *
		 * @param other  array to check against
		 *
		 * @return       true if array is equal, false otherwise
		 */
		bool operator==(const UniformArray & other) const;

	private:
		std::unordered_map<size_t, Uniform> uniforms;
	};
}
