#pragma once

#include "convexHull.h"
#include "mat4.h"
#include "ray.h"
#include "vec3Array.h"

class Vec2;

class Camera {
public:
	/**
	 * constructor
	 *
	 * @param name         name of camera
	 * @param near         near distance
	 * @param far          far distance
	 * @param aspectRatio  aspect ratio
	 */
	inline Camera(const std::string & name, float near, float far,
			float aspectRatio, const Mat4 & projectionMatrix,
			const ConvexHull & convexHull) :
					name(name),
					near(near),
					far(far),
					aspectRatio(aspectRatio),
					projectionMatrix(projectionMatrix),
					convexHull(convexHull) {
	}

	/**
	 * destructor
	 */
	inline virtual ~Camera() {
	}

	/**
	 * clone camera with new name
	 *
	 * @param name  new name
	 *
	 * @return      new camera
	 */
	virtual std::shared_ptr<Camera> clone(const std::string & name) const = 0;

	/**
	 * get camera aspect ratio
	 *
	 * @return  aspect ratio
	 */
	inline float getAspectRatio() const {
		return aspectRatio;
	}

	/**
	 * get convex hull of camera
	 *
	 * @return  convex hull
	 */
	inline const ConvexHull & getConvexHull() const {
		return convexHull;
	}

	/**
	 * get corners of camera within z range
	 *
	 * @param nearZ  near z corners
	 * @param farZ   far x corners
	 *
	 * @return       corners within z range
	 */
	virtual Vec3Array getCorners(double nearZ, double farZ) const = 0;

	/**
	 * get camera far distance
	 *
	 * @return  far distance
	 */
	inline float getFar() const {
		return far;
	}

	/**
	 * get camera name
	 *
	 * @return  name of camera
	 */
	inline const std::string & getName() const {
		return name;
	}

	/**
	 * get camera near distance
	 *
	 * @return  near distance
	 */
	inline float getNear() const {
		return near;
	}

	/**
	 * get camera projection matrix
	 *
	 * @return  projection matrix
	 */
	inline const Mat4 & getProjectionMatrix() const {
		return projectionMatrix;
	}

	/**
	 * get ray based on 2d point at camera near
	 *
	 * @param pos  position on near plane
	 *
	 * @return     camera space ray
	 */
	virtual Ray getRay(const Vec2 & pos) const = 0;

	/**
	 * transform projection matrix, proj' = proj * m
	 *
	 * @param m  matrix to transform projection by
	 */
	inline void transform(const Mat4 & m) {
		projectionMatrix.mul(m);
	}

protected:
	std::string name;
	float near;
	float far;
	float aspectRatio;
	Mat4 projectionMatrix;
	ConvexHull convexHull;
};
