#include "breakpointMarker.h"

#include <cassert>

struct BreakpointMarker::impl {
	int m_startLine;
	int m_endLine;
	bool m_active;
	bool m_enabled;

	/*
	 */
	impl(int startLine, int endLine) :
					m_startLine(startLine),
					m_endLine(endLine),
					m_active(false),
					m_enabled(true) {
	}
};

/*
 */
BreakpointMarker::BreakpointMarker(int startLine, int endLine) :
		pimpl(new impl(startLine, endLine)) {
}

/*
 */
BreakpointMarker::~BreakpointMarker() {
}

/*
 *
 */
void BreakpointMarker::toggle() {
	pimpl->m_active = !pimpl->m_active;
	pimpl->m_enabled = true;
}

/*
 *
 */
void BreakpointMarker::setActive(bool active) {
	if (pimpl->m_active != active) {
		pimpl->m_active = active;
		// any change to active state resets enabled flag
		pimpl->m_enabled = true;
	}
}

/*
 *
 */
void BreakpointMarker::setEnabled(bool enabled) {
	assert(pimpl->m_active);
	pimpl->m_enabled = enabled;
}

/*
 *
 */
bool BreakpointMarker::isActive() const {
	return pimpl->m_active;
}

/*
 *
 */
bool BreakpointMarker::isEnabled() const {
	return pimpl->m_enabled;
}

/*
 *
 */
int BreakpointMarker::getStartLine() const {
	return pimpl->m_startLine;
}

/*
 *
 */
int BreakpointMarker::getEndLine() const {
	return pimpl->m_endLine;
}
