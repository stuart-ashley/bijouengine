#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

class CollisionHierarchy;
class Constraint;
class Intersection;
class Quat;
class Ray;
class Transform;
class Vec3;

class RigidBody {
public:

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
	RigidBody(const std::string & name, float inverseMass,
			const Transform & rotTrans, const Vec3 & velocity,
			const Vec3 & angularVelocity, const CollisionHierarchy & collision,
			const Vec3 & gravity, double sgp,
			const std::unordered_set<std::string> & nocollide, bool doFriction);

	/**
	 * Apply impulse to body
	 *
	 * @param point    point of impulse
	 * @param impulse  impulse vector
	 */
	void applyImpulse(const Vec3 & point, const Vec3 & impulse);

	/**
	 * fix rotation constraint
	 *
	 * @param that        other body to which constrained
	 * @param constraint  constraint between bodies
	 */
	void fixConstraintRotation(RigidBody & that, const Constraint & constraint);

	/**
	 * fix translational constraint
	 *
	 * @param that        other body to which constrained
	 * @param constraint  constraint between bodies
	 */
	void fixConstraintTranslation(RigidBody & that,
			const Constraint & constraint);

	/**
	 * fix velocity for constraint
	 *
	 * @param that        other body to which constrained
	 * @param constraint  constraint between bodies
	 */
	void fixConstraintVelocity(RigidBody & that, const Constraint & constraint);

	/**
	 * get angular velocity of body
	 *
	 * @return  angular velocity of body
	 */
	const Vec3 & getAngularVelocity() const;

	/**
	 * get bodies collision hierarchy
	 *
	 * @return  collision hierarchy of body
	 */
	const CollisionHierarchy & getCollision() const;

	/**
	 * get bodies inverse mass
	 *
	 * @return  inverse mass of body
	 */
	float getInverseMass() const;

	/**
	 * get bodies linear velocity
	 *
	 * @return  linear velocity of body
	 */
	const Vec3 & getLinearVelocity() const;

	/**
	 * get name of body
	 *
	 * @return  name of body
	 */
	const std::string & getName() const;

	/**
	 * get bodies collision radius
	 *
	 * @return  collision radius of body
	 */
	double getRadius() const;

	/**
	 * get bodies rotation
	 *
	 * @return  rotation of body
	 */
	Quat getRotation() const;

	/**
	 * get bodies standard gravitational parameter
	 *
	 * @return  standard gravitational parameter of body
	 */
	double getSgp() const;

	/**
	 * get bodies transform
	 *
	 * @return  transform of body
	 */
	const Transform & getTransform() const;

	/**
	 * get bodies translation
	 *
	 * @return  translation of body
	 */
	Vec3 getTranslation() const;

	/**
	 * is name in list of bodies not to collide with
	 *
	 * @param name  name to test
	 *
	 * @return      true if not to collide, false otherwise
	 */
	bool noCollide(const std::string & name) const;

	/**
	 * is this body equal to other
	 *
	 * @param other  body to compare to
	 *
	 * @return       true if equal, false otherwise
	 */
	inline bool operator==(const RigidBody & other) const{
		return pimpl == other.pimpl;
	}

	/**
	 * is this body not equal to other
	 *
	 * @param other  body to compare to
	 *
	 * @return       true if not equal, false otherwise
	 */
	inline bool operator!=(const RigidBody & other) const{
		return pimpl != other.pimpl;
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
	bool rayIntersection(const Ray & ray, Vec3 & point,
			double & distance) const;

	/**
	 * Resolve collision, if any
	 *
	 * @param that  other rigid body
	 *
	 * @return      list of intersections
	 */
	std::vector<Intersection> resolveCollision(RigidBody & that);

	/**
	 * set bodies angular velocity
	 *
	 * @param v  new angular velocity
	 */
	void setAngularVelocity(const Vec3 & v);

	/**
	 * set bodies angular velocity
	 *
	 * @param x  x component of new angular velocity
	 * @param y  y component of new angular velocity
	 * @param z  z component of new angular velocity
	 */
	void setAngularVelocity(double x, double y, double z);

	/**
	 * set bodies linear velocity
	 *
	 * @param v  new linear velocity
	 */
	void setLinearVelocity(const Vec3 & v);

	/**
	 * set bodies linear velocity
	 *
	 * @param x  x component of new linear velocity
	 * @param y  y component of new linear velocity
	 * @param z  z component of new linear velocity
	 */
	void setLinearVelocity(double x, double y, double z);

	/**
	 * set bodies rotation
	 *
	 * @param q  new rotation
	 */
	void setRotation(const Quat & q);

	/**
	 * set bodies transform
	 *
	 * @param t  new transform
	 */
	void setTransform(const Transform & t);

	/**
	 * set bodies translation
	 *
	 * @param v  new translation
	 */
	void setTranslation(const Vec3 & v);

	/**
	 * update body
	 *
	 * @param time  time step to update for
	 */
	void step(double time);

	/**
	 * check validity of body, check collision loaded
	 */
	bool validate();

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

