#include "collisionHierarchy.h"

#include "boundingBox.h"
#include "color.h"
#include "convexHull.h"
#include "debugGeometry.h"
#include "intersection.h"
#include "parallelPlanes.h"
#include "sphere.h"
#include "terrain.h"
#include "transform.h"
#include "triangleList.h"
#include "vec3.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

#include <cassert>

namespace {

	/*
	 *
	 */
	struct Factory: public Executable {
		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			std::shared_ptr<ConvexHull> convexHull;
			std::shared_ptr<Terrain> terrain;
			std::shared_ptr<BoundingBox> box;
			std::shared_ptr<Sphere> sphere;

			std::vector<CollisionHierarchy> collisions;
			for (unsigned i = 0; i < nArgs; ++i) {
				auto arg = stack.top();
				stack.pop();

				if (typeid(*arg) == typeid(CollisionHierarchy)) {
					collisions.emplace_back(
							*std::static_pointer_cast<CollisionHierarchy>(arg));
					continue;
				}

				if (convexHull == nullptr && terrain == nullptr
						&& box == nullptr && sphere == nullptr) {
					if (typeid(*arg) == typeid(ConvexHull)) {
						convexHull = std::static_pointer_cast<ConvexHull>(arg);
						continue;
					} else if (typeid(*arg) == typeid(Terrain)) {
						terrain = std::static_pointer_cast<Terrain>(arg);
						continue;
					} else if (typeid(*arg) == typeid(BoundingBox)) {
						box = std::static_pointer_cast<BoundingBox>(arg);
						continue;
					} else if (typeid(*arg) == typeid(Sphere)) {
						sphere = std::static_pointer_cast<Sphere>(arg);
						continue;
					}
				}

				scriptExecutionAssert(false,
						"Require ConvexHull, Terrain, Sphere or BoundingBox and optionally Collision");
			}

			if (convexHull != nullptr) {
				stack.emplace(
						std::make_shared<CollisionHierarchy>(convexHull,
								collisions));
			} else if (box != nullptr) {
				stack.emplace(
						std::make_shared<CollisionHierarchy>(box, collisions));
			} else if (terrain != nullptr) {
				stack.emplace(
						std::make_shared<CollisionHierarchy>(terrain,
								collisions));
			} else if (sphere != nullptr) {
				stack.emplace(
						std::make_shared<CollisionHierarchy>(sphere,
								collisions));
			} else {
				scriptExecutionAssert(false,
						"Require ConvexHull, Terrain, Sphere or BoundingBox and optionally Collision");
			}
		}
	};

	struct Collision {
		std::shared_ptr<BoundingBox> bounds;
		std::shared_ptr<Sphere> sphere;
		std::shared_ptr<ConvexHull> hull;
		std::shared_ptr<Terrain> terrain;
		std::shared_ptr<ParallelPlanes> planes;

		/**
		 * Construct from bounding box
		 *
		 * @param bounds
		 */
		Collision(const std::shared_ptr<BoundingBox> & bounds) :
				bounds(bounds) {
		}

		/**
		 * Construct from point
		 *
		 * @param hull
		 */
		Collision(const std::shared_ptr<Sphere> & sphere) :
				sphere(sphere) {
		}

		/**
		 * Construct from convex hull
		 *
		 * @param hull
		 */
		Collision(const std::shared_ptr<ConvexHull> & hull) :
				hull(hull) {
		}

		/**
		 * Construct from terrain mesh
		 *
		 * @param terrain
		 */
		Collision(const std::shared_ptr<Terrain> & terrain) :
				terrain(terrain) {
		}

		/**
		 * Construct from parallel planes mesh
		 *
		 * @param terrain
		 */
		Collision(const std::shared_ptr<ParallelPlanes> & planes) :
				planes(planes) {
		}

		/**
		 * Test for intersection between two collision shapes
		 *
		 * @param that
		 * @param toThis
		 * @return
		 */
		bool intersection(const Collision & that,
				const Transform & toThis) const {
			const auto & a = getBounds();
			auto b = that.getBounds().transformed(toThis);

			return a.intersects(b);
		}

		/**
		 * Calculate intersection of two collision shapes
		 *
		 * @param that
		 * @param toThis
		 * @param intersection  intersection, written only if collision
		 *
		 * @return              true if collision, false otherwise
		 */
		bool collide(const Collision & that, const Transform & toThis,
				Intersection & intersection) const {
			if (sphere != nullptr) {
				return sphereCollide(that, toThis, intersection);
			}
			if (hull != nullptr) {
				return hullCollide(that, toThis, intersection);
			}
			if (terrain != nullptr) {
				return terrainCollide(that, toThis, intersection);
			}
			if (planes != nullptr) {
				return planesCollide(that, toThis, intersection);
			}
			assert(bounds != nullptr);
			return boundsCollide(that, toThis, intersection);
		}

		/**
		 * Collision shape intersection with ray
		 *
		 * @param ray
		 * @param point
		 * @param distance
		 * @return
		 */
		bool rayIntersection(const Ray & ray, Vec3 & point,
				double & distance) const {
			if (hull != nullptr) {
				return hull->rayIntersection(ray, point, distance);
			}
			if (terrain != nullptr) {
				return terrain->getTriangleList().rayIntersection(ray, point,
						distance);
			}
			if (planes != nullptr) {
				return planes->rayIntersection(ray, point, distance);
			}
			if (bounds != nullptr) {
				return bounds->rayIntersection(ray, point, distance);
			}
			if (sphere != nullptr) {
				return sphere->rayIntersection(ray, point, distance);
			}
			assert(false);
			return false;
		}

		/**
		 * Collision bounds
		 *
		 * @return bounding box of collision shape
		 */
		const BoundingBox & getBounds() const {
			if (bounds != nullptr) {
				return *bounds;
			}
			if (sphere != nullptr) {
				return sphere->getBounds();
			}
			if (hull != nullptr) {
				return hull->getBounds();
			}
			if (terrain != nullptr) {
				return terrain->getBounds();
			}
			if (planes != nullptr) {
				return planes->getBounds();
			}
			assert(false);
			static BoundingBox maxBounds(
					Vec3(-std::numeric_limits<double>::max(),
							-std::numeric_limits<double>::max(),
							-std::numeric_limits<double>::max()),
					Vec3(std::numeric_limits<double>::max(),
							std::numeric_limits<double>::max(),
							std::numeric_limits<double>::max()));
			return maxBounds;
		}

		/**
		 * Validate
		 *
		 * @return true, if valid, false, if not valid
		 */
		bool validate() const {
			if (hull != nullptr) {
				return hull->validate();
			}
			if (terrain != nullptr) {
				return terrain->validate();
			}
			if (planes != nullptr) {
				return planes->validate();
			}
			return true;
		}

	private:
		/**
		 * Calculate intersection of a point with another collision shape
		 *
		 * @param that
		 * @param toThis        transform 'that' collision to sphere space
		 * @param intersection  intersection, written only if collision
		 *
		 * @return              true if collision, false otherwise
		 */
		bool sphereCollide(const Collision & that, const Transform & toThis,
				Intersection & intersection) const {
			assert(sphere != nullptr);
			if (that.sphere != nullptr) {
				return sphere->collide(*that.sphere, toThis, intersection);
			}
			if (that.bounds != nullptr) {
				return sphere->collide(*that.bounds, toThis, intersection);
			}
			if (that.hull != nullptr) {
				// reverse order
				if (that.hull->collide(*sphere, toThis.inverse(),
						intersection)) {
					// undo reverse order
					intersection.transform(toThis);
					intersection.flipNormal();
					return true;
				}
				return false;
			}
			if (that.terrain != nullptr) {
				// reverse order
				if (that.terrain->collide(*sphere, toThis.inverse(),
						intersection)) {
					// undo reverse order
					intersection.transform(toThis);
					intersection.flipNormal();
					return true;
				}
				return false;
			}
			assert(false);
			return nullptr;
		}

		/**
		 * Calculate intersection of a hull with another collision shape
		 *
		 * @param that
		 * @param toThis
		 * @param intersection  intersection, written only if collision
		 *
		 * @return              true if collision, false otherwise
		 */
		bool hullCollide(const Collision & that, const Transform & toThis,
				Intersection & intersection) const {
			assert(hull != nullptr);
			if (that.sphere != nullptr) {
				return hull->collide(*that.sphere, toThis, intersection);
			}
			if (that.hull != nullptr) {
				return hull->collide(*that.hull, toThis, intersection);
			}
			if (that.terrain != nullptr) {
				// reverse order
				if (that.terrain->collide(*hull, toThis.inverse(),
						intersection)) {
					// undo reverse order
					intersection.transform(toThis);
					intersection.flipNormal();
					return true;
				}
				return false;
			}
			if (that.planes != nullptr) {
				// reverse order
				if (that.planes->collide(*hull, toThis.inverse(),
						intersection)) {
					// undo reverse order
					intersection.transform(toThis);
					intersection.flipNormal();
					return true;
				}
				return false;
			}
			if (that.bounds != nullptr) {
				return hull->collide(*that.bounds, toThis, intersection);
			}
			assert(false);
			return nullptr;
		}

		/**
		 * Calculate intersection of a terrain with another collision shape
		 *
		 * @param that
		 * @param toThis
		 * @param intersection  intersection, written only if collision
		 *
		 * @return              true if collision, false otherwise
		 */
		bool terrainCollide(const Collision & that, const Transform & toThis,
				Intersection & intersection) const {
			assert(terrain != nullptr);
			if (that.sphere != nullptr) {
				return terrain->collide(*that.sphere, toThis, intersection);
			}
			if (that.hull != nullptr) {
				return terrain->collide(*that.hull, toThis, intersection);
			}
			if (that.bounds != nullptr) {
				return terrain->collide(*that.bounds, toThis, intersection);
			}
			assert(false);
			return false;
			// TODO terrain against bounds
		}

		/**
		 * Calculate intersection of bounds with another collision shape
		 *
		 * @param that
		 * @param toThis
		 * @param intersection  intersection, written only if collision
		 *
		 * @return              true if collision, false otherwise
		 */
		bool boundsCollide(const Collision & that, const Transform & toThis,
				Intersection & intersection) const {
			assert(bounds != nullptr);
			if (that.sphere != nullptr) {
				// reverse order
				if (that.sphere->collide(*bounds, toThis.inverse(),
						intersection)) {
					// undo reverse order
					intersection.transform(toThis);
					intersection.flipNormal();
					return true;
				}
				return false;
			}
			if (that.bounds != nullptr) {
				return bounds->collide(*that.bounds, toThis, intersection);
			}
			if (that.terrain != nullptr) {
				// reverse order
				if (that.terrain->collide(*bounds, toThis.inverse(),
						intersection)) {
					// undo reverse order
					intersection.transform(toThis);
					intersection.flipNormal();
					return true;
				}
				return false;
			}
			if (that.hull != nullptr) {
				// reverse order
				if (that.hull->collide(*bounds, toThis.inverse(),
						intersection)) {
					// undo reverse order
					intersection.transform(toThis);
					intersection.flipNormal();
					return true;
				}
				return false;
			}
			assert(false);
			return false;
		}

		/**
		 * Calculate intersection of a parallel planes with another collision
		 * shape
		 *
		 * @param that
		 * @param toThis
		 * @param intersection  intersection, written only if collision
		 *
		 * @return              true if collision, false otherwise
		 */
		bool planesCollide(const Collision & that, const Transform & toThis,
				Intersection & intersection) const {
			assert(planes != nullptr);
			if (that.hull != nullptr) {
				return planes->collide(*that.hull, toThis, intersection);
			}
			assert(false);
			return false;
			// TODO terrain against bounds
			// TODO bounds against terrain
		}

	};
}

struct CollisionHierarchy::impl {
	Collision collision;
	std::vector<CollisionHierarchy> children;

	/*
	 *
	 */
	impl(const std::shared_ptr<BoundingBox> & bounds,
			const std::vector<CollisionHierarchy> & children) :
			collision(bounds), children(children) {
	}

	/*
	 *
	 */
	impl(const std::shared_ptr<Sphere> & sphere,
			const std::vector<CollisionHierarchy> & children) :
			collision(sphere), children(children) {
	}

	/*
	 *
	 */
	impl(const std::shared_ptr<ConvexHull> & hull,
			const std::vector<CollisionHierarchy> & children) :
			collision(hull), children(children) {
	}

	/*
	 *
	 */
	impl(const std::shared_ptr<Terrain> & terrain,
			const std::vector<CollisionHierarchy> & children) :
			collision(terrain), children(children) {
	}

	/*
	 *
	 */
	impl(const std::shared_ptr<ParallelPlanes> & planes,
			const std::vector<CollisionHierarchy> & children) :
			collision(planes), children(children) {
	}

};

/**
 * Construct from bounding box and children
 *
 * @param bounds
 * @param children
 */
CollisionHierarchy::CollisionHierarchy(
		const std::shared_ptr<BoundingBox> & bounds,
		const std::vector<CollisionHierarchy> & children) :
		pimpl(new impl(bounds, children)) {
}

/**
 * Construct from sphere and children
 *
 * @param bounds
 * @param children
 */
CollisionHierarchy::CollisionHierarchy(const std::shared_ptr<Sphere> & sphere,
		std::vector<CollisionHierarchy> & children) :
		pimpl(new impl(sphere, children)) {
}

/**
 * Construct from convex hull and children
 *
 * @param hull
 * @param children
 */
CollisionHierarchy::CollisionHierarchy(const std::shared_ptr<ConvexHull> & hull,
		std::vector<CollisionHierarchy> & children) :
		pimpl(new impl(hull, children)) {
}

/**
 * Construct from terrain mesh and children
 *
 * @param terrain
 * @param children
 */
CollisionHierarchy::CollisionHierarchy(const std::shared_ptr<Terrain> & terrain,
		std::vector<CollisionHierarchy> & children) :
		pimpl(new impl(terrain, children)) {
}

/**
 * Construct from parallel planes mesh and children
 *
 * @param terrain
 * @param children
 */
CollisionHierarchy::CollisionHierarchy(
		const std::shared_ptr<ParallelPlanes> & planes,
		std::vector<CollisionHierarchy> & children) :
		pimpl(new impl(planes, children)) {
}

/**
 * destructor
 */
CollisionHierarchy::~CollisionHierarchy() {
}

/**
 * Leaf node collision
 *
 * @param that
 * @param toThis
 * @return
 */
PRIVATE std::vector<Intersection> CollisionHierarchy::leafCollide(
		const CollisionHierarchy & that, const Transform & toThis) const {
	std::vector<Intersection> intersections;
	Intersection intxn;
	if (pimpl->collision.collide(that.pimpl->collision, toThis, intxn)) {
		intersections.emplace_back(intxn);
	}
	return intersections;
}

/**
 * Collide two hierarchies, traversing 'that' children
 *
 * @param that
 * @param toThis
 * @return
 */
PRIVATE std::vector<Intersection> CollisionHierarchy::rCollide(
		const CollisionHierarchy & that, const Transform & toThis) const {
	if (that.pimpl->children.size() == 0) {
		if (pimpl->children.size() == 0) {
			return leafCollide(that, toThis);
		} else {
			return collide(that, toThis);
		}
	}
	std::vector<Intersection> intersections;
	if (pimpl->collision.intersection(that.pimpl->collision, toThis) == false) {
		return intersections;
	}

	for (const auto & child : that.pimpl->children) {
		auto childIntersections = collide(child, toThis);
		intersections.insert(intersections.end(), childIntersections.begin(),
				childIntersections.end());
	}
	return intersections;
}

/**
 * Collide two hierarchies, traversing 'this' children
 *
 * @param that
 * @param toThis
 * @return
 */
std::vector<Intersection> CollisionHierarchy::collide(
		const CollisionHierarchy & that, const Transform & toThis) const {
	if (pimpl->children.size() == 0) {
		if (that.pimpl->children.size() == 0) {
			return leafCollide(that, toThis);
		} else {
			return rCollide(that, toThis);
		}
	}

	std::vector<Intersection> intersections;

	if (pimpl->collision.intersection(that.pimpl->collision, toThis) == false) {
		return intersections;
	}

	for (const auto & child : pimpl->children) {
		auto childIntersection = child.rCollide(that, toThis);
		intersections.insert(intersections.end(), childIntersection.begin(),
				childIntersection.end());
	}
	return intersections;
}

/**
 * ray intersection
 *
 * @param ray
 * @param point
 * @param distance
 * @return
 */
bool CollisionHierarchy::rayIntersection(const Ray & ray, Vec3 & point,
		double & distance) const {
	if (pimpl->children.size() == 0) {
		return pimpl->collision.rayIntersection(ray, point, distance);
	}
	bool hit = false;
	Vec3 p;
	double d = std::numeric_limits<double>::max();
	for (const auto & child : pimpl->children) {
		if (child.rayIntersection(ray, point, distance)) {
			if (d < distance) {
				distance = d;
				point = p;
			} else {
				d = distance;
				p = point;
			}
			hit = true;
		}
	}
	return hit;
}

/**
 * get bounds
 *
 * @return bounding box of collision hierarchy
 */
const BoundingBox & CollisionHierarchy::getBounds() const {
	return pimpl->collision.getBounds();
}

/**
 * Draw debug geometry to list
 *
 * @param debug
 * @param toWorld
 */
void CollisionHierarchy::draw(std::vector<DebugGeometry> & debug,
		const Transform & toWorld) const {
	if (pimpl->collision.bounds != nullptr) {
		debug.emplace_back(toWorld, Color::red(), *pimpl->collision.bounds);
	} else if (pimpl->collision.hull != nullptr) {
		debug.emplace_back(toWorld, Color::red(),
				pimpl->collision.hull->getTriangleList());
	} else if (pimpl->collision.terrain != nullptr) {
		debug.emplace_back(toWorld, Color::red(),
				pimpl->collision.terrain->getTriangleList());
	} else if (pimpl->collision.planes != nullptr) {
		debug.emplace_back(toWorld, Color::red(),
				pimpl->collision.planes->getTriangleList());
	} else if (pimpl->collision.sphere != nullptr) {
		debug.emplace_back(toWorld, Color::blue(),
				pimpl->collision.sphere->getTriangleList());
	}
	for (const auto & child : pimpl->children) {
		child.draw(debug, toWorld);
	}
}

/**
 * validate
 *
 * @return true, if valid, otherwise false
 */
bool CollisionHierarchy::validate() const {
	if (pimpl->collision.validate() == false) {
		return false;
	}
	for (const auto & child : pimpl->children) {
		if (child.validate() == false) {
			return false;
		}
	}
	return true;
}
/**
 * get script object factory for CollisionHierarchy
 *
 * @return  CollisionHierarchy factory
 */
STATIC ScriptObjectPtr CollisionHierarchy::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
