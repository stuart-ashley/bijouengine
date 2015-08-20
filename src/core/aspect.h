#pragma once

#include "camera.h"
#include "convexHull.h"
#include "mat4.h"
#include "transform.h"

class Aspect {
public:

	inline Aspect(const std::shared_ptr<Camera> & camera,
			const Transform & rotTrans) :
					m_camera(camera),
					m_rotTrans(rotTrans),
					m_transform(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1),
					m_convexHull(camera->getConvexHull()) {
	}

	/**
	 *
	 * @param plane
	 */
	void addClipPlane(const Plane & plane);

	/**
	 *
	 * @param m
	 */
	void transform(const Mat4 & m);

	/**
	 *
	 * @param plane
	 */
	void mirror(const Plane & plane);

	inline const ConvexHull & getConvexHull() const {
		return m_convexHull;
	}

	inline Mat4 getInverseTransform() const {
		return m_transform.inverse();
	}

	inline const Mat4 & getTransform() const {
		return m_transform;
	}

	inline const Transform & getRotTrans() const {
		return m_rotTrans;
	}

	inline const std::shared_ptr<Camera> & getCamera() const {
		return m_camera;
	}

	inline const std::vector<Plane> & getClipPlanes() const {
		return m_clipPlanes;
	}

private:
	std::shared_ptr<Camera> m_camera;
	Transform m_rotTrans;
	Mat4 m_transform;
	std::vector<Plane> m_clipPlanes;
	ConvexHull m_convexHull;
};

