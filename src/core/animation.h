#pragma once

#include "../scripting/scriptObject.h"

#include <unordered_map>

class Bone;

class Animation: public ScriptObject {
public:

	/**
	 * constructor
	 *
	 * @param name
	 * @param sFrame
	 * @param eFrame
	 * @param offset
	 * @param fps
	 * @param bones
	 */
	Animation(const std::string & name, float sFrame, float eFrame,
			float offset, float fps,
			const std::unordered_map<std::string, Bone> & bones);

	/**
	 * destructor
	 */
	~Animation();

	/**
	 * Get first animation element
	 *
	 * @return first animation element
	 */
	const Bone & getBone() const;

	/**
	 * get named bone
	 *
	 * @param name  name of bone to get
	 *
	 * @return      named bone
	 */
	const Bone & getBone(const std::string & name) const;

	/**
	 * Get animation elements
	 *
	 * @return list of animation elements
	 */
	const std::unordered_map<std::string, Bone> & getBones() const;

	/**
	 * get end frame of animation
	 *
	 * @return  end frame
	 */
	float getEndFrame() const;

	/**
	 * get animation speed in frames per second
	 *
	 * @return  frames per second
	 */
	float getFPS() const;

	/**
	 * get frame step from delta time
	 *
	 * @param dt  delta time in seconds
	 *
	 * @return    frame step
	 */
	float getFrameStep(float dt) const;

	/**
	 * get animation name
	 *
	 * @return  name of animation
	 */
	const std::string & getName() const;

	/**
	 * get animation offset
	 *
	 * @return  offset of animation
	 */
	float getOffset() const;

	/**
	 * get start frame of animation
	 *
	 * @return  start frame
	 */
	float getStartFrame() const;

	/**
	 * check existence of named bone
	 *
	 * @param name  name of bone to check
	 *
	 * @return      true if bone exists, false otherwise
	 */
	bool hasBone(const std::string & name) const;

	/**
	 * get length of animation
	 *
	 * @return  length of animation
	 */
	float length() const;

	/**
	 * calculate next animation frame, wrap as necessary
	 *
	 * @param frame  current animation frame
	 * @param dt     delta time in seconds
	 *
	 * @return       next animation frame
	 */
	float nextFrame(float frame, float dt) const;

	/**
	 * Validate elements
	 *
	 * @return true if all elements valid, false otherwise
	 */
	bool validate() const;

	/**
	 * get script object factory for Animation
	 *
	 * @return  Animation factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

