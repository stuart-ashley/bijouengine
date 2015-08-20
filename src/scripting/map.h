#pragma once

#include "scriptObject.h"

#include <memory>
#include <vector>
#include <unordered_map>

class Map: public ScriptObject {
public:
	Map();
	~Map();

	ScriptObjectPtr get(const ScriptObjectPtr & key) const;
	std::vector<ScriptObjectPtr> getKeys() const;
	std::vector<ScriptObjectPtr> getValues() const;
	void put(const ScriptObjectPtr & key, const ScriptObjectPtr & value);
	void remove(const ScriptObjectPtr & key);

	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const override;

	/**
	 * get script object factory for Map
	 *
	 * @return  Map factory
	 */
	static const ScriptObjectPtr & getFactory();
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

