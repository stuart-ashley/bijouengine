#include "rigidBody.h"

#include "boundingBox.h"
#include "collisionHierarchy.h"
#include "constraint.h"
#include "intersection.h"
#include "mat3.h"
#include "ray.h"
#include "transform.h"
#include "vec3.h"

#include <algorithm>
#include <cassert>

struct RigidBody::impl {
	std::string name;

	float inverseMass;
	Mat3 inverseInertialTensor;
	Vec3 linearVelocity;
	Vec3 angularVelocity;
	Transform rotTrans;
	Vec3 gravity;
	double sgp; // standard gravitational parameter

	CollisionHierarchy collision;
	double radius;

	std::unordered_set<std::string> nocollide;

	bool doFriction;
	Vec3 lastPosition;

	bool valid;

	impl(const std::string & name, float inverseMass,
			const Transform & rotTrans, const Vec3 & velocity,
			const Vec3 & angularVelocity, const CollisionHierarchy & collision,
			const Vec3 & gravity, double sgp,
			const std::unordered_set<std::string> & nocollide, bool doFriction) :
					name(name),
					inverseMass(inverseMass),
					inverseInertialTensor(0, 0, 0, 0, 0, 0, 0, 0, 0), // temporary
					linearVelocity(velocity),
					angularVelocity(angularVelocity),
					rotTrans(rotTrans),
					gravity(gravity),
					sgp(sgp),
					collision(collision),
					radius(-1),
					nocollide(nocollide),
					doFriction(doFriction),
					lastPosition(rotTrans.getTranslation()),
					valid(false) {
	}

	/**
	 * apply friction
	 */
	void applyFriction(impl & that, const Normal & normal) {
		Vec3 currentPosition = rotTrans.getTranslation();

		// calculate displacement from plane
		Vec3 tmp = currentPosition - lastPosition;
		double l = tmp.dot(normal);

		// calculate planar distance between points
		tmp.scaleAdd(-l, normal, currentPosition - lastPosition);
		double d = tmp.length();

		double e = 100000;

		if (d < e) {
			tmp.scaleAdd(l, normal, lastPosition);
			rotTrans.setTranslation(tmp);
			lastPosition = tmp;
		}

		currentPosition = that.rotTrans.getTranslation();

		// calculate displacement from plane
		tmp = currentPosition - that.lastPosition;
		l = tmp.dot(normal);

		// calculate planar distance between points
		tmp.scaleAdd(-l, normal, currentPosition - that.lastPosition);
		d = tmp.length();

		if (d < e) {
			tmp.scaleAdd(l, normal, that.lastPosition);
			that.rotTrans.setTranslation(tmp);
			that.lastPosition = tmp;
		}
	}

	/**
	 *
	 * @param that          body colliding against
	 * @param intersection  intersection to calculate impulse for
	 * @param outImpulse    calculated impulse
	 *
	 * @return              true if impulse calculated, false otherwise
	 */
	bool collisionImpulse(const impl & that, const Intersection & intersection,
			Vec3 & outImpulse) const {
		const auto & la = linearVelocity;
		const auto & wa = angularVelocity;

		const auto & lb = that.linearVelocity;
		const auto & wb = that.angularVelocity;

		// ra = point - ta
		Vec3 ra = intersection.getPoint() - rotTrans.getTranslation();
		// rb = point - tb
		Vec3 rb = intersection.getPoint() - that.rotTrans.getTranslation();

		// va = wa × ra + la
		Vec3 va = wa.cross(ra) + la;
		// vb = wb × rb + lb
		Vec3 vb = wb.cross(rb) + lb;

		// relativeVelocity = va - vb
		Vec3 relativeVelocity = va - vb;

		if (relativeVelocity.dot(intersection.getNormal()) > 0) {
			return false;
		}

		double e = 0;

		Mat3 iita = worldSpaceInverseInertialTensor();
		Mat3 iitb = that.worldSpaceInverseInertialTensor();

		// j = -e * rv·n / [ im + oim + n·((iita*(ra×n))×ra) +
		// n·((iitb*(rb×n))×rb) ]
		Vec3 tmpA = (iita * ra.cross(intersection.getNormal())).cross(ra);

		Vec3 tmpB = (iitb * rb.cross(intersection.getNormal())).cross(rb);

		double j = (-e * relativeVelocity.dot(intersection.getNormal()))
				/ (inverseMass + that.inverseMass
						+ tmpA.dot(intersection.getNormal())
						+ tmpB.dot(intersection.getNormal()));

		// impulse = j * normal
		Vec3 impulse;
		impulse.scale(j, intersection.getNormal());

		double relativeSpeed = relativeVelocity.length();
		if (relativeSpeed <= 0) {
			return false;
		}

		Normal n = relativeVelocity.normalized();
		// j = -rv·n / [ im + oim + n·((iita*(ra×n))×ra) +
		// n·((iitb*(rb×n))×rb) ]
		tmpA = (iita * ra.cross(n)).cross(ra);

		tmpB = (iitb * rb.cross(n)).cross(rb);

		j = -relativeSpeed
				/ (inverseMass + that.inverseMass + tmpA.dot(n) + tmpB.dot(n));

		// impulse = impulse + j * n
		outImpulse.scaleAdd(j, n, impulse);

		return true;
	}

	/**
	 *
	 * @param point
	 * @param impulse
	 */
	void shift(const Vec3 & point, const Vec3 & impulse) {
		Vec3 tmp = rotTrans.getTranslation();
		tmp.scaleAdd(inverseMass, impulse, tmp);
		rotTrans.setTranslation(tmp);

		Mat3 iita = worldSpaceInverseInertialTensor();

		Vec3 ra = point - rotTrans.getTranslation();

		Vec3 dAngularVelocity = iita * ra.cross(impulse);

		double l = dAngularVelocity.length();
		if (l > 0.001) {
			tmp = dAngularVelocity / l;
			Quat r = rotTrans.getRotation();
			rotTrans.setRotation(tmp, (float) (l * .5));
			rotTrans.rotate(r);
		}
	}

	/**
	 * Calculate inverse inertial tensor in world space
	 *
	 * @return Inverse inertial tensor in world space
	 */
	Mat3 worldSpaceInverseInertialTensor() const {
		auto r = rotTrans.getRotation();
		// wsiit = r * iit * r⁻¹
		Mat3 wsiit = Mat3(r) * inverseInertialTensor;
		return wsiit * Mat3(r.conjugate());
	}
};

/**
 * constructor
 *
 * @param name             name of body
 * @param inverseMass      inverse mass
 * @param rotTrans         initial transform
 * @param velocity         initial velocity
 * @param angularVelocity  initial angular velocity
 * @param collision        body collision
 * @param gravity          body gravity
 * @param sgp              standard gravitational parameter
 * @param nocollide        list of names of objects to not collide with
 * @param doFriction       friction flag
 */
RigidBody::RigidBody(const std::string & name, float inverseMass,
		const Transform & rotTrans, const Vec3 & velocity,
		const Vec3 & angularVelocity, const CollisionHierarchy & collision,
		const Vec3 & gravity, double sgp,
		const std::unordered_set<std::string> & nocollide, bool doFriction) :
				pimpl(
						new impl(name, inverseMass, rotTrans, velocity,
								angularVelocity, collision, gravity, sgp,
								nocollide, doFriction)) {
}

/**
 * Apply impulse to body
 *
 * @param point    point of impulse
 * @param impulse  impulse vector
 */
void RigidBody::applyImpulse(const Vec3 & point, const Vec3 & impulse) {
	// v = v + mass⁻¹ * impulse
	pimpl->linearVelocity.scaleAdd(pimpl->inverseMass, impulse,
			pimpl->linearVelocity);

	Mat3 iita = pimpl->worldSpaceInverseInertialTensor();

	// r = p - t
	Vec3 r = point - pimpl->rotTrans.getTranslation();

	// δω = iita * ( r × impulse )
	Vec3 dAngularVelocity = iita * r.cross(impulse);

	// ω = ω + δω
	pimpl->angularVelocity += dAngularVelocity;
}

/**
 * fix rotation constraint
 *
 * @param that        other body to which constrained
 * @param constraint  constraint between bodies
 */
void RigidBody::fixConstraintRotation(RigidBody & that,
		const Constraint & constraint) {
	// pivot rotation for 'this'
	Quat qa = pimpl->rotTrans.getRotation() * constraint.getPivot0Rot();

	// pivot rotation for 'that'
	Quat qb = that.pimpl->rotTrans.getRotation() * constraint.getPivot1Rot();

	Quat dq(0, 0, 0, 1);

	switch (constraint.getLimitFlags() & 56) {
	case 48: {
		// align x axis
		Vec3 u = qa.rotateX(1);
		Vec3 v = qb.rotateX(1);
		Vec3 n = u.cross(v);

		dq.set((float) n.getX(), (float) n.getY(), (float) n.getZ(),
				1 + (float) u.dot(v));
		break;
	}
	case 40: {
		// align y axis
		Vec3 u = qa.rotateY(1);
		Vec3 v = qb.rotateY(1);
		Vec3 n = u.cross(v);

		dq.set((float) n.getX(), (float) n.getY(), (float) n.getZ(),
				1 + (float) u.dot(v));
		break;
	}
	case 24: {
		// align z axis
		Vec3 u = qa.rotateZ(1);
		Vec3 v = qb.rotateZ(1);
		Vec3 n = u.cross(v);

		dq.set((float) n.getX(), (float) n.getY(), (float) n.getZ(),
				1 + (float) u.dot(v));
		break;
	}
	case 56:
		// rotation from qa to qb ( dq * qa = qb )
		dq = qb * qa.conjugate();
		break;
	default:
		assert(false);
	}

	// fix rotations
	if (pimpl->inverseMass > 0) {
		Quat a(0, 0, 0, 1);
		float t = (float) (pimpl->inverseMass
				/ (pimpl->inverseMass + that.pimpl->inverseMass));
		a.interpolate(dq, t);
		Quat r = pimpl->rotTrans.getRotation();
		pimpl->rotTrans.setRotation(a);
		pimpl->rotTrans.rotate(r);
	}

	if (that.pimpl->inverseMass > 0) {
		Quat a(0, 0, 0, 1);
		float t = (float) (that.pimpl->inverseMass
				/ (pimpl->inverseMass + that.pimpl->inverseMass));
		a.interpolate(dq.conjugate(), t);
		Quat r = that.pimpl->rotTrans.getRotation();
		that.pimpl->rotTrans.setRotation(a);
		that.pimpl->rotTrans.rotate(r);
	}
}

/**
 * fix translational constraint
 *
 * @param that        other body to which constrained
 * @param constraint  constraint between bodies
 */
void RigidBody::fixConstraintTranslation(RigidBody & that,
		const Constraint & constraint) {
	// pivot translation for 'this' and 'that'
	Vec3 pa = constraint.getPivot0Pos();
	pimpl->rotTrans.transformPoint(pa);
	Vec3 pb = constraint.getPivot1Pos();
	that.pimpl->rotTrans.transformPoint(pb);

	// ra = point - ta
	Vec3 ra = pa - pimpl->rotTrans.getTranslation();
	// rb = point - tb
	Vec3 rb = pb - that.pimpl->rotTrans.getTranslation();

	// ( pa - pb ) / time
	Vec3 requiredVelocity = pa - pb;

	Quat pivotRot = pimpl->rotTrans.getRotation() * constraint.getPivot0Rot();

	double requiredSpeed = requiredVelocity.length();
	if (requiredSpeed == 0) {
		return;
	}

	Normal directionOfMotion = requiredVelocity.normalized();

	Mat3 iita = pimpl->worldSpaceInverseInertialTensor();
	Mat3 iitb = that.pimpl->worldSpaceInverseInertialTensor();

	// j = -rv·n / [ im + oim + n·((iita*(ra×n))×ra) +
	// n·((iitb*(rb×n))×rb) ]
	Vec3 tmpA = (iita * ra.cross(directionOfMotion)).cross(ra);

	Vec3 tmpB = (iitb * rb.cross(directionOfMotion)).cross(rb);

	double j =
			-requiredSpeed
					/ (pimpl->inverseMass + that.pimpl->inverseMass
							+ tmpA.dot(directionOfMotion)
							+ tmpB.dot(directionOfMotion));

	// impulse = j * n
	Vec3 impulse;
	impulse.scaleAdd(j, directionOfMotion, impulse);

	pimpl->shift(pa, impulse);
	that.pimpl->shift(pb, -impulse);
}

/**
 * fix velocity for constraint
 *
 * @param that        other body to which constrained
 * @param constraint  constraint between bodies
 */
void RigidBody::fixConstraintVelocity(RigidBody & that,
		const Constraint & constraint) {
	// pivot translation for 'this' and 'that'
	Vec3 pa = constraint.getPivot0Pos();
	pimpl->rotTrans.transformPoint(pa);
	Vec3 pb = constraint.getPivot1Pos();
	that.pimpl->rotTrans.transformPoint(pb);

	// linear velocity
	Vec3 ra = pa - pimpl->rotTrans.getTranslation();
	Vec3 rb = pb - that.pimpl->rotTrans.getTranslation();

	Vec3 va = pimpl->angularVelocity.cross(ra) + pimpl->linearVelocity;
	Vec3 vb = that.pimpl->angularVelocity.cross(rb)
			+ that.pimpl->linearVelocity;

	// relative velocity va to vb
	Vec3 relativeVelocity = vb - va;

	Mat3 iita = pimpl->worldSpaceInverseInertialTensor();
	Mat3 iitb = that.pimpl->worldSpaceInverseInertialTensor();

	Mat3 raCross(0, (float) ra.getZ(), -(float) ra.getY(), -(float) ra.getZ(),
			0, (float) ra.getX(), (float) ra.getY(), -(float) ra.getX(), 0);
	Mat3 ma(pimpl->inverseMass, 0, 0, 0, pimpl->inverseMass, 0, 0, 0,
			pimpl->inverseMass);

	ma -= (raCross * iita) * raCross;

	Mat3 rbCross(0, (float) rb.getZ(), -(float) rb.getY(), -(float) rb.getZ(),
			0, (float) rb.getX(), (float) rb.getY(), -(float) rb.getX(), 0);
	Mat3 mb(that.pimpl->inverseMass, 0, 0, 0, that.pimpl->inverseMass, 0, 0, 0,
			that.pimpl->inverseMass);

	mb -= (rbCross * iitb) * rbCross;

	// impulse = (ma + mb)⁻¹ * dv
	Mat3 matrix = ma + mb;
	Vec3 impulse = matrix.inverse() * relativeVelocity;

	applyImpulse(pa, impulse);
	that.applyImpulse(pb, -impulse);
}

/**
 * get angular velocity of body
 *
 * @return  angular velocity of body
 */
const Vec3 & RigidBody::getAngularVelocity() const {
	return pimpl->angularVelocity;
}

/**
 * get bodies collision hierarchy
 *
 * @return  collision hierarchy of body
 */
const CollisionHierarchy & RigidBody::getCollision() const {
	return pimpl->collision;
}

/**
 * get bodies inverse mass
 *
 * @return  inverse mass of body
 */
float RigidBody::getInverseMass() const {
	return pimpl->inverseMass;
}

/**
 * get bodies linear velocity
 *
 * @return  linear velocity of body
 */
const Vec3 & RigidBody::getLinearVelocity() const {
	return pimpl->linearVelocity;
}

/**
 * get name of body
 *
 * @return  name of body
 */
const std::string & RigidBody::getName() const {
	return pimpl->name;
}

/**
 * get bodies collision radius
 *
 * @return  collision radius of body
 */
double RigidBody::getRadius() const {
	return pimpl->radius;
}

/**
 * get bodies rotation
 *
 * @return  rotation of body
 */
Quat RigidBody::getRotation() const {
	return pimpl->rotTrans.getRotation();
}

/**
 * get bodies standard gravitational parameter
 *
 * @return  standard gravitational parameter of body
 */
double RigidBody::getSgp() const {
	return pimpl->sgp;
}

/**
 * get bodies transform
 *
 * @return  transform of body
 */
const Transform & RigidBody::getTransform() const {
	return pimpl->rotTrans;
}

/**
 * get bodies translation
 *
 * @return  translation of body
 */
Vec3 RigidBody::getTranslation() const {
	return pimpl->rotTrans.getTranslation();
}

/**
 * is name in list of bodies not to collide with
 *
 * @param name  name to test
 *
 * @return      true if not to collide, false otherwise
 */
bool RigidBody::noCollide(const std::string & name) const {
	if (pimpl->nocollide.size() == 0) {
		return false;
	}
	return pimpl->nocollide.find(name) != pimpl->nocollide.end();
}

/**
 * Intersection against ray, writes point and distance and returns true if
 * intersection, otherwise returns false leaving point and distance
 * untouched
 *
 * @param ray       ray to insect with
 * @param point     written if intersection occurs, returns intersection point
 * @param distance  written if intersection occurs, returns distance along ray to
 *                  intersection point, will be negative if ray is inside object
 *
 * @return          result of intersection
 */
bool RigidBody::rayIntersection(const Ray & ray, Vec3 & point,
		double & distance) const {
	// ray in 'this' space
	Ray r = ray;
	r.transform(pimpl->rotTrans.inverse());

	if (pimpl->collision.rayIntersection(r, point, distance)) {
		// point out of 'this' space
		pimpl->rotTrans.transformPoint(point);
		return true;
	}
	return false;
}

/**
 * Resolve collision, if any
 *
 * @param that  other rigid body
 *
 * @return      list of intersections
 */
std::vector<Intersection> RigidBody::resolveCollision(RigidBody & that) {
	auto dp = pimpl->rotTrans.getTranslation() -
			that.pimpl->rotTrans.getTranslation();
	auto r = pimpl->radius + that.pimpl->radius;
	if (dp.dot(dp) > r * r) {
		return std::vector<Intersection>();
	}

	auto intersections = pimpl->collision.collide(that.pimpl->collision,
			that.pimpl->rotTrans.to(pimpl->rotTrans));

	for (auto & i : intersections) {
		// intersection into world space
		i.transform(pimpl->rotTrans);

		// separate this and that
		Vec3 v;
		v.scale(i.getDepth() / (pimpl->inverseMass + that.pimpl->inverseMass),
				i.getNormal());

		auto tmp = pimpl->rotTrans.getTranslation();
		tmp.scaleAdd(pimpl->inverseMass, v, tmp);
		pimpl->rotTrans.setTranslation(tmp);

		tmp = that.pimpl->rotTrans.getTranslation();
		tmp.scaleAdd(-that.pimpl->inverseMass, v, tmp);
		that.pimpl->rotTrans.setTranslation(tmp);

		// calculate impulse in world
		Vec3 impulse;
		if (pimpl->collisionImpulse(*that.pimpl, i, impulse) == false) {
			continue;
		}

		applyImpulse(i.getPoint(), impulse);
		that.applyImpulse(i.getPoint(), -impulse);

		// friction
		if (pimpl->doFriction && that.pimpl->doFriction) {
			pimpl->applyFriction(*that.pimpl, i.getNormal());
		}
	}

	return intersections;
}

/**
 * set bodies angular velocity
 *
 * @param v  new angular velocity
 */
void RigidBody::setAngularVelocity(const Vec3 & v) {
	pimpl->angularVelocity = v;
}

/**
 * set bodies angular velocity
 *
 * @param x  x component of new angular velocity
 * @param y  y component of new angular velocity
 * @param z  z component of new angular velocity
 */
void RigidBody::setAngularVelocity(double x, double y, double z) {
	pimpl->angularVelocity.set(x, y, z);
}

/**
 * set bodies linear velocity
 *
 * @param v  new linear velocity
 */
void RigidBody::setLinearVelocity(const Vec3 & v) {
	pimpl->linearVelocity = v;
}

/**
 * set bodies linear velocity
 *
 * @param x  x component of new linear velocity
 * @param y  y component of new linear velocity
 * @param z  z component of new linear velocity
 */
void RigidBody::setLinearVelocity(double x, double y, double z) {
	pimpl->linearVelocity.set(x, y, z);
}

/**
 * set bodies rotation
 *
 * @param q  new rotation
 */
void RigidBody::setRotation(const Quat & q) {
	assert(std::isfinite(q.getX()));
	assert(std::isfinite(q.getY()));
	assert(std::isfinite(q.getZ()));
	assert(std::isfinite(q.getW()));
	pimpl->rotTrans.setRotation(q);
}

/**
 * set bodies transform
 *
 * @param t  new transform
 */
void RigidBody::setTransform(const Transform & t) {
	Vec3 v = t.getTranslation();
	Quat q = t.getRotation();
	assert(std::isfinite(q.getX()));
	assert(std::isfinite(q.getY()));
	assert(std::isfinite(q.getZ()));
	assert(std::isfinite(q.getW()));
	pimpl->rotTrans.set(t);
}

/**
 * set bodies translation
 *
 * @param v  new translation
 */
void RigidBody::setTranslation(const Vec3 & v) {
	pimpl->rotTrans.setTranslation(v);
}

/**
 * update body
 *
 * @param time  time step to update for
 */
void RigidBody::step(double time) {
	pimpl->lastPosition = pimpl->rotTrans.getTranslation();
	Vec3 tmp;
	tmp.scaleAdd(time, pimpl->linearVelocity, pimpl->lastPosition);

	// gravity
	if (pimpl->inverseMass > 0) {
		tmp.scaleAdd(time * time * .5, pimpl->gravity, tmp);
		pimpl->linearVelocity.scaleAdd(time, pimpl->gravity,
				pimpl->linearVelocity);
	}

	pimpl->rotTrans.setTranslation(tmp);

	double l = pimpl->angularVelocity.length();
	if (l > 0.001) {
		tmp = pimpl->angularVelocity / l;
		Quat r = pimpl->rotTrans.getRotation();
		pimpl->rotTrans.setRotation(tmp, (float) (l * time * .5));
		pimpl->rotTrans.rotate(r);
	}

	// damping
	if (pimpl->gravity.length() > 0) {
		pimpl->linearVelocity *= 1 - time * .1;
		pimpl->angularVelocity *= 1 - time * .1;
	}
}

/**
 * check validity of body, check collision loaded
 */
bool RigidBody::validate() {
	if (pimpl->valid == true) {
		return true;
	}
	if (pimpl->collision.validate() == false) {
		return false;
	}
	auto bounds = pimpl->collision.getBounds();
	Vec3 extents = bounds.getExtents();
	pimpl->radius = bounds.getRadius() + bounds.getCentre().length();

	// tensor
	double ex2 = extents.getX() * extents.getX();
	double ey2 = extents.getY() * extents.getY();
	double ez2 = extents.getZ() * extents.getZ();
	pimpl->inverseInertialTensor = Mat3(
			(float) (12.f * pimpl->inverseMass / (ey2 + ez2)), 0, 0, 0,
			(float) (12.f * pimpl->inverseMass / (ez2 + ex2)), 0, 0, 0,
			(float) (12.f * pimpl->inverseMass / (ex2 + ey2)));

	pimpl->valid = true;

	return true;
}
