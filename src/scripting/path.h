#pragma once

#include "scriptObject.h"

#include <memory>
#include <vector>

class Path final: public ScriptObject {
public:

	/**
	 *
	 */
	Path(const std::string & canonicalPath);

	/**
	 *
	 */
	Path(const Path & parent, const std::string & name);

	/**
	 *
	 */
	~Path();

	/**
	 *
	 */
	const std::string & getCanonicalName() const;

	/**
	 *
	 */
	std::vector<std::shared_ptr<Path>> getDirectoryContents() const;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const override;

	/**
	 *
	 */
	const std::string & getName() const;

	/**
	 *
	 */
	Path getParent() const;

	/**
	 *
	 */
	bool isDir() const;

	/**
	 *
	 */
	bool isFile() const;

	/**
	 *
	 */
	bool isHidden() const;

	/**
	 *
	 */
	std::string toString() const override;

private:
	std::string m_canonicalName;
	std::string m_name;
};
