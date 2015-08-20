#pragma once

#include "scriptObject.h"

class Pair final: public ScriptObject {
public:
	inline Pair(const ScriptObjectPtr & key, const ScriptObjectPtr & value) {
		m_key = key;
		m_value = value;
	}

	inline virtual ~Pair() = default;

	inline const ScriptObjectPtr & getKey() const {
		return m_key;
	}

	inline const ScriptObjectPtr & getValue() const {
		return m_value;
	}

	/**
	 * get script object factory for Pair
	 *
	 * @return  Pair factory
	 */
	static const ScriptObjectPtr & getFactory();
private:
	ScriptObjectPtr m_key;
	ScriptObjectPtr m_value;
};

