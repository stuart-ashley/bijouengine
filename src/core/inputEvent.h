#pragma once

#include "../scripting/scriptObject.h"

class InputEvent final: public ScriptObject {
public:

	inline InputEvent(const std::string & action, const ScriptObjectPtr & data) :
			m_action(action), m_data(data) {
	}

	/**
	 * default destructor
	 */
	inline virtual ~InputEvent() = default;

	inline const std::string & getAction() const {
		return m_action;
	}

	inline const ScriptObjectPtr & getData() const {
		return m_data;
	}

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const;

private:
	std::string m_action;
	ScriptObjectPtr m_data;
};

