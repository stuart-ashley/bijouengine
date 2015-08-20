#pragma once

#include "camera.h"

class OrthographicCamera: public Camera {
public:

	/**
	 * Construct an orthographic camera. Projection matrix generated is
	 * identical to one produced by glOrtho
	 *
	 * @param name    name of camera
	 * @param left    left extents of view
	 * @param right   right extents of view
	 * @param bottom  bottom extent
	 * @param top     top extent
	 * @param near    near plane
	 * @param far     far plane
	 */
	OrthographicCamera(const std::string & name, float left, float right,
			float bottom, float top, float near, float far);

	Ray getRay(const Vec2 & pos) const override;

	Vec3Array getCorners(double nearZ, double farZ) const override;

	std::shared_ptr<Camera> clone(const std::string & name) const override;

private:
	float left;
	float right;
	float bottom;
	float top;
};

