#include "pair.h"

#include "map.h"
#include "executable.h"
#include "none.h"
#include "list.h"
#include "parameters.h"
#include "scriptExecutionException.h"

namespace {
	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto map = std::make_shared<Map>();

			// loop over arguments
			for (unsigned i = 0; i < nArgs; ++i) {
				// get key : value pair
				auto pair = getArg<Pair>("key : value pair", stack, i);

				// add to map
				map->put(pair.getKey(), pair.getValue());
			}

			// push map on stack
			stack.emplace(map);
		}
	};

	/*
	 *
	 */
	class Get: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto map = std::static_pointer_cast<Map>(self);

			auto key = stack.top();
			stack.pop();

			const auto & val = map->get(key);
			if (val == nullptr) {
				stack.emplace(None::none());
			} else {
				stack.emplace(val);
			}
		}
	};

	/*
	 *
	 */
	class GetKeys: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto map = std::static_pointer_cast<Map>(self);

			stack.emplace(std::make_shared<List>(map->getKeys()));
		}
	};

	/*
	 *
	 */
	class GetValues: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto map = std::static_pointer_cast<Map>(self);

			stack.emplace(std::make_shared<List>(map->getValues()));
		}
	};

	/*
	 *
	 */
	class Put: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto map = std::static_pointer_cast<Map>(self);

			auto key = stack.top();
			stack.pop();

			auto val = stack.top();
			stack.pop();

			map->put(key, val);
		}
	};

	/*
	 *
	 */
	class Remove: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto map = std::static_pointer_cast<Map>(self);

			auto key = stack.top();
			stack.pop();

			map->remove(key);
		}
	};

	struct key_equals: std::binary_function<std::shared_ptr<int>,
			std::shared_ptr<int>, bool> {
		bool operator()(const ScriptObjectPtr & objPtr1,
				const ScriptObjectPtr & objPtr2) const {
			return objPtr1->equals(objPtr2);
		}
	};

	struct hash_func: std::unary_function<std::shared_ptr<int>, std::size_t> {
		std::size_t operator()(const ScriptObjectPtr & objPtr) const {
			return objPtr->getHash();
		}
	};
}

struct Map::impl {
	std::unordered_map<ScriptObjectPtr, ScriptObjectPtr, hash_func, key_equals> m_map;
};

/*
 *
 */
Map::Map() :
		pimpl(new impl()) {
}

/*
 *
 */
Map::~Map() {
}

/*
 *
 */
ScriptObjectPtr Map::get(const ScriptObjectPtr & key) const {
	auto entry = pimpl->m_map.find(key);
	if (entry != pimpl->m_map.end()) {
		return entry->second;
	}
	return nullptr;
}

/*
 *
 */
std::vector<ScriptObjectPtr> Map::getKeys() const {
	std::vector<ScriptObjectPtr> keys;
	keys.reserve(pimpl->m_map.size());

	for (const auto & entry : pimpl->m_map) {
		keys.emplace_back(entry.first);
	}
	return keys;
}

/*
 *
 */
std::vector<ScriptObjectPtr> Map::getValues() const {
	std::vector<ScriptObjectPtr> values;
	values.reserve(pimpl->m_map.size());

	for (const auto & entry : pimpl->m_map) {
		values.emplace_back(entry.second);
	}
	return values;
}

/*
 *
 */
void Map::put(const ScriptObjectPtr & key, const ScriptObjectPtr & value) {
	auto result = pimpl->m_map.emplace(key, value);
	if (result.second == false) {
		result.first->second = value;
	}
}

/*
 *
 */
void Map::remove(const ScriptObjectPtr & key) {
	pimpl->m_map.erase(key);
}

/*
 *
 */
ScriptObjectPtr Map::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "get", std::make_shared<Get>() },
			{ "getKeys", std::make_shared<GetKeys>() },
			{ "getValues", std::make_shared<GetValues>() },
			{ "put", std::make_shared<Put>() },
			{ "remove", std::make_shared<Remove>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}

	return ScriptObject::getMember(execState, name);
}

/**
 * get script object factory for Map
 *
 * @return  Map factory
 */
const ScriptObjectPtr & Map::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
