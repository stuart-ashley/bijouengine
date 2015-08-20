#pragma once

#include "../scripting/scriptObject.h"

#include <string>
#include <vector>

class Bezier;
class Transform;

class Bone: public ScriptObject {
public:

	/**
	 * constructor
	 *
	 * @param name    name of bone
	 * @param curves  list of curves for bone
	 */
	Bone(const std::string & name, const std::vector<Bezier> & curves);

	/**
	 * destructor
	 */
	~Bone();

	/**
	 * get curve at given index
	 *
	 * @param idx  index to get
	 *
	 * @return     curve at index
	 */
	const Bezier & get(int idx) const;

	/**
	 * get bone name
	 *
	 * @return  name of bone
	 */
	const std::string & getName();

	/**
	 *
	 * @param current  current transform
	 * @param frame    current frame
	 * @param d        frame step
	 *
	 * @return         updated transform
	 */
	Transform getTransform(const Transform & current, float frame,
			float d) const;

	/**
	 * get number of curves
	 *
	 * @return  number of curves
	 */
	size_t size() const;

	/**
	 * validate bone (ie check loaded)
	 *
	 * @return  true if valid, false otherwise
	 */
	bool validate() const;

	/**
	 * get script object factory for Bone
	 *
	 * @return  Bone factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

