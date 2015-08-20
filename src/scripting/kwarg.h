#pragma once

#include "scriptObject.h"

#include <string>

/**
 * keyword argument
 */
class Kwarg final: public ScriptObject {
public:
	/**
	 * constructor
	 *
	 * @param key name of keyword argument
	 */
	inline Kwarg(const std::string & key) :
			m_key(key) {
	}

	/** no copy constructor */
	inline Kwarg(const Kwarg &) = delete;

	/** no move constructor */
	inline Kwarg(Kwarg &&) = delete;

	/** destructor */
	inline virtual ~Kwarg() = default;

	/** no copy */
	inline Kwarg &operator=(const Kwarg &) = delete;

	/**
	 * get key name
	 *
	 * @return keyword name
	 */
	inline const std::string & getKey() const {
		return m_key;
	}
private:
	/** name of keyword */
	std::string m_key;
};
