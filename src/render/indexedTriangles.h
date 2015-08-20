#pragma once

#include "../core/indexArray.h"

namespace render {
	/**
	 * IndexedTriangles uses pimpl so that the vertex buffer object can be shared
	 */
	class IndexedTriangles final {
	public:

		inline IndexedTriangles(const IndexedTriangles &) = default;

		inline ~IndexedTriangles() = default;

		IndexedTriangles & operator=(const IndexedTriangles & other) = delete;

		IndexedTriangles(const IndexArray & indices);

		/**
		 *
		 */
		void drawElements();

		/**
		 *
		 */
		void drawInstances(size_t nInstances);

		/**
		 *
		 * @return
		 */
		size_t numTriangles() const;

		/**
		 *
		 * @param other
		 * @return
		 */
		bool operator==(const IndexedTriangles & other) const;

		/**
		 *
		 * @return
		 */
		bool validate() const;

	private:
		IndexedTriangles() = delete;

		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

