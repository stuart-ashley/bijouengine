#pragma once

#include "scriptObject.h"

#include <memory>
#include <string>

class Real: public ScriptObject {
public:

	inline Real(double value) {
		m_value = value;
	}

	/**
	 * destructor
	 */
	~Real();

	/**
	 * is this script object equal to other script object
	 *
	 * @param other  script object to compare against
	 *
	 * @return       true if equal, false otherwise
	 */
	bool equals(const ScriptObjectPtr & other) const override;

	float getFloat() const;

	short getInt16() const;

	int getInt32() const;

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

	inline double getValue() const {
		return m_value;
	}

	bool isInt16() const;

	bool isInt32() const;

	inline std::string toString() const override {
		if (isInt32()) {
			return std::to_string(static_cast<int>(m_value));
		} else {
			return std::to_string(m_value);
		}
	}

private:
	double m_value;
};
