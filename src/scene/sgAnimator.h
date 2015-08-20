#pragma once

#include "updateNode.h"

#include "../scripting/scriptObject.h"

#include <string>
#include <unordered_map>
#include <vector>

class Animation;
class AnimBlend;
class Quat;
class Transform;
class Vec3;

class SgAnimator: public ScriptObject, public UpdateNode {
public:

	/**
	 * Constructor
	 *
	 * @param animations
	 */
	SgAnimator(const Animation & active);

	/**
	 * destructor
	 */
	~SgAnimator();

	/**
	 *
	 * @return
	 */
	Vec3 getDeltaTranslation(const std::string & bone, float dt) const;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const override;

	/**
	 * get rotation of named bone
	 *
	 * @param bone  name of bone
	 *
	 * @return      rotation for requested bone
	 */
	Quat getRotation(const std::string & bone) const;

	/**
	 *
	 * @return
	 */
	const std::unordered_map<std::string, Transform> & getTransforms() const;

	/**
	 * get translation of named bone
	 *
	 * @param bone  name of bone
	 *
	 * @return      translation for requested bone
	 */
	Vec3 getTranslation(const std::string & bone) const;

	/**
	 *
	 * @param bone
	 * @param rotation
	 */
	bool rotate(const std::string & bone, const Quat & rotation);

	/**
	 *
	 * @param name
	 * @return
	 */
	void setAnimation(const Animation & animation);

	/**
	 *
	 * @param bone
	 * @param rotation
	 */
	void setRotation(const std::string & bone, const Quat & rotation);

	/**
	 *
	 * @param bone
	 * @param translation
	 */
	void setTranslation(const std::string & bone, const Vec3 & translation);

	/**
	 *
	 * @param state
	 */
	void update(UpdateState & state) override;

	/**
	 * get script object factory for SgAnimator
	 *
	 * @return  SgAnimator factory
	 */
	static ScriptObjectPtr getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

