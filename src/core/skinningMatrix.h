#pragma once

#include "../scripting/scriptObject.h"

class Transform;

class SkinningMatrix: public ScriptObject {
public:

	/**
	 *
	 * @param bone
	 * @param index
	 * @param parent
	 * @param fromParent
	 * @param toRestPose
	 */
	SkinningMatrix(const std::string & bone, int index, int parent,
			const Transform & fromParent, const Transform & toRestPose);

	/**
	 *
	 * @param other
	 */
	SkinningMatrix(const SkinningMatrix & other);

	/**
	 *
	 */
	~SkinningMatrix();

	/**
	 *
	 * @return
	 */
	const std::string & getBone() const;

	/**
	 *
	 * @return
	 */
	const Transform & getFinalTransform() const;

	/**
	 *
	 * @return
	 */
	const Transform & getFromParent() const;

	/**
	 *
	 * @return
	 */
	const Transform & getHierarchyTransform() const;

	/**
	 *
	 * @return
	 */
	int getIndex() const;

	/**
	 *
	 * @return
	 */
	int getParent() const;

	/**
	 *
	 * @return
	 */
	const Transform & getToRestPose() const;

	/**
	 *
	 * @param transform
	 */
	void setFinalTransform(const Transform & transform);

	/**
	 *
	 * @param transform
	 */
	void setHierarchyTransform(const Transform & transform);

	/**
	 * get script object factory for SkinningMatrix
	 *
	 * @return  SkinningMatrix factory
	 */
	static ScriptObjectPtr getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

