#pragma once

#include "vertexAttribute.h"

#include <vector>

namespace render {
	class Shader;

	class VertexAttributeArray {
	public:

		/**
		 * Default constructor
		 */
		VertexAttributeArray();

		/**
		 * Copy constructor, does a shallow copy
		 */
		VertexAttributeArray(const VertexAttributeArray & other);

		/**
		 *
		 * @param attribute
		 */
		void add(const VertexAttribute & attribute);

		/**
		 *
		 * @param shader
		 */
		void bind(const std::shared_ptr<Shader> & shader);

		/**
		 *
		 * @param other
		 * @return
		 */
		bool isInstanceable(const VertexAttributeArray & other) const;

		/**
		 *
		 * @return
		 */
		bool validate() const;

	private:
		std::vector<VertexAttribute> attributes;
	};
}

