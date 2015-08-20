#pragma once

#include "../scripting/scriptObject.h"

#include <string>
#include <unordered_map>

class ScriptExecutionState;

namespace render {
	class RenderModule: public ScriptObject {
	public:

		/**
		 * constructor
		 *
		 * @param currentDir
		 */
		RenderModule(const std::string & currentDir);

		/**
		 * destructor
		 */
		~RenderModule();

		/**
		 * get named script object member
		 *
		 * @param execState  current script execution state
		 * @param name       name of member
		 *
		 * @return           script object represented by name
		 */
		ScriptObjectPtr getMember(ScriptExecutionState &,
				const std::string & name) const;

	private:
		std::unordered_map<std::string, ScriptObjectPtr> members;
	};
}

