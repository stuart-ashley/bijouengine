#include "list.h"

#include "executable.h"
#include "parameters.h"

#include <algorithm>
#include <cassert>

namespace {
	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			std::vector<ScriptObjectPtr> list;
			list.reserve(nArgs);

			for (unsigned i = 0; i < nArgs; ++i) {
				list.emplace_back(stack.top());
				stack.pop();
			}

			stack.emplace(std::make_shared<List>(list));
		}
	};

	/*
	 *
	 */
	class Add: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto list = std::static_pointer_cast<List>(self);

			auto item = stack.top();
			stack.pop();

			list->add(item);
		}
	};

	/*
	 *
	 */
	class AddAll: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto list = std::static_pointer_cast<List>(self);

			auto item = stack.top();
			stack.pop();

			scriptExecutionAssertType<List>(item,
					"Require single List argument");

			list->addAll(*std::static_pointer_cast<List>(item));
		}
	};

	/*
	 *
	 */
	class Contains: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto list = std::static_pointer_cast<List>(self);

			auto item = stack.top();
			stack.pop();

			if (list->contains(item)) {
				stack.push(Bool::True());
			} else {
				stack.push(Bool::False());
			}
		}
	};

	/*
	 *
	 */
	class Get: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto list = std::static_pointer_cast<List>(self);

			auto idx = getInt32Arg(stack, 1);

			scriptExecutionAssert(idx >= 0 && idx < list->size(),
					"Index out of bounds");

			stack.push(list->get(idx));
		}
	};

	/*
	 * index of item in list or -1 if not in list
	 */
	class IndexOf: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto list = std::static_pointer_cast<List>(self);

			auto item = stack.top();
			stack.pop();

			stack.push(std::make_shared<Real>(list->indexOf(item)));
		}
	};

	/*
	 * move items idx and above to the right, making room for new item
	 */
	class Insert: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto list = std::static_pointer_cast<List>(self);

			auto idx = getInt32Arg(stack, 1);

			scriptExecutionAssert(idx >= 0 && idx <= list->size(),
					"Index out of bounds");

			auto item = stack.top();
			stack.pop();

			list->insert(idx, item);
		}
	};

	/*
	 *
	 */
	class Remove: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto list = std::static_pointer_cast<List>(self);

			auto item = stack.top();
			stack.pop();

			list->remove(item);
		}
	};

	/*
	 *
	 */
	class RemoveIndex: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto list = std::static_pointer_cast<List>(self);

			auto idx = getInt32Arg(stack, 1);

			scriptExecutionAssert(idx >= 0 && idx < list->size(),
					"Index out of bounds");

			list->remove(idx);
		}
	};

	/*
	 *
	 */
	class Set: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto list = std::static_pointer_cast<List>(self);

			auto idx = getInt32Arg(stack, 1);

			scriptExecutionAssert(idx >= 0 && idx < list->size(),
					"Index out of bounds");

			auto item = stack.top();
			stack.pop();

			list->set(idx, item);
		}
	};

	/*
	 *
	 */
	class Size: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto list = std::static_pointer_cast<List>(self);

			stack.push(std::make_shared<Real>(list->size()));
		}
	};
}

/**
 * destructor
 */
List::~List() {
}

/**
 * add item to end of list
 *
 * @param item  item to add
 */
void List::add(const ScriptObjectPtr & item) {
	m_list.emplace_back(item);
}

/**
 * add items in list to end of list
 *
 * @param list  list of items to add
 */
void List::addAll(const List & list) {
	m_list.insert(m_list.end(), list.begin(), list.end());
}

/**
 * does list contains item
 *
 * @param item  item to test for
 *
 * @return      true if item in list, false otherwise
 */
bool List::contains(const ScriptObjectPtr & item) const {
	return std::find(m_list.begin(), m_list.end(), item) != m_list.end();
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr List::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "add", std::make_shared<Add>() },
			{ "addAll", std::make_shared<AddAll>() },
			{ "contains", std::make_shared<Contains>() },
			{ "get", std::make_shared<Get>() },
			{ "indexOf", std::make_shared<IndexOf>() },
			{ "insert", std::make_shared<Insert>() },
			{ "remove", std::make_shared<Remove>() },
			{ "removeIndex", std::make_shared<RemoveIndex>() },
			{ "set", std::make_shared<Set>() },
			{ "size", std::make_shared<Size>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * find index of item in list
 *
 * @param item  item to find
 *
 * @return      index of item in list, -1 if not in list
 */
int List::indexOf(const ScriptObjectPtr & item) {
	auto it = std::find(m_list.begin(), m_list.end(), item);
	if (it == m_list.end()) {
		return -1;
	}
	return static_cast<int>(std::distance(m_list.begin(), it));
}

/**
 * insert item at index, shifting other items to make room
 *
 * @param index  index to insert
 * @param item   item to insert
 */
void List::insert(size_t index, const ScriptObjectPtr & item) {
	assert(index <= m_list.size());
	m_list.insert(m_list.begin() + index, item);
}

/**
 * remove specified item from list, if it exists
 *
 * @param item  item to remove
 */
void List::remove(const ScriptObjectPtr & item) {
	m_list.erase(std::remove(m_list.begin(), m_list.end(), item), m_list.end());
}

/**
 * remove item at specified index from list
 *
 * @param index  index of item to be remove
 */
void List::remove(size_t index) {
	assert(index <= m_list.size());
	m_list.erase(m_list.begin() + index);
}

/**
 * set item at specified index
 *
 * @param index  index to set item for
 * @param item   item to replace current one at index
 */
void List::set(size_t index, const ScriptObjectPtr & item) {
	assert(index < m_list.size());
	m_list[index] = item;
}

/**
 * get script object factory for List
 *
 * @return  List factory
 */
STATIC const ScriptObjectPtr & List::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
