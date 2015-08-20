#pragma once

#include "transform.h"

class EndEffector {
public:

	EndEffector(const std::string & parent, const Transform & pivot) :
			parent(parent), pivot(pivot), convergence(10) {
	}

	inline float getConvergence() const {
		return convergence;
	}

	inline const Quat & getGoalRotation() const {
		return goalRotation;
	}

	inline const Vec3 & getGoalTranslation() const {
		return goalTranslation;
	}

	inline const std::string & getParent() const {
		return parent;
	}

	inline const Transform & getPivot() const {
		return pivot;
	}

	inline void setConvergence(float c) {
		convergence = c;
	}

	inline void setGoalRotation(const Quat & rotation) {
		goalRotation = rotation;
	}

	inline void setGoalTranslation(const Vec3 & translation) {
		goalTranslation = translation;
	}

private:
	std::string parent;
	Transform pivot;
	Vec3 goalTranslation;
	Quat goalRotation;
	float convergence;
};

