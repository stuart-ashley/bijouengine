#pragma once

#include <memory>
#include <string>

typedef std::shared_ptr<class ScriptObject> ScriptObjectPtr;

class ScriptExecutionState;

class ScriptObject {
public:

	/**
	 * destructor
	 */
	virtual ~ScriptObject();

	/**
	 * is this script object equal to other script object
	 *
	 * @param other  script object to compare against
	 *
	 * @return       true if equal, false otherwise
	 */
	virtual bool equals(const ScriptObjectPtr & other) const;

	/**
	 * get hash for script object
	 *
	 * @return  hash for script object
	 */
	virtual size_t getHash() const;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	virtual ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const;

	/**
	 * set named script object member
	 *
	 * @param name   name of member
	 * @param value  desired value
	 */
	virtual void setMember(const std::string & name,
			const ScriptObjectPtr & value);

	/**
	 * calculate string representation of ScriptObject
	 *
	 * @return  ScriptObject as string
	 */
	virtual std::string toString() const;

protected:
	ScriptObject() = default;
};

