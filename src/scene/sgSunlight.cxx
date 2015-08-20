#include "sgSunlight.h"

#include "builder.h"
#include "lights.h"

#include "../scripting/executable.h"
#include "../scripting/none.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

namespace {
	std::vector<BaseParameter> params = { Parameter<String>("name",
			std::make_shared<String>("")), Parameter<Color>("color", nullptr),
			Parameter<Vec2>("extent", None::none()) };
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

			const auto & name =
					std::static_pointer_cast<String>(args["name"])->getValue();

			const auto & colorArg = std::static_pointer_cast<Color>(
					args["color"]);

			if (args["extent"] == None::none()) {
				stack.push(
						std::make_shared<SgSunlight>(
								AbstractSunlight(name, *colorArg)));
			} else {
				const auto & extentArg = std::static_pointer_cast<Vec2>(
						args["extent"]);
				stack.push(
						std::make_shared<SgSunlight>(
								AbstractSunlight(name, *colorArg, *extentArg)));
			}

		}
	};
}

/**
 *
 * @param sunlight
 */
SgSunlight::SgSunlight(const AbstractSunlight & sunlight) :
		light(sunlight) {
}

/**
 *
 * @param builder
 */
OVERRIDE void SgSunlight::taskInit(Builder & builder) {
	builder.getLights().addSunlight(light, builder.getTransform());
}

/**
 * get script object factory for SgSunlight
 *
 * @return  SgSunlight factory
 */
STATIC const ScriptObjectPtr & SgSunlight::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
