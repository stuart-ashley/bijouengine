#include "skinningMatrix.h"

#include "transform.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <cassert>

namespace {
	/*
	 *
	 */
	std::vector<BaseParameter> matrixParams = {
			Parameter<String>("bone", nullptr),
			Parameter<Real>("index", nullptr),
			Parameter<Real>("parent", std::make_shared<Real>(-1)),
			Parameter<Transform>("fromParent", nullptr),
			Parameter<Transform>("toRestPose", nullptr) };

	/*
	 *
	 */
	struct Factory: public Executable {
		Parameters parameters;

		Factory() :
				parameters(matrixParams) {
		}

		void execute(const ScriptObjectPtr &, unsigned int nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);

			auto bone = std::static_pointer_cast<String>(args["bone"]);
			auto index = std::static_pointer_cast<Real>(args["index"]);
			auto parent = std::static_pointer_cast<Real>(args["parent"]);
			auto fromParent = std::static_pointer_cast<Transform>(
					args["fromParent"]);
			auto toRestPose = std::static_pointer_cast<Transform>(
					args["toRestPose"]);

			assert(index->getInt32() > parent->getInt32());

			stack.push(
					std::make_shared<SkinningMatrix>(bone->getValue(),
							index->getInt32(), parent->getInt32(), *fromParent,
							*toRestPose));
		}
	};
}

struct SkinningMatrix::impl {
	std::string m_bone;
	int m_index;
	int m_parent;
	Transform m_fromParent;
	Transform m_toRestPose;
	Transform m_hierarchy;
	Transform m_transform;

	impl(const std::string & bone, int index, int parent,
			const Transform & fromParent, const Transform & toRestPose) :
					m_bone(bone),
					m_index(index),
					m_parent(parent),
					m_fromParent(fromParent),
					m_toRestPose(toRestPose) {
	}

	impl(const impl & other) :
					m_bone(other.m_bone),
					m_index(other.m_index),
					m_parent(other.m_parent),
					m_fromParent(other.m_fromParent),
					m_toRestPose(other.m_toRestPose) {
	}

};

/**
 *
 * @param bone
 * @param index
 * @param parent
 * @param fromParent
 * @param toRestPose
 */
SkinningMatrix::SkinningMatrix(const std::string & bone, int index, int parent,
		const Transform & fromParent, const Transform & toRestPose) :
		pimpl(new impl(bone, index, parent, fromParent, toRestPose)) {
}

/**
 *
 * @param other
 */
SkinningMatrix::SkinningMatrix(const SkinningMatrix & other) :
		pimpl(new impl(*other.pimpl)) {
}

/**
 *
 */
SkinningMatrix::~SkinningMatrix() {
}

/**
 *
 * @return
 */
const std::string & SkinningMatrix::getBone() const {
	return pimpl->m_bone;
}

/**
 *
 * @return
 */
const Transform & SkinningMatrix::getFinalTransform() const {
	return pimpl->m_transform;
}

/**
 *
 * @return
 */
const Transform & SkinningMatrix::getFromParent() const {
	return pimpl->m_fromParent;
}

/**
 *
 * @return
 */
const Transform & SkinningMatrix::getHierarchyTransform() const {
	return pimpl->m_hierarchy;
}

/**
 *
 * @return
 */
int SkinningMatrix::getIndex() const {
	return pimpl->m_index;
}

/**
 *
 * @return
 */
int SkinningMatrix::getParent() const {
	return pimpl->m_parent;
}

/**
 *
 * @return
 */
const Transform & SkinningMatrix::getToRestPose() const {
	return pimpl->m_toRestPose;
}

/**
 *
 * @param transform
 */
void SkinningMatrix::setFinalTransform(const Transform & transform) {
	pimpl->m_transform = transform;
}

/**
 *
 * @param transform
 */
void SkinningMatrix::setHierarchyTransform(const Transform & transform) {
	pimpl->m_hierarchy = transform;
}

/**
 * get script object factory for SkinningMatrix
 *
 * @return  SkinningMatrix factory
 */
STATIC ScriptObjectPtr SkinningMatrix::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
