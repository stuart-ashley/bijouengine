#pragma once

#include "scriptObject.h"

#include <memory>

class String final: public ScriptObject {
public:

	inline String(const std::string & str) :
			m_string(str) {
	}

	/**
	 * destructor
	 */
	~String();

	/**
	 * is this script object equal to other script object
	 *
	 * @param other  script object to compare against
	 *
	 * @return       true if equal, false otherwise
	 */
	bool equals(const ScriptObjectPtr & other) const override;

	/**
	 * get hash for script object
	 *
	 * @return  hash for script object
	 */
	size_t getHash() const override;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const override;

	inline const std::string & getValue() const {
		return m_string;
	}

	inline std::string toString() const override {
		return m_string;
	}

private:
	std::string m_string;
};
