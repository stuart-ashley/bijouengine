#include "sgSkinningMatrices.h"

#include "updateState.h"

#include "../core/skinningMatrix.h"
#include "../core/transform.h"

#include "../render/renderState.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

namespace {

	/*
	 *
	 */
	struct Factory: public Executable {
		void execute(const ScriptObjectPtr &, unsigned int nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			std::vector<SkinningMatrix> matrices;
			for (unsigned int i = 0; i < nArgs; ++i) {
				matrices.emplace_back(getArg<SkinningMatrix>("SkinningMatrix", stack, i));
			}

			stack.push(std::make_shared<SgSkinningMatrices>(matrices));
		}
	};
}

struct SgSkinningMatrices::impl {
	std::vector<SkinningMatrix> m_matrices;

	impl(const std::vector<SkinningMatrix> & matrices) :
			m_matrices(matrices) {
	}
};

/**
 * constructor
 */
SgSkinningMatrices::SgSkinningMatrices(
		const std::vector<SkinningMatrix> & matrices) :
		pimpl(new impl(matrices)) {
}

/**
 * destructor
 */
SgSkinningMatrices::~SgSkinningMatrices(){
}

/**
 *
 */
OVERRIDE void SgSkinningMatrices::update(UpdateState & state) {
	for (auto & m : pimpl->m_matrices) {
		Transform hierarchy;
		if (m.getParent() >= 0) {
			hierarchy =
					pimpl->m_matrices.at(m.getParent()).getHierarchyTransform();
		}
		hierarchy.transform(m.getFromParent());
		auto bone = state.getBoneTransform(m.getBone());
		hierarchy.transform(bone);
		m.setHierarchyTransform(hierarchy);

		auto final = hierarchy;
		final.transform(m.getToRestPose());
		m.setFinalTransform(final);
	}
}

/**
 *
 */
OVERRIDE void SgSkinningMatrices::visualize(render::ViewBuilder & vb) {
	for (const auto & m : pimpl->m_matrices) {
		vb.getState().setSkinningMatrix(m.getIndex(), m.getFinalTransform());
	}
}

/**
 * get script object factory for SgSkinningMatrices
 *
 * @return  SgSkinningMatrices factory
 */
STATIC ScriptObjectPtr SgSkinningMatrices::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
