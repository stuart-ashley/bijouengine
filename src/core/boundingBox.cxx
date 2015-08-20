#include "boundingBox.h"

#include "intersection.h"
#include "quat.h"
#include "ray.h"
#include "transform.h"
#include "vec3.h"

#include "../scripting/executable.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace {
	const float epsilon = .00001f;

	/*
	 *
	 */
	std::vector<BaseParameter> params = { Parameter<Real>("min", nullptr),
			Parameter<Real>("max", nullptr) };

	/*
	 *
	 */
	struct Factory: public Executable {
		Parameters parameters;

		Factory() :
				parameters(params) {
		}

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			auto args = parameters.getArgs(nArgs, stack);
			auto min = std::static_pointer_cast<Vec3>(args["min"]);
			auto max = std::static_pointer_cast<Vec3>(args["max"]);
			stack.emplace(std::make_shared<BoundingBox>(*min, *max));
		}
	};

	/*
	 *
	 */
	class Contains: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto box = std::static_pointer_cast<BoundingBox>(self);

			auto point = getArg<Vec3>("Vec3", stack, 1);

			if (box->contains(point)) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	/*
	 *
	 */
	class GetRadius: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto box = std::static_pointer_cast<BoundingBox>(self);
			stack.emplace(std::make_shared<Real>(box->getRadius()));
		}
	};

	/*
	 *
	 */
	class Add: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto box = std::static_pointer_cast<BoundingBox>(self);

			auto bb = getArg<BoundingBox>("bounding box", stack, 1);

			stack.emplace(std::make_shared<BoundingBox>(*box + bb));
		}
	};

	/*
	 *
	 */
	class GetMin: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto box = std::static_pointer_cast<BoundingBox>(self);

			stack.emplace(std::make_shared<Vec3>(box->getMin()));
		}
	};

	/*
	 *
	 */
	class GetMax: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto box = std::static_pointer_cast<BoundingBox>(self);

			stack.emplace(std::make_shared<Vec3>(box->getMax()));
		}
	};

	const Normal directions[6] = {
			Normal(1.f, 0.f, 0.f),
			Normal(0.f, 1.f, 0.f),
			Normal(0.f, 0.f, 1.f),
			Normal(-1.f, 0.f, 0.f),
			Normal(0.f, -1.f, 0.f),
			Normal(0.f, 0.f, -1.f) };
}

/**
 * construct bounding box from single point
 *
 * @param a  first point
 */
BoundingBox::BoundingBox(const Vec3 & a) :
		m_min(a), m_max(a) {
}

/**
 * construct bounding box from two points
 *
 * @param a  first point
 * @param b  second point
 */
BoundingBox::BoundingBox(const Vec3 & a, const Vec3 & b) :
				m_min(std::min(a.getX(), b.getX()),
						std::min(a.getY(), b.getY()),
						std::min(a.getZ(), b.getZ())),
				m_max(std::max(a.getX(), b.getX()),
						std::max(a.getY(), b.getY()),
						std::max(a.getZ(), b.getZ())) {
}

/**
 * construct bounding box from three points
 *
 * @param a  first point
 * @param b  second point
 * @param c  third point
 */
BoundingBox::BoundingBox(const Vec3 & a, const Vec3 & b, const Vec3 & c) :
				m_min(std::min(std::min(a.getX(), b.getX()), c.getX()),
						std::min(std::min(a.getY(), b.getY()), c.getY()),
						std::min(std::min(a.getZ(), b.getZ()), c.getZ())),
				m_max(std::max(std::max(a.getX(), b.getX()), c.getX()),
						std::max(std::max(a.getY(), b.getY()), c.getY()),
						std::max(std::max(a.getZ(), b.getZ()), c.getZ())) {
}

/**
 * Collide two boxes
 *
 * @param that          box to collide against
 * @param thatToThis    from 'that' space to 'this' space
 * @param intersection  intersection, written only if collision
 *
 * @return              true if collision, false otherwise
 */
bool BoundingBox::collide(const BoundingBox & that,
		const Transform & thatToThis, Intersection & intersection) const {
	// that box in this space
	auto thatInThisSpace = that.transformed(thatToThis);

	if (intersects(thatInThisSpace) == false) {
		return false;
	}
	// 'this' space to 'that' space
	Transform thisToThat = thatToThis.inverse();

	// 'this' space in 'that' space
	auto thisInThatSpace = transformed(thisToThat);

	if (thisInThatSpace.intersects(that) == false) {
		return false;
	}

	// 'this' to 'that' space rotation matrix
	Mat3 thisToThatMat3 = thisToThat.getRotationMatrix();
	Mat3 thatToThisMat3 = thatToThis.getRotationMatrix();

	double intxnDepth = std::numeric_limits<double>::max();
	Vec3 intxnPoint;
	Normal intxnNormal = directions[0]; // initial value not important

	// 'this' x-axis
	if (m_min.getX() < thatInThisSpace.m_min.getX()) {
		if (m_max.getX() - thatInThisSpace.m_min.getX() < intxnDepth) {
			intxnDepth = m_max.getX() - thatInThisSpace.m_min.getX();
			intxnNormal = directions[3];
			intxnPoint = Vec3(
					thisToThatMat3.get(0, 0) > 0 ?
							that.m_min.getX() : that.m_max.getX(),
					thisToThatMat3.get(1, 0) > 0 ?
							that.m_min.getY() : that.m_max.getY(),
					thisToThatMat3.get(2, 0) > 0 ?
							that.m_min.getZ() : that.m_max.getZ());
			thatToThis.transformPoint(intxnPoint);
			intxnPoint.setX(intxnPoint.getX() + intxnDepth);
		}
	} else {
		if (thatInThisSpace.m_max.getX() - m_min.getX() < intxnDepth) {
			intxnDepth = thatInThisSpace.m_max.getX() - m_min.getX();
			intxnNormal = directions[0];
			intxnPoint = Vec3(
					thisToThatMat3.get(0, 0) < 0 ?
							that.m_min.getX() : that.m_max.getX(),
					thisToThatMat3.get(1, 0) < 0 ?
							that.m_min.getY() : that.m_max.getY(),
					thisToThatMat3.get(2, 0) < 0 ?
							that.m_min.getZ() : that.m_max.getZ());
			thatToThis.transformPoint(intxnPoint);
			intxnPoint.setX(intxnPoint.getX() - intxnDepth);

		}
	}
	// 'this' y-axis
	if (m_min.getY() < thatInThisSpace.m_min.getY()) {
		if (m_max.getY() - thatInThisSpace.m_min.getY() < intxnDepth) {
			intxnDepth = m_max.getY() - thatInThisSpace.m_min.getY();
			intxnNormal = directions[4];
			intxnPoint = Vec3(
					thisToThatMat3.get(0, 1) > 0 ?
							that.m_min.getX() : that.m_max.getX(),
					thisToThatMat3.get(1, 1) > 0 ?
							that.m_min.getY() : that.m_max.getY(),
					thisToThatMat3.get(2, 1) > 0 ?
							that.m_min.getZ() : that.m_max.getZ());
			thatToThis.transformPoint(intxnPoint);
			intxnPoint.setY(intxnPoint.getY() + intxnDepth);
		}
	} else {
		if (thatInThisSpace.m_max.getY() - m_min.getY() < intxnDepth) {
			intxnDepth = thatInThisSpace.m_max.getY() - m_min.getY();
			intxnNormal = directions[1];
			intxnPoint = Vec3(
					thisToThatMat3.get(0, 1) < 0 ?
							that.m_min.getX() : that.m_max.getX(),
					thisToThatMat3.get(1, 1) < 0 ?
							that.m_min.getY() : that.m_max.getY(),
					thisToThatMat3.get(2, 1) < 0 ?
							that.m_min.getZ() : that.m_max.getZ());
			thatToThis.transformPoint(intxnPoint);
			intxnPoint.setY(intxnPoint.getY() - intxnDepth);
		}
	}
	// 'this' z-axis
	if (m_min.getZ() < thatInThisSpace.m_min.getZ()) {
		if (m_max.getZ() - thatInThisSpace.m_min.getZ() < intxnDepth) {
			intxnDepth = m_max.getZ() - thatInThisSpace.m_min.getZ();
			intxnNormal = directions[5];
			intxnPoint = Vec3(
					thisToThatMat3.get(0, 2) > 0 ?
							that.m_min.getX() : that.m_max.getX(),
					thisToThatMat3.get(1, 2) > 0 ?
							that.m_min.getY() : that.m_max.getY(),
					thisToThatMat3.get(2, 2) > 0 ?
							that.m_min.getZ() : that.m_max.getZ());
			thatToThis.transformPoint(intxnPoint);
			intxnPoint.setZ(intxnPoint.getZ() + intxnDepth);
		}
	} else {
		if (thatInThisSpace.m_max.getZ() - m_min.getZ() < intxnDepth) {
			intxnDepth = thatInThisSpace.m_max.getZ() - m_min.getZ();
			intxnNormal = directions[2];
			intxnPoint = Vec3(
					thisToThatMat3.get(0, 2) < 0 ?
							that.m_min.getX() : that.m_max.getX(),
					thisToThatMat3.get(1, 2) < 0 ?
							that.m_min.getY() : that.m_max.getY(),
					thisToThatMat3.get(2, 2) < 0 ?
							that.m_min.getZ() : that.m_max.getZ());
			thatToThis.transformPoint(intxnPoint);
			intxnPoint.setZ(intxnPoint.getZ() - intxnDepth);
		}
	}

	// 'that' x-axis
	if (that.m_min.getX() < thisInThatSpace.m_min.getX()) {
		if (that.m_max.getX() - thisInThatSpace.m_min.getX() < intxnDepth) {
			intxnDepth = that.m_max.getX() - thisInThatSpace.m_min.getX();
			intxnNormal = Normal(thatToThisMat3.get(0, 0),
					thatToThisMat3.get(1, 0), thatToThisMat3.get(2, 0));
			intxnPoint = Vec3(
					thatToThisMat3.get(0, 0) > 0 ? m_min.getX() : m_max.getX(),
					thatToThisMat3.get(1, 0) > 0 ? m_min.getY() : m_max.getY(),
					thatToThisMat3.get(2, 0) > 0 ? m_min.getZ() : m_max.getZ());
		}
	} else {
		if (thisInThatSpace.m_max.getX() - that.m_min.getX() < intxnDepth) {
			intxnDepth = thisInThatSpace.m_max.getX() - that.m_min.getX();
			intxnNormal = Normal(-thatToThisMat3.get(0, 0),
					-thatToThisMat3.get(1, 0), -thatToThisMat3.get(2, 0));
			intxnPoint = Vec3(
					thatToThisMat3.get(0, 0) < 0 ? m_min.getX() : m_max.getX(),
					thatToThisMat3.get(1, 0) < 0 ? m_min.getY() : m_max.getY(),
					thatToThisMat3.get(2, 0) < 0 ? m_min.getZ() : m_max.getZ());
		}
	}
	// 'that' y-axis
	if (that.m_min.getY() < thisInThatSpace.m_min.getY()) {
		if (that.m_max.getY() - thisInThatSpace.m_min.getY() < intxnDepth) {
			intxnDepth = that.m_max.getY() - thisInThatSpace.m_min.getY();
			intxnNormal = Normal(thatToThisMat3.get(0, 1),
					thatToThisMat3.get(1, 1), thatToThisMat3.get(2, 1));
			intxnPoint = Vec3(
					thatToThisMat3.get(0, 1) > 0 ? m_min.getX() : m_max.getX(),
					thatToThisMat3.get(1, 1) > 0 ? m_min.getY() : m_max.getY(),
					thatToThisMat3.get(2, 1) > 0 ? m_min.getZ() : m_max.getZ());
		}
	} else {
		if (thisInThatSpace.m_max.getY() - that.m_min.getY() < intxnDepth) {
			intxnDepth = thisInThatSpace.m_max.getY() - that.m_min.getY();
			intxnNormal = Normal(-thatToThisMat3.get(0, 1),
					-thatToThisMat3.get(1, 1), -thatToThisMat3.get(2, 1));
			intxnPoint = Vec3(
					thatToThisMat3.get(0, 1) < 0 ? m_min.getX() : m_max.getX(),
					thatToThisMat3.get(1, 1) < 0 ? m_min.getY() : m_max.getY(),
					thatToThisMat3.get(2, 1) < 0 ? m_min.getZ() : m_max.getZ());
		}
	}
	// 'that' z-axis
	if (that.m_min.getZ() < thisInThatSpace.m_min.getZ()) {
		if (that.m_max.getZ() - thisInThatSpace.m_min.getZ() < intxnDepth) {
			intxnDepth = that.m_max.getZ() - thisInThatSpace.m_min.getZ();
			intxnNormal = Normal(thatToThisMat3.get(0, 2),
					thatToThisMat3.get(1, 2), thatToThisMat3.get(2, 2));
			intxnPoint = Vec3(
					thatToThisMat3.get(0, 2) > 0 ? m_min.getX() : m_max.getX(),
					thatToThisMat3.get(1, 2) > 0 ? m_min.getY() : m_max.getY(),
					thatToThisMat3.get(2, 2) > 0 ? m_min.getZ() : m_max.getZ());
		}
	} else {
		if (thisInThatSpace.m_max.getZ() - that.m_min.getZ() < intxnDepth) {
			intxnDepth = thisInThatSpace.m_max.getZ() - that.m_min.getZ();
			intxnNormal = Normal(-thatToThisMat3.get(0, 2),
					-thatToThisMat3.get(1, 2), -thatToThisMat3.get(2, 2));
			intxnPoint = Vec3(
					thatToThisMat3.get(0, 2) < 0 ? m_min.getX() : m_max.getX(),
					thatToThisMat3.get(1, 2) < 0 ? m_min.getY() : m_max.getY(),
					thatToThisMat3.get(2, 2) < 0 ? m_min.getZ() : m_max.getZ());
		}
	}

	// edge cross
	Normal thisSpaceAxes[9] = {
			Normal(0, -thatToThisMat3.get(2, 0), thatToThisMat3.get(1, 0)),
			Normal(0, -thatToThisMat3.get(2, 1), thatToThisMat3.get(1, 1)),
			Normal(0, -thatToThisMat3.get(2, 2), thatToThisMat3.get(1, 2)),
			Normal(thatToThisMat3.get(2, 0), 0,	-thatToThisMat3.get(0, 0)),
			Normal(thatToThisMat3.get(2, 1), 0,	-thatToThisMat3.get(0, 1)),
			Normal(thatToThisMat3.get(2, 2), 0,	-thatToThisMat3.get(0, 2)),
			Normal(-thatToThisMat3.get(1, 0), thatToThisMat3.get(0, 0), 0),
			Normal(-thatToThisMat3.get(1, 1), thatToThisMat3.get(0, 1), 0),
			Normal(-thatToThisMat3.get(1, 2), thatToThisMat3.get(0, 2), 0) };
	Normal thatSpaceAxes[9] = {
			Normal(0, thisToThatMat3.get(2, 0), -thisToThatMat3.get(1, 0)),
			Normal(-thisToThatMat3.get(2, 0), 0, thisToThatMat3.get(0, 0)),
			Normal(thisToThatMat3.get(1, 0), -thisToThatMat3.get(0, 0), 0),
			Normal(0, thisToThatMat3.get(2, 1), -thisToThatMat3.get(1, 1)),
			Normal(-thisToThatMat3.get(2, 1), 0, thisToThatMat3.get(0, 1)),
			Normal(thisToThatMat3.get(1, 1), -thisToThatMat3.get(0, 1), 0),
			Normal(0, thisToThatMat3.get(2, 2),	-thisToThatMat3.get(1, 2)),
			Normal(-thisToThatMat3.get(2, 2), 0, thisToThatMat3.get(0, 2)),
			Normal(thisToThatMat3.get(1, 2), -thisToThatMat3.get(0, 2), 0) };
	// edges in 'this' space
	Normal thatEdges[3] = {
			Normal(thatToThisMat3.get(0, 0), thatToThisMat3.get(1, 0), thatToThisMat3.get(2, 0)),
			Normal(thatToThisMat3.get(0, 1), thatToThisMat3.get(1, 1), thatToThisMat3.get(2, 1)),
			Normal(thatToThisMat3.get(0, 2), thatToThisMat3.get(1, 2), thatToThisMat3.get(2, 2)) };
	// 'this' edge dot 'that' edge in 'this' space
	float edgeDot[9] = {
			thatToThisMat3.get(0, 0), thatToThisMat3.get(0, 1),	thatToThisMat3.get(0, 2),
			thatToThisMat3.get(1, 0), thatToThisMat3.get(1, 1), thatToThisMat3.get(1, 2),
			thatToThisMat3.get(2, 0), thatToThisMat3.get(2, 1),	thatToThisMat3.get(2, 2) };

	Vec3 thisMinPoint;
	Vec3 thisMaxPoint;
	Vec3 thatMinPoint;
	Vec3 thatMaxPoint;

	for (int i = 0; i < 9; ++i) {
		// parallel edges
		if (edgeDot[i] * edgeDot[i] > .99999) {
			continue;
		}

		const Normal & thisAxis = thisSpaceAxes[i];
		if (thisAxis.invalid()) {
			continue;
		}
		// extremes for axis
		getDirectionMinMax(thisAxis, thisMinPoint, thisMaxPoint);

		double thisMin = thisMinPoint.dot(thisAxis);
		double thisMax = thisMaxPoint.dot(thisAxis);

		// in 'that' space
		const Normal & thatAxis = thatSpaceAxes[i];

		// extremes for axis
		that.getDirectionMinMax(thatAxis, thatMinPoint, thatMaxPoint);

		// to this space
		thatToThis.transformPoint(thatMinPoint);
		thatToThis.transformPoint(thatMaxPoint);

		double thatMin = thatMinPoint.dot(thisAxis);
		double thatMax = thatMaxPoint.dot(thisAxis);

		// check for gap
		if (thisMin > thatMax || thatMin > thisMax) {
			return false;
		}

		// update
		if (thatMin < thisMin) {
			if (thatMax - thisMin < intxnDepth) {
				const Normal & u = directions[i / 3];
				const Normal & v = thatEdges[i % 3];

				// closest point
				Vec3 pq = thisMinPoint - thatMaxPoint;
				double s = edgeDot[i] * pq.dot(v)
						- pq.dot(u) / (1 - edgeDot[i] * edgeDot[i]);

				intxnDepth = thatMax - thisMin;
				intxnNormal = thisAxis;
				intxnPoint.scaleAdd(s, u, thisMinPoint);
			}
		} else {
			if (thisMax - thatMin < intxnDepth) {
				const Normal & u = directions[i / 3];
				const Normal & v = thatEdges[i % 3];

				// closest point
				Vec3 pq = thisMaxPoint - thatMinPoint;
				double s = edgeDot[i] * pq.dot(v)
						- pq.dot(u) / (1 - edgeDot[i] * edgeDot[i]);

				intxnDepth = thisMax - thatMin;
				intxnNormal = -thisAxis;
				intxnPoint.scaleAdd(s, u, thisMaxPoint);
			}
		}
	}
	intersection.set(intxnPoint, intxnNormal, intxnDepth);
	return true;
}

/**
 * does this bounds contain the other bounds
 * ie would this bounds union other bounds equal this bounds
 *
 * @param other  other bounds
 *
 * @return       true if other bounds in contained within this bounds,
 *               false otherwise
 */
bool BoundingBox::contains(const BoundingBox & other) const {
	return m_min.getX() <= other.m_min.getX()
			&& m_max.getX() >= other.m_max.getX()
			&& m_min.getY() <= other.m_min.getY()
			&& m_max.getY() >= other.m_max.getY()
			&& m_min.getZ() <= other.m_min.getZ()
			&& m_max.getZ() >= other.m_max.getZ();
}

/**
 * calculate centre of bounds
 *
 * @return  centre of bounds
 */
Vec3 BoundingBox::getCentre() const {
	return Vec3((m_min.getX() + m_max.getX()) * .5,
			(m_min.getY() + m_max.getY()) * .5,
			(m_min.getZ() + m_max.getZ()) * .5);
}

/**
 * minimum and maximum point in given direction
 *
 * @param direction  direction to get values for
 * @param minPoint   minimum point in direction
 * @param maxPoint   maximum point in direction
 */
void BoundingBox::getDirectionMinMax(const Normal & direction, Vec3 & minPoint,
		Vec3 & maxPoint) const {

	if (direction.getX() < 0) {
		minPoint.setX(m_max.getX());
		maxPoint.setX(m_min.getX());
	} else {
		minPoint.setX(m_min.getX());
		maxPoint.setX(m_max.getX());
	}

	if (direction.getY() < 0) {
		minPoint.setY(m_max.getY());
		maxPoint.setY(m_min.getY());
	} else {
		minPoint.setY(m_min.getY());
		maxPoint.setY(m_max.getY());
	}

	if (direction.getZ() < 0) {
		minPoint.setZ(m_max.getZ());
		maxPoint.setZ(m_min.getZ());
	} else {
		minPoint.setZ(m_min.getZ());
		maxPoint.setZ(m_max.getZ());
	}
}

/**
 * calculate extents of bounds
 * ie width, height, depth
 *
 * @return  extents of bounds
 */
Vec3 BoundingBox::getExtents() const {
	return Vec3(m_max.getX() - m_min.getX(), m_max.getY() - m_min.getY(),
			m_max.getZ() - m_min.getZ());
}

/**
 * get maximum point of bounds
 *
 * @return  maximum point of bounds
 */
const Vec3 & BoundingBox::getMax() const {
	return m_max;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr BoundingBox::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "__add__", std::make_shared<Add>() },
			{ "contains", std::make_shared<Contains>() },
			{ "getMax", std::make_shared<GetMax>() },
			{ "getMin", std::make_shared<GetMin>() },
			{ "getRadius", std::make_shared<GetRadius>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * get minimum point of bounds
 *
 * @return  minimum point of bounds
 */
const Vec3 & BoundingBox::getMin() const {
	return m_min;
}

/**
 * calculate radius of bounds
 *
 * @return  radius of bounds
 */
double BoundingBox::getRadius() const {
	return getExtents().length() / 2;
}

/**
 * calculate surface area of bounds
 *
 * @return  surface area of bounds
 */
double BoundingBox::getSurfaceArea() const {
	auto x = m_max.getX() - m_min.getX();
	auto y = m_max.getY() - m_min.getY();
	auto z = m_max.getZ() - m_min.getZ();

	return 2 * (x * y + y * z + z * x);
}

/**
 * calculate volume of bounds
 *
 * @return  volume of bounds
 */
double BoundingBox::getVolume() const {
	return (m_max.getX() - m_min.getX()) * (m_max.getY() - m_min.getY())
			* (m_max.getZ() - m_min.getZ());
}

/**
 * does this bounds intersect other bounds
 * ie does this bounds overlap other bounds
 *    it is not enough to share a face, edge or vertex
 *
 * @param other  bounds test against
 *
 * @return       true if this bounds intersects other bounds
 *               false otherwise
 */
bool BoundingBox::intersects(const BoundingBox & other) const {
	return m_min.getX() < other.m_max.getX()
			&& other.m_min.getX() < m_max.getX()
			&& m_min.getY() < other.m_max.getY()
			&& other.m_min.getY() < m_max.getY()
			&& m_min.getZ() < other.m_max.getZ()
			&& other.m_min.getZ() < m_max.getZ();
}

/**
 * is bounding box empty
 *
 * @return  true if empty bounds, false otherwise
 */
bool BoundingBox::isEmpty() const {
	return m_min.getX() > m_max.getX() || m_min.getY() > m_max.getY()
			|| m_min.getZ() > m_max.getZ();
}

/**
 * add other bounds to this bounds
 *
 * @param other  bounds to form union with
 */
BoundingBox & BoundingBox::operator+=(const BoundingBox & other) {
	m_min.set(std::min(m_min.getX(), other.m_min.getX()),
			std::min(m_min.getY(), other.m_min.getY()),
			std::min(m_min.getZ(), other.m_min.getZ()));

	m_max.set(std::max(m_max.getX(), other.m_max.getX()),
			std::max(m_max.getY(), other.m_max.getY()),
			std::max(m_max.getZ(), other.m_max.getZ()));

	return *this;
}

/**
 * add point to this bounds
 *
 * @param point  point to form union with
 */
BoundingBox & BoundingBox::operator+=(const Vec3 & point) {
	m_min.set(std::min(m_min.getX(), point.getX()),
			std::min(m_min.getY(), point.getY()),
			std::min(m_min.getZ(), point.getZ()));

	m_max.set(std::max(m_max.getX(), point.getX()),
			std::max(m_max.getY(), point.getY()),
			std::max(m_max.getZ(), point.getZ()));

	return *this;
}

/**
 * union of this this and other bounds
 *
 * @param other  other bounds
 *
 * @return       union of bounds
 */
BoundingBox BoundingBox::operator+(const BoundingBox & other) const {
	return BoundingBox(
			Vec3(std::min(m_min.getX(), other.m_min.getX()),
					std::min(m_min.getY(), other.m_min.getY()),
					std::min(m_min.getZ(), other.m_min.getZ())),
			Vec3(std::max(m_max.getX(), other.m_max.getX()),
					std::max(m_max.getY(), other.m_max.getY()),
					std::max(m_max.getZ(), other.m_max.getZ())));
}

/**
 * Intersection against ray, tmin & tmax are the extents of the ray
 * and are truncated after intersection, if there is no intersection
 * this may possibly create a negative extents ie tmin > tmax
 *
 * @param ray   ray to insect with
 * @param tmin  minimum extent of ray
 * @param tmax  maximum extent of ray
 *
 * @return      result of intersection
 */
bool BoundingBox::rayIntersect(const Ray & ray, double & tmin,
		double & tmax) const {
	if (rayIntersectX(ray, tmin, tmax) == false) {
		return false;
	}

	if (rayIntersectY(ray, tmin, tmax) == false) {
		return false;
	}

	if (rayIntersectZ(ray, tmin, tmax) == false) {
		return false;
	}

	return true;
}

/**
 * Intersection against ray. Negative ray intersections are discarded. If
 * ray starts outside box, intersection is at entry. If ray starts in box,
 * intersection is at exit.
 *
 * @param ray       ray to insect with
 * @param point     written if intersection occurs, returns intersection point on
 *                  surface of bounding box
 * @param distance  written if intersection occurs, returns distance along ray to
 *                  intersection point on the surface on the bounding box
 *
 * @return          result of intersection
 */
bool BoundingBox::rayIntersection(const Ray & ray, Vec3 & point,
		double & distance) const {
	double tmin = 0;
	double tmax = std::numeric_limits<double>::max();

	if (rayIntersectX(ray, tmin, tmax) == false) {
		return false;
	}

	if (rayIntersectY(ray, tmin, tmax) == false) {
		return false;
	}

	if (rayIntersectZ(ray, tmin, tmax) == false) {
		return false;
	}

	if (tmin > 0) {
		point.scaleAdd(tmin, ray.getDirection(), ray.getStart());
		distance = tmin;
	} else {
		point.scaleAdd(tmax, ray.getDirection(), ray.getStart());
		distance = tmax;
	}

	return true;
}

/**
 * calculate string representation of bounds
 *
 * @return  bounds as string
 */
OVERRIDE std::string BoundingBox::toString() const {
	return "BoundigBox(min=" + m_min.toString() + " max=" + m_max.toString()
			+ ")";
}

/**
 * transform bounding box
 *
 * @param transform  transform for bounding box
 *
 * @return           transformed bounding box
 */
BoundingBox BoundingBox::transformed(const Transform & transform) const {
	auto m = transform.getRotationMatrix();
	double m00 = m.get(0, 0);
	double m01 = m.get(0, 1);
	double m02 = m.get(0, 2);
	double m10 = m.get(1, 0);
	double m11 = m.get(1, 1);
	double m12 = m.get(1, 2);
	double m20 = m.get(2, 0);
	double m21 = m.get(2, 1);
	double m22 = m.get(2, 2);

	auto minx = std::min(m00 * m_min.getX(), m00 * m_max.getX());
	auto miny = std::min(m10 * m_min.getX(), m10 * m_max.getX());
	auto minz = std::min(m20 * m_min.getX(), m20 * m_max.getX());

	auto maxx = std::max(m00 * m_min.getX(), m00 * m_max.getX());
	auto maxy = std::max(m10 * m_min.getX(), m10 * m_max.getX());
	auto maxz = std::max(m20 * m_min.getX(), m20 * m_max.getX());

	minx += std::min(m01 * m_min.getY(), m01 * m_max.getY());
	miny += std::min(m11 * m_min.getY(), m11 * m_max.getY());
	minz += std::min(m21 * m_min.getY(), m21 * m_max.getY());

	maxx += std::max(m01 * m_min.getY(), m01 * m_max.getY());
	maxy += std::max(m11 * m_min.getY(), m11 * m_max.getY());
	maxz += std::max(m21 * m_min.getY(), m21 * m_max.getY());

	minx += std::min(m02 * m_min.getZ(), m02 * m_max.getZ());
	miny += std::min(m12 * m_min.getZ(), m12 * m_max.getZ());
	minz += std::min(m22 * m_min.getZ(), m22 * m_max.getZ());

	maxx += std::max(m02 * m_min.getZ(), m02 * m_max.getZ());
	maxy += std::max(m12 * m_min.getZ(), m12 * m_max.getZ());
	maxz += std::max(m22 * m_min.getZ(), m22 * m_max.getZ());

	auto translation = transform.getTranslation();

	return BoundingBox(Vec3(minx, miny, minz) + translation,
			Vec3(maxx, maxy, maxz) + translation);
}

/**
 * get empty bounding box
 *
 * @return  empty bounding box
 */
STATIC const BoundingBox & BoundingBox::empty() {
	static BoundingBox empty;
	return empty;
}

/**
 * get script object factory for BoundingBox
 *
 * @return  BoundingBox factory
 */
STATIC const ScriptObjectPtr & BoundingBox::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}

/**
 * construct empty bounding box
 */
PRIVATE BoundingBox::BoundingBox() :
				m_min(
						Vec3(std::numeric_limits<double>::max(),
								std::numeric_limits<double>::max(),
								std::numeric_limits<double>::max())),
				m_max(
						Vec3(-std::numeric_limits<double>::max(),
								-std::numeric_limits<double>::max(),
								-std::numeric_limits<double>::max())) {
}

/**
 * Intersection against ray in x-axis only, tmin & tmax are the extents
 * of the ray and are truncated after intersection, if there is no
 * intersection this may possibly create a negative extents ie tmin > tmax
 *
 * @param ray   ray to insect with
 * @param tmin  minimum extent of ray
 * @param tmax  maximum extent of ray
 *
 * @return      result of intersection
 */
PRIVATE bool BoundingBox::rayIntersectX(const Ray & ray, double & tmin,
		double & tmax) const {
	float x = ray.getDirection().getX();
	if (std::abs(x) > epsilon) {
		double t1 = (m_max.getX() - ray.getStart().getX()) / x;
		double t2 = (m_min.getX() - ray.getStart().getX()) / x;
		tmin = std::max(std::min(t1, t2), tmin);
		tmax = std::min(std::max(t1, t2), tmax);
		if (tmin > tmax) {
			return false;
		}
	} else if (ray.getStart().getX() > m_max.getX()
			|| ray.getStart().getX() < m_min.getX()) {
		return false;
	}
	return true;
}

/**
 * Intersection against ray in y-axis only, tmin & tmax are the extents
 * of the ray and are truncated after intersection, if there is no
 * intersection this may possibly create a negative extents ie tmin > tmax
 *
 * @param ray   ray to insect with
 * @param tmin  minimum extent of ray
 * @param tmax  maximum extent of ray
 *
 * @return      result of intersection
 */
PRIVATE bool BoundingBox::rayIntersectY(const Ray & ray, double & tmin,
		double & tmax) const {
	float y = ray.getDirection().getY();
	if (std::abs(y) > epsilon) {
		double t1 = (m_max.getY() - ray.getStart().getY()) / y;
		double t2 = (m_min.getY() - ray.getStart().getY()) / y;
		tmin = std::max(std::min(t1, t2), tmin);
		tmax = std::min(std::max(t1, t2), tmax);
		if (tmin > tmax) {
			return false;
		}
	} else if (ray.getStart().getY() > m_max.getY()
			|| ray.getStart().getY() < m_min.getY()) {
		return false;
	}
	return true;
}

/**
 * Intersection against ray in z-axis only, tmin & tmax are the extents
 * of the ray and are truncated after intersection, if there is no
 * intersection this may possibly create a negative extents ie tmin > tmax
 *
 * @param ray   ray to insect with
 * @param tmin  minimum extent of ray
 * @param tmax  maximum extent of ray
 *
 * @return      result of intersection
 */
PRIVATE bool BoundingBox::rayIntersectZ(const Ray & ray, double & tmin,
		double & tmax) const {
	float z = ray.getDirection().getZ();
	if (std::abs(z) > epsilon) {
		double t1 = (m_max.getZ() - ray.getStart().getZ()) / z;
		double t2 = (m_min.getZ() - ray.getStart().getZ()) / z;
		tmin = std::max(std::min(t1, t2), tmin);
		tmax = std::min(std::max(t1, t2), tmax);
		if (tmin > tmax) {
			return false;
		}
	} else if (ray.getStart().getZ() > m_max.getZ()
			|| ray.getStart().getZ() < m_min.getZ()) {
		return false;
	}
	return true;
}
