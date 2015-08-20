#include "sgIndexedTriangles.h"

#include "../core/aspect.h"
#include "../core/binaryFileCache.h"
#include "../core/indexArray.h"
#include "../core/transform.h"
#include "../core/vec3.h"

#include "../render/blendFlag.h"
#include "../render/indexedTriangles.h"
#include "../render/renderState.h"
#include "../render/view.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

using namespace render;

namespace {
	std::vector<BaseParameter> params = { Parameter<String>("file", nullptr),
			Parameter<Real>("offset", nullptr), Parameter<Real>("indexCount",
					nullptr) };
	/*
	 *
	 */
	class Factory: public Executable {
	public:
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			if (typeid(*stack.top()) == typeid(List)) {
				auto list = getArg<List>("list", stack, 1);

				std::vector<int16_t> indices;
				indices.reserve(list.size());

				for (const auto & e : list) {
					scriptExecutionAssertType<Real>(e,
							"Require 16 bit integer list");

					auto num = std::static_pointer_cast<Real>(e);
					scriptExecutionAssert(num->isInt16(),
							"Require 16 bit integer list");

					indices.emplace_back(num->getInt16());
				}

				stack.push(
						std::make_shared<SgIndexedTriangles>(
								IndexedTriangles(IndexArray(indices))));
			} else {
				auto args = parameters.getArgs(nArgs, stack);

				auto file = std::static_pointer_cast<String>(args["file"]);
				auto offset = std::static_pointer_cast<Real>(args["offset"]);
				auto count = std::static_pointer_cast<Real>(args["indexCount"]);

				scriptExecutionAssert(offset->isInt32(),
						"Require integer offset");

				scriptExecutionAssert(count->isInt32(),
						"Require integer indexCount");

				IndexedTriangles triangles(
						IndexArray(
								BinaryFileCache::get(currentDir,
										file->getValue()), offset->getInt32(),
								count->getInt32()));

				stack.push(std::make_shared<SgIndexedTriangles>(triangles));
			}
		}
	};
}

/**
 * Construct from index array
 */
SgIndexedTriangles::SgIndexedTriangles(const render::IndexedTriangles & indices) :
		indices(indices), valid(false) {
}

/**
 * Add index array to render graph
 */
void SgIndexedTriangles::visualize(render::ViewBuilder & vb) {
	if (valid == false) {
		valid = indices.validate();
		if (valid == false) {
			return;
		}
	}

	vb.incPolyCount(indices.numTriangles());

	auto & state = vb.getState();

	switch (state.getBlend()) {
	case BlendFlag::NONE:
		vb.addIndexedTriangles(indices);
		break;
	default:
		auto worldToCamera = state.getTransform().to(
				vb.getView()->getAspect().getRotTrans());
		Vec3 t;
		worldToCamera.transformPoint(t);
		vb.addAlphaIndexedTriangles(indices, static_cast<float>(-t.getZ()));
		break;
	}
}

/**
 * get script object factory for SgIndexedTriangles
 *
 * @param currentDir  current directory of caller
 *
 * @return            SgIndexedTriangles factory
 */
STATIC ScriptObjectPtr SgIndexedTriangles::getFactory(
		const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
