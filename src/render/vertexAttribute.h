#pragma once

#include <memory>
#include <string>
#include <vector>

class Color;
class Normal;
class Vec2;
class Vec3;

namespace render {
	class VertexBuffer;

	class VertexAttribute {
	public:
		enum class Type {
			FLOAT, SHORT, USHORT, BYTE
		};

		/**
		 *
		 * @param buffer
		 * @param name
		 * @param size
		 * @param type
		 * @param offset
		 * @param stride
		 */
		VertexAttribute(const std::shared_ptr<VertexBuffer> & buffer,
				const std::string & name, int size, Type type, int offset,
				int stride);

		/**
		 *
		 * @param name
		 * @param array
		 */
		VertexAttribute(const std::string & name,
				const std::vector<Vec2> & array);

		/**
		 *
		 * @param name
		 * @param array
		 */
		VertexAttribute(const std::string & name,
				const std::vector<Vec3> & array);

		/**
		 *
		 * @param name
		 * @param array
		 */
		VertexAttribute(const std::string & name,
				const std::vector<Normal> & array);

		/**
		 *
		 * @param name
		 * @param array
		 */
		VertexAttribute(const std::string & name,
				const std::vector<Color> array);

		/**
		 *
		 * @param name
		 * @param size
		 * @param array
		 */
		VertexAttribute(const std::string & name, int size,
				const std::vector<float> array);

		/**
		 *
		 * @param idx
		 */
		void bind(int idx);

		/**
		 *
		 * @return
		 */
		const std::string & getName() const;

		/**
		 *
		 * @param other
		 * @return
		 */
		bool operator==(const VertexAttribute & other) const;

		/**
		 *
		 * @return
		 */
		bool validate() const;

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

