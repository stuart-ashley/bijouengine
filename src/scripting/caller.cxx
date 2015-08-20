#include "caller.h"

#include <unordered_map>

struct Caller::impl {
	std::string m_filename;
	int m_line;
	std::unordered_map<std::string, ScriptObjectPtr> m_locals;

	impl(const std::string & filename, int line,
			const std::unordered_map<std::string, ScriptObjectPtr> & locals) :
			m_filename(filename), m_line(line), m_locals(locals) {
	}
};

/*
 *
 */
Caller::Caller(const std::string & filename, int line,
		const std::unordered_map<std::string, ScriptObjectPtr> & locals) :
		pimpl(new impl(filename, line, locals)) {
}

/*
 *
 */
Caller::~Caller() {
}

/*
 *
 */
const std::string & Caller::getFilename() const {
	return pimpl->m_filename;
}

/*
 *
 */
int Caller::getLine() const {
	return pimpl->m_line;
}

/*
 *
 */
const std::unordered_map<std::string, ScriptObjectPtr> & Caller::getLocals() const {
	return pimpl->m_locals;
}
