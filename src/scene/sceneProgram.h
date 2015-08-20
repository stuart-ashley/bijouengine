#pragma once

#include "../scripting/breakpointMarker.h"
#include "../scripting/program.h"

#include <memory>
#include <string>
#include <vector>

class UpdateState;

class SceneProgram final: public ScriptObject {
public:

	SceneProgram(const std::string & canonicalFilename);

	/**
	 * default destructor
	 */
	inline virtual ~SceneProgram() = default;

	void addLoadedCallback(const ScriptObjectPtr & callback);

	void execute(const std::shared_ptr<UpdateState> & state) const;

	void executeLoadedCallbacks();

	std::shared_ptr<BreakpointMarker> getBreakpoint(int line) const;

	std::vector<std::shared_ptr<BreakpointMarker>> getBreakpoints() const;

	const std::string & getContent() const;

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
	 * set named script object member
	 *
	 * @param name   name of member
	 * @param value  desired value
	 */
	void setMember(const std::string & name, const ScriptObjectPtr & value)
			override;

	bool valid() const;

private:
	std::shared_ptr<Program> program;
	std::vector<ScriptObjectPtr> callbacks;
};

