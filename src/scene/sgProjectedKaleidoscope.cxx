#include "sgProjectedKaleidoscope.h"

#include "builder.h"
#include "lights.h"

#include "../core/vec2.h"

#include "../render/texture.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

using namespace render;

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {

	Parameter<Texture>("texture", nullptr),

	Parameter<Real>("numSegments", nullptr),

	Parameter<Real>("radius", nullptr),

	Parameter<Real>("alpha", nullptr),

	Parameter<Real>("beta", nullptr),

	Parameter<Vec2>("uv0", nullptr),

	Parameter<Vec2>("uv1", nullptr),

	Parameter<Vec2>("uv2", nullptr),

	};

	/*
	 *
	 */
	struct Factory: public Executable {
		Parameters parameters;

		Factory() :
				parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);

			auto texture = std::static_pointer_cast<Texture>(args["texture"]);

			auto numSegments = std::static_pointer_cast<Real>(
					args["numSegments"])->getInt32();

			auto radius =
					std::static_pointer_cast<Real>(args["radius"])->getFloat();

			auto alpha =
					std::static_pointer_cast<Real>(args["alpha"])->getFloat();

			auto beta =
					std::static_pointer_cast<Real>(args["beta"])->getFloat();

			const auto & uv0 = *std::static_pointer_cast<Vec2>(args["uv0"]);

			const auto & uv1 = *std::static_pointer_cast<Vec2>(args["uv1"]);

			const auto & uv2 = *std::static_pointer_cast<Vec2>(args["uv2"]);

			stack.push(
					std::make_shared<SgProjectedKaleidoscope>(
							AbstractProjectedKaleidoscope(texture, numSegments,
									radius, alpha, beta, uv0, uv1, uv2)));
		}
	};
}

/**
 * constructor
 *
 * @param abstractProjectedKaleidoscope
 */
SgProjectedKaleidoscope::SgProjectedKaleidoscope(
		const render::AbstractProjectedKaleidoscope & abstractProjectedKaleidoscope) :
		abstractProjectedKaleidoscope(abstractProjectedKaleidoscope) {
}

/**
 *
 * @param builder
 */
OVERRIDE void SgProjectedKaleidoscope::taskInit(Builder & builder) {
	builder.getLights().addProjectedKaleidoscope(abstractProjectedKaleidoscope,
			builder.getTransform());
}

/**
 * get script object factory for SgProjectedKaleidoscope
 *
 * @return  SgProjectedKaleidoscope factory
 */
STATIC const ScriptObjectPtr & SgProjectedKaleidoscope::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
