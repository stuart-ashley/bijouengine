#pragma once

#include "scriptObject.h"

#include <memory>

class BreakpointMarker: public ScriptObject {
public:
	BreakpointMarker(int startLine, int endLine);
	~BreakpointMarker();

	void toggle();
	void setActive(bool active);
	void setEnabled(bool enabled);
	bool isActive() const;
	bool isEnabled() const;
	int getStartLine() const;
	int getEndLine() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
