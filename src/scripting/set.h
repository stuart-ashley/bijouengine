#pragma once

#include "scriptObject.h"

#include <unordered_set>
#include <string>

class Set: public ScriptObject {
public:
	inline Set(const std::unordered_set<ScriptObjectPtr> & set) :
			m_set(set) {
	}
	~Set();

	inline bool contains(const ScriptObjectPtr & element) const {
		return m_set.find(element) != m_set.end();
	}

	inline void add(const ScriptObjectPtr & element) {
		m_set.insert(element);
	}

	inline void remove(const ScriptObjectPtr & element) {
		m_set.erase(element);
	}

	inline size_t size() const {
		return m_set.size();
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
			const std::string & name) const override;

	/**
	 * get script object factory for Set
	 *
	 * @return  Set factory
	 */
	static const ScriptObjectPtr & getFactory();
private:
	std::unordered_set<ScriptObjectPtr> m_set;
};

