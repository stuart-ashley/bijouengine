#include "sphere.h"

#include "boundingBox.h"
#include "indexArray.h"
#include "intersection.h"
#include "ray.h"
#include "transform.h"
#include "triangleList.h"
#include "vec3.h"
#include "vec3Array.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

#include <cmath>

namespace {

	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 2);

			auto point = getArg<Vec3>("Vec3", stack, 1);
			float radius = static_cast<float>(getNumericArg(stack, 2));

			stack.emplace(std::make_shared<Sphere>(point, radius));
		}
	};

	/*
	 * indices and vertices for icosahedron
	 */
	const double s = 0.5257311121191336;
	const double t = 0.85065080835204;

	Vec3Array vertexArray = { Vec3(-s, t, 0), Vec3(s, t, 0), Vec3(-s, -t, 0),
			Vec3(s, -t, 0), Vec3(0, -s, t), Vec3(0, s, t), Vec3(0, -s, -t),
			Vec3(0, s, -t), Vec3(t, 0, -s), Vec3(t, 0, s), Vec3(-t, 0, -s),
			Vec3(-t, 0, s) };

	IndexArray indexArray = { 0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
			1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8, 3, 9, 4, 3, 4, 2,
			3, 2, 6, 3, 6, 8, 3, 8, 9, 4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9,
			8, 1 };
}

/*
 *
 */
struct Sphere::impl {
	Vec3 point;
	float radius;
	std::unique_ptr<BoundingBox> bounds;
	std::unique_ptr<TriangleList> triangleList;

	impl(const Vec3 & point, float radius) :
			point(point), radius(radius) {
	}
};

/**
 * construct sphere from point and radius
 *
 * @param point   centre of sphere
 * @param radius  radius of sphere
 */
Sphere::Sphere(const Vec3 & point, float radius) :
		pimpl(new impl(point, radius)) {
}

/**
 * destructor
 */
Sphere::~Sphere() {
}

/**
 * Collide sphere against sphere, update intersection if collision
 *
 * @param other         sphere to collide against
 * @param otherToThis   transform from other sphere space to this sphere space
 * @param intersection  intersection, updated if collision occurs
 *
 * @return              true if spheres collide, false otherwise
 */
bool Sphere::collide(const Sphere & other, const Transform & otherToThis,
		Intersection & intersection) const {
	Vec3 p = other.pimpl->point;
	otherToThis.transformPoint(p);

	Vec3 delta = pimpl->point - p;
	double d = delta.length();

	if (d > pimpl->radius + other.pimpl->radius) {
		return false;
	}

	double intxnDepth = pimpl->radius + other.pimpl->radius - d;
	Normal intxnNormal(0.f, 0.f, 1.f);
	Vec3 intxnPoint;

	if (d < 0.0001) {
		intxnPoint = pimpl->point;
	} else {
		intxnNormal = delta.normalized();
		intxnPoint.scaleAdd(-.5 * (pimpl->radius - other.pimpl->radius + d),
				delta, pimpl->point);
	}

	intersection.set(intxnPoint, intxnNormal, intxnDepth);
	return true;
}

/**
 * Collide sphere against bounding box, update intersection if collision
 *
 * @param box           bounding box to collide against
 * @param boxToThis     transform from box space to this sphere space
 * @param intersection  intersection, updated if collision occurs
 *
 * @return              true if collision, false otherwise
 */
bool Sphere::collide(const BoundingBox & box, const Transform & boxToThis,
		Intersection & intersection) const {
	Vec3 p = pimpl->point;
	boxToThis.inverseTransformPoint(p);

	Vec3 n;

	const auto & min = box.getMin();
	const auto & max = box.getMax();

	if (p.getX() < min.getX()) {
		n.setX(p.getX() - min.getX());
	} else if (p.getX() > max.getX()) {
		n.setX(p.getX() - max.getX());
	}

	if (p.getY() < min.getY()) {
		n.setY(p.getY() - min.getY());
	} else if (p.getY() > max.getY()) {
		n.setY(p.getY() - max.getY());
	}

	if (p.getZ() < min.getZ()) {
		n.setZ(p.getZ() - min.getZ());
	} else if (p.getZ() > max.getZ()) {
		n.setZ(p.getZ() - max.getZ());
	}

	double d = n.length();
	if (d > pimpl->radius) {
		return false;
	}

	double intxnDepth;
	Normal intxnNormal(1.f, 0.f, 0.f);  // normal in box space
	Vec3 intxnPoint;              // point on sphere in box space

	// test sphere centre outside box
	if (d > 0) {
		intxnDepth = pimpl->radius - d;
		intxnNormal = n.normalized();
		intxnPoint.scaleAdd(-pimpl->radius, intxnNormal, p);
	} else {
		auto dmin = p - min;
		auto dmax = max - p;

		if (dmin.getX() < dmax.getX()) {
			intxnDepth = pimpl->radius + dmin.getX();
			intxnNormal.set(-1, 0, 0);
			intxnPoint.set(min.getX() + intxnDepth, p.getY(), p.getZ());
		} else {
			intxnDepth = pimpl->radius + dmax.getX();
			intxnNormal.set(1, 0, 0);
			intxnPoint.set(max.getX() - intxnDepth, p.getY(), p.getZ());
		}

		if (dmin.getY() < dmax.getY()) {
			if (pimpl->radius + dmin.getY() < intxnDepth) {
				intxnDepth = pimpl->radius + dmin.getY();
				intxnNormal.set(0, -1, 0);
				intxnPoint.set(p.getX(), min.getY() + intxnDepth, p.getZ());
			}
		} else {
			if (pimpl->radius + dmax.getY() < intxnDepth) {
				intxnDepth = pimpl->radius + dmax.getY();
				intxnNormal.set(0, 1, 0);
				intxnPoint.set(p.getX(), max.getY() - intxnDepth, p.getZ());
			}
		}

		if (dmin.getZ() < dmax.getZ()) {
			if (pimpl->radius + dmin.getZ() < intxnDepth) {
				intxnDepth = pimpl->radius + dmin.getZ();
				intxnNormal.set(0, 0, -1);
				intxnPoint.set(p.getX(), p.getY(), min.getZ() + intxnDepth);
			}
		} else {
			if (pimpl->radius + dmax.getZ() < intxnDepth) {
				intxnDepth = pimpl->radius + dmax.getZ();
				intxnNormal.set(0, 0, 1);
				intxnPoint.set(p.getX(), p.getY(), max.getZ() - intxnDepth);
			}
		}
	}

	intersection.set(intxnPoint, intxnNormal, intxnDepth);
	intersection.transform(boxToThis);

	return true;
}

/**
 * get bounds of sphere
 *
 * @return  bounds
 */
const BoundingBox & Sphere::getBounds() const {
	if (pimpl->bounds != nullptr) {
		return *pimpl->bounds;
	}

	Vec3 min(pimpl->point.getX() - pimpl->radius,
			pimpl->point.getY() - pimpl->radius,
			pimpl->point.getZ() - pimpl->radius);

	Vec3 max(pimpl->point.getX() + pimpl->radius,
			pimpl->point.getY() + pimpl->radius,
			pimpl->point.getZ() + pimpl->radius);

	pimpl->bounds = std::unique_ptr<BoundingBox>(new BoundingBox(min, max));

	return *pimpl->bounds;
}

/**
 * get centre of sphere
 *
 * @return  centre
 */
const Vec3 & Sphere::getPoint() const {
	return pimpl->point;
}

/**
 * get sphere radius
 *
 * @return  radius
 */
float Sphere::getRadius() const {
	return pimpl->radius;
}

/**
 * sphere as icosahedron
 *
 * @return  icosahedron triangles
 */
const TriangleList & Sphere::getTriangleList() const {
	if (pimpl->triangleList == nullptr) {
		pimpl->triangleList = std::unique_ptr<TriangleList>(
				new TriangleList(indexArray,
						vertexArray.scaled(pimpl->radius)));
	}
	return *pimpl->triangleList;
}

/**
 * Intersection against ray. Negative ray intersections are discarded. If
 * ray starts outside sphere, intersection is at entry. If ray starts in
 * sphere, intersection is at exit.
 *
 * @param ray       ray to insect with
 * @param point     written if intersection occurs, intersection point on
 *                  surface of sphere
 * @param distance  written if intersection occurs, distance along ray to
 *                  intersection point on the surface of the sphere
 *
 * @return result of intersection
 */
bool Sphere::rayIntersection(const Ray & ray, Vec3 & point,
		double & distance) const {
	Vec3 v = ray.getStart() - pimpl->point;
	double b = v.dot(ray.getDirection());
	double discriminant = b * b - v.dot(v) + pimpl->radius * pimpl->radius;

	if (discriminant < 0) {
		// no solution
		return false;
	}

	if (discriminant == 0) {
		// single solution
		if (-b >= 0) {
			// positive along ray
			distance = -b;
			point.scaleAdd(-b, ray.getDirection(), ray.getStart());
			return true;
		}
		return false;
	}

	// two solutions
	double t0 = -b + std::sqrt(discriminant);
	double t1 = -b - std::sqrt(discriminant);

	if (t0 >= 0 && (t0 < t1 || t1 < 0)) {
		// positive along ray
		distance = t0;
		point.scaleAdd(t0, ray.getDirection(), ray.getStart());
		return true;
	}

	if (t1 >= 0 && (t1 < t0 || t0 < 0)) {
		// positive along ray
		distance = t1;
		point.scaleAdd(t1, ray.getDirection(), ray.getStart());
		return true;
	}

	return false;
}

/**
 * get script object factory for Sphere
 *
 * @return  Sphere factory
 */
STATIC const ScriptObjectPtr & Sphere::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
