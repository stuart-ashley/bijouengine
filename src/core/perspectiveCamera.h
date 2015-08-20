#pragma once

#include "camera.h"

class PerspectiveCamera: public Camera {
public:
	/**
	 * Construct a projective camera. Projection matrix generated is identical
	 * to that produced by gluPerspective
	 *
	 * @param name           name of camera
	 * @param fovyInDegrees  field of view in degrees
	 * @param aspectRatio    aspect ratio of camera, horizontal / vertical length
	 * @param near           near plane
	 * @param far            far plane
	 */
	PerspectiveCamera(const std::string & name, float fovyInDegrees,
			float aspectRatio, float near, float far);

	std::shared_ptr<Camera> clone(const std::string & name) const override;

	Vec3Array getCorners(double nearZ, double farZ) const override;

	inline float getFovyInDegrees() const {
		return fovyInDegrees;
	}

	Ray getRay(const Vec2 & pos) const override;

	inline float getTanHalfFovy() const {
		return tanHalfFovy;
	}
private:
	float fovyInDegrees;
	float tanHalfFovy;
};

