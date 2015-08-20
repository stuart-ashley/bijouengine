#include "sgProjectedTexture.h"

#include "builder.h"
#include "lights.h"

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
			Parameter<Real>("width", nullptr),
			Parameter<Real>("height", nullptr),
			Parameter<Real>("alpha", nullptr),
			Parameter<Real>("beta", nullptr) };

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

			float width =
					std::static_pointer_cast<Real>(args["width"])->getFloat();

			float height =
					std::static_pointer_cast<Real>(args["height"])->getFloat();

			float alpha =
					std::static_pointer_cast<Real>(args["alpha"])->getFloat();

			float beta =
					std::static_pointer_cast<Real>(args["beta"])->getFloat();

			stack.push(
					std::make_shared<SgProjectedTexture>(
							AbstractProjectedTexture(texture, width, height,
									alpha, beta)));
		}
	};
}

/**
 * constructor
 *
 * @param abstractProjectedTexture
 */
SgProjectedTexture::SgProjectedTexture(
		const render::AbstractProjectedTexture & abstractProjectedTexture) :
		abstractProjectedTexture(abstractProjectedTexture) {
}

/**
 *
 * @param builder
 */
OVERRIDE void SgProjectedTexture::taskInit(Builder & builder) {
	builder.getLights().addProjectedTexture(abstractProjectedTexture,
			builder.getTransform());
}

/**
 * get script object factory for SgProjectedTexture
 *
 * @return  SgProjectedTexture factory
 */
STATIC const ScriptObjectPtr & SgProjectedTexture::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
