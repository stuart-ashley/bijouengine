#include "config.h"
#include "loadingCallback.h"
#include "loadManager.h"

#include "../scripting/bool.h"
#include "../scripting/parser.h"
#include "../scripting/real.h"
#include "../scripting/string.h"

#include <cassert>
#include <iostream>
#include <mutex>
#include <unordered_map>

struct Config::impl {
	struct Loader: public LoadingCallback {
		Config::impl & m_impl;

		Loader(Config::impl & impl, const std::string & currentDir,
				const std::string & filename) :
				LoadingCallback(currentDir, filename), m_impl(impl) {
			LoadManager::getInstance()->load(*this);
		}

		void load(std::istream & stream) override {
			std::string str;
			stream.seekg(0, std::ios::end);
			str.reserve(stream.tellg());
			stream.seekg(0, std::ios::beg);

			str.assign((std::istreambuf_iterator<char>(stream)),
					std::istreambuf_iterator<char>());

			Tokens tokens(str);
			Tokens::Iterator titr(tokens);

			while (titr.hasNext()) {
				// variable name
				auto name = titr.get().getValue();

				// expect identifier
				if (titr.accept(Token::Type::IDENT) == false) {
					titr.error("Require: identifier");
				}

				// expect assignment
				if (titr.accept(Token::Type::ASSIGN) == false) {
					titr.error("Require: '=='");
				}

				// parse term
				auto value = constTerm(titr);

				if (value == nullptr) {
					titr.error("Expect constant term for '" + name + "'");
				}

				// expect semicolon
				if (titr.accept(Token::Type::SEMI) == false) {
					titr.error("Require: ';'");
				}

				m_impl.put(name, value);
			}
		}
	};

	std::mutex m_lock;
	std::unordered_map<std::string, ScriptObjectPtr> m_members;

	const ScriptObjectPtr & get(const std::string & name) {
		static ScriptObjectPtr n = nullptr;

		std::lock_guard<std::mutex> locker(m_lock);

		auto it = m_members.find(name);
		if (it != m_members.end()) {
			return it->second;
		}
		return n;
	}

	void put(const std::string & key, const ScriptObjectPtr & value) {
		std::lock_guard<std::mutex> locker(m_lock);
		auto result = m_members.emplace(key, value);
		if (result.second == false) {
			result.first->second = value;
		}
	}
};

/*
 *
 */
Config::Config() :
		pimpl(new impl()) {
}

/*
 *
 */
Config::~Config() {
}

/*
 *
 */
bool Config::getBoolean(const std::string & name) const {
	const auto & value = pimpl->get(name);
	if (value != nullptr) {
		if (typeid(*value) == typeid(Bool)) {
			return value == Bool::True();
		}
	}
	return false;
}

/*
 *
 */
double Config::getDouble(const std::string & name) const {
	const auto & value = pimpl->get(name);
	if (value != nullptr) {
		if (typeid(*value) == typeid(Real)) {
			return (std::static_pointer_cast<Real>(value))->getValue();
		}
	}
	assert(false && "");
	return 0;
}

/*
 *
 */
float Config::getFloat(const std::string & name) const {
	const auto & value = pimpl->get(name);
	if (value != nullptr) {
		if (typeid(*value) == typeid(Real)) {
			return std::static_pointer_cast<Real>(value)->getFloat();
		}
	}
	assert(false && "");
	return 0;
}

/*
 *
 */
int Config::getInteger(const std::string & name) const {
	const auto & value = pimpl->get(name);
	if (value != nullptr) {
		if (typeid(*value) == typeid(Real)) {
			return std::static_pointer_cast<Real>(value)->getInt32();
		}
	}
	assert(false && "");
	return 0;
}

/*
 *
 */
const std::string & Config::getString(const std::string & name) const {
	static std::string empty = "";
	const auto & value = pimpl->get(name);
	if (value != nullptr) {
		return std::static_pointer_cast<String>(value)->getValue();
	}
	return empty;
}

/*
 *
 */
void Config::init(const std::string & currentDir,
		const std::string & filename) {
	Config::impl::Loader foo(*pimpl, currentDir, filename);
}

/*
 *
 */
void Config::set(const std::string & name, const ScriptObjectPtr & value) {
	pimpl->put(name, value);
}

/*
 *
 */
STATIC Config & Config::getInstance() {
	static Config instance;
	return instance;
}
