#pragma once

#include "scriptObject.h"

#include <memory>
#include <vector>

class List: public ScriptObject {
public:

	inline List() {
	}

	inline List(const std::vector<ScriptObjectPtr> & list) :
			m_list(list) {
	}

	/**
	 * destructor
	 */
	~List();

	/**
	 * add item to end of list
	 *
	 * @param item  item to add
	 */
	void add(const ScriptObjectPtr & item);

	/**
	 * add items in list to end of list
	 *
	 * @param list  list of items to add
	 */
	void addAll(const List & list);

	inline std::vector<ScriptObjectPtr>::const_iterator begin() const {
		return m_list.cbegin();
	}

	/**
	 * does list contains item
	 *
	 * @param item  item to test for
	 *
	 * @return      true if item in list, false otherwise
	 */
	bool contains(const ScriptObjectPtr & item) const;

	inline std::vector<ScriptObjectPtr>::const_iterator end() const {
		return m_list.cend();
	}

	inline ScriptObjectPtr get(size_t idx) const {
		return m_list.at(idx);
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

	/**
	 * find index of item in list
	 *
	 * @param item  item to find
	 *
	 * @return      index of item in list, -1 if not in list
	 */
	int indexOf(const ScriptObjectPtr & item);

	/**
	 * insert item at index, shifting other items to make room
	 *
	 * @param index  index to insert
	 * @param item   item to insert
	 */
	void insert(size_t index, const ScriptObjectPtr & item);

	/**
	 * remove specified item from list, if it exists
	 *
	 * @param item  item to remove
	 */
	void remove(const ScriptObjectPtr & item);

	/**
	 * remove item at specified index from list
	 *
	 * @param index  index of item to be remove
	 */
	void remove(size_t index);

	/**
	 * set item at specified index
	 *
	 * @param index  index to set item for
	 * @param item   item to replace current one at index
	 */
	void set(size_t index, const ScriptObjectPtr & item);

	inline int size() const {
		return static_cast<int>(m_list.size());
	}

	/**
	 * get script object factory for List
	 *
	 * @return  List factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	std::vector<ScriptObjectPtr> m_list;
};
