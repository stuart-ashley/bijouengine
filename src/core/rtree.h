#pragma once

#include "boundingBox.h"

#include <memory>

template<unsigned SIZE, typename TYPE>
class RTree {
private:
	struct Branch;

	struct Node {
	private:
		BoundingBox m_bounds;
		std::unique_ptr<Branch> m_branch;
		std::unique_ptr<TYPE> m_leaf;
	public:
		Node() :
						m_bounds(BoundingBox::empty()),
						m_branch(nullptr),
						m_leaf(nullptr) {
		}

		Node(const Node & node) :
						m_bounds(node.m_bounds),
						m_branch(
								node.m_branch == nullptr ?
										nullptr : new Branch(*node.m_branch)),
						m_leaf(
								node.m_leaf == nullptr ?
										nullptr : new TYPE(*node.m_leaf)) {
		}

		Node(const BoundingBox & bounds, TYPE leaf) :
				m_bounds(bounds), m_branch(nullptr), m_leaf(new TYPE(leaf)) {
		}

		~Node() {
		}

		Node & operator=(const Node & node) {
			m_bounds = node.m_bounds;
			m_branch =
					node.m_branch == nullptr ?
							nullptr :
							std::unique_ptr<Branch>(new Branch(*node.m_branch));
			m_leaf =
					node.m_leaf == nullptr ?
							nullptr :
							std::unique_ptr<TYPE>(new TYPE(*node.m_leaf));
			return *this;
		}

		const BoundingBox & getBounds() const {
			return m_bounds;
		}

		bool isLeaf() const {
			return m_leaf != nullptr;
		}

		void insert(const Node & insertNode) {
			m_bounds += insertNode.getBounds();
			if (isLeaf()) {
				m_branch = std::unique_ptr<Branch>(
						new Branch(*this, insertNode));
				m_leaf = nullptr;
			} else {
				getBranch().insert(insertNode);
			}
		}

		TYPE & getLeaf() const {
			return *m_leaf;
		}

		Branch & getBranch() const {
			return *m_branch;
		}
	};

	struct Branch {
	private:
		Node m_node[SIZE];
		unsigned m_nNodes;
	public:
		Branch() :
				m_nNodes(0) {
		}

		Branch(const Node & n0, const Node & n1) :
				m_nNodes(2) {
			m_node[0] = n0;
			m_node[1] = n1;
		}

		~Branch() {
		}

		bool empty() const {
			return m_nNodes == 0;
		}

		void insert(const Node & insertNode) {
			if (m_nNodes < SIZE) {
				m_node[m_nNodes] = insertNode;
				++m_nNodes;
				return;
			}

			// find most appropriate ( defined as the one with the least dimensions
			// summed after insert ) node to insert into
			int nodeIndex = 0;
			BoundingBox bounds(insertNode.getBounds() + m_node[0].getBounds());
			auto size1 = bounds.getSurfaceArea();

			for (unsigned i = 1; i < m_nNodes; ++i) {
				bounds = insertNode.getBounds() + m_node[i].getBounds();
				auto size = bounds.getSurfaceArea();

				if (size > size1) {
					continue;
				}

				nodeIndex = i;
				size1 = size;
			}

			m_node[nodeIndex].insert(insertNode);
		}

		template<typename FUNC>
		void intersect(const BoundingBox & bounds, FUNC & fn) const {
			for (unsigned i = 0; i < m_nNodes; ++i) {
				if (!m_node[i].getBounds().intersects(bounds)) {
					continue;
				}
				if (m_node[i].isLeaf()) {
					fn(m_node[i].getLeaf());
				} else {
					m_node[i].getBranch().intersect(bounds, fn);
				}
			}
		}

		template<typename FUNC>
		void rayIntersect(const Ray & ray, double tmin, double tmax,
				FUNC & fn) const {
			for (unsigned i = 0; i < m_nNodes; ++i) {
				double t0 = tmin;
				double t1 = tmax;
				if (!m_node[i].getBounds().rayIntersect(ray, t0, t1)) {
					continue;
				}
				if (m_node[i].isLeaf()) {
					fn(m_node[i].getLeaf());
				} else {
					m_node[i].getBranch().rayIntersect(ray, t0, t1, fn);
				}
			}
		}
	};

	BoundingBox m_bounds;
	Branch m_branch;

public:
	RTree() :
			m_bounds(BoundingBox::empty()) {
	}

	~RTree() {
	}

	bool empty() const {
		return m_branch.empty();
	}

	// insert new leaf into tree
	void insert(const BoundingBox & bounds, TYPE const & data) {
		m_bounds = empty() ? bounds : (m_bounds + bounds);
		m_branch.insert(Node(bounds, data));
	}

	// call function 'fn' passing parameter of type TYPE if it was inserted
	// with bounds intersecting 'bounds' for all those that do
	template<typename FUNC>
	void intersect(const BoundingBox & bounds, FUNC & fn) const {
		if (empty()) {
			return;
		}
		if (!bounds.intersects(m_bounds)) {
			return;
		}

		m_branch.intersect(bounds, fn);
	}

	template<typename FUNC>
	void rayIntersect(const Ray & ray, double t0, double t1, FUNC & fn) const {
		if (empty()) {
			return;
		}
		if (m_bounds.rayIntersect(ray, t0, t1)) {
			m_branch.rayIntersect(ray, t0, t1, fn);
		}
	}
};
