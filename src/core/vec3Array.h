#pragma once

#include "../scripting/scriptObject.h"

#include <initializer_list>
#include <memory>
#include <vector>

class BinaryFile;
class BoundingBox;
class Mat4;
class Normal;
class Quat;
class Transform;
class Vec3;

class Vec3Array: public ScriptObject {
public:

	Vec3Array(const std::initializer_list<Vec3> & vertices);

	Vec3Array(const std::vector<Vec3> & vertices);

	Vec3Array(const std::shared_ptr<BinaryFile> & binaryFile, int offset,
			int count);

	~Vec3Array();

	/**
	 * add vertex to array
	 *
	 * @param v  vertex to add
	 */
	void add(const Vec3 & v);

	std::vector<Vec3>::const_iterator begin() const;

	std::vector<Vec3>::const_iterator end() const;

	const Vec3 & get(size_t idx) const;

	/**
	 * calculate bounds
	 *
	 * @return  bounds
	 */
	const BoundingBox & getBounds() const;

	/**
	 * minimum and maximum distance in given direction
	 *
	 * @param direction  direction to get values for
	 * @param minDist    minimum distance in direction
	 * @param maxDist    maximum distance in direction
	 */
	void getDirectionMinMax(const Normal & direction, double & minDist,
			double & maxDist) const;

	/**
	 * minimum and maximum point and distance in given direction
	 *
	 * @param direction  direction to get values for
	 * @param minPoint   minimum point in direction
	 * @param maxPoint   maximum point in direction
	 * @param minDist    minimum distance in direction
	 * @param maxDist    maximum distance in direction
	 */
	void getDirectionMinMax(const Normal & direction, Vec3 & minPoint,
			Vec3 & maxPoint, double & minDist, double & maxDist) const;

	/**
	 * Produce a rotated version of this array
	 *
	 * @param rotation  amount to rotate
	 * @return          rotated array
	 */
	Vec3Array rotated(const Quat & rotation) const;

	/**
	 * produce scaled version of vertices
	 *
	 * @param s  scale factor
	 *
	 * @return   scaled vertices
	 */
	Vec3Array scaled(float s) const;

	/**
	 * get number of vertices in array
	 *
	 * @return  number of vertices
	 */
	size_t size() const;

	/**
	 *
	 * @return
	 */
	std::string toString() const override;

	/**
	 * Produce transformed version of vertices by matrix
	 *
	 * @param matrix  transformation matrix
	 *
	 * @return        transformed vertices
	 */
	Vec3Array transformed(const Mat4 & m) const;

	/**
	 * Produce transformed version of vertices by transform
	 *
	 * @param transform  transformation
	 *
	 * @return           transformed vertices
	 */
	Vec3Array transformed(const Transform & transform) const;

	/**
	 * If the vertices are from a file make sure it's loaded
	 *
	 * @return  true if loaded, false otherwise
	 */
	bool validate() const;

	/**
	 * get script object factory for Vec3Array
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            Vec3Array factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};
