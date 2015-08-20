#include "ray.h"

#include "transform.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/scriptExecutionException.h"

namespace {

	/*
	 *
	 */
	struct Factory: public Executable {
		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 2);

			auto start = getArg<Vec3>("Vec3", stack, 1);
			auto end = getArg<Vec3>("Vec3", stack, 2);

			stack.emplace(std::make_shared<Ray>(start, end));
		}
	};
}

/*
 *
 */
void Ray::transform(const Transform & rotTrans) {
	rotTrans.transformPoint(m_start);
	rotTrans.rotate(m_direction);
	m_end.scaleAdd(m_length, m_direction, m_start);
}

/**
 * get script object factory for Ray
 *
 * @return  Ray factory
 */
STATIC ScriptObjectPtr Ray::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
