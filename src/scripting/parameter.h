#pragma once

#include <memory>
#include <string>

#include "scriptObject.h"

class BaseParameter {
public:
	inline BaseParameter(const std::string & name, const std::type_info * type,
			ScriptObjectPtr value) :
			m_name(name), m_type(type), m_value(value) {
	}
	BaseParameter(const BaseParameter &) = default;
	~BaseParameter() = default;

	inline const std::string & getName() const {
		return m_name;
	}

	inline bool hasType() const {
		return m_type != nullptr;
	}

	inline const std::type_info & getType() const {
		return *m_type;
	}

	inline ScriptObjectPtr getValue() const {
		return m_value;
	}
private:
	std::string m_name;
	const std::type_info * m_type;
	ScriptObjectPtr m_value;
};

template<typename TYPE>
class Parameter: public BaseParameter {
public:
	inline Parameter(const std::string & name, ScriptObjectPtr value) :
			BaseParameter(name, &typeid(TYPE), value) {
	}
};
