#pragma once

#include "../scripting/scriptObject.h"

#include <memory>
#include <string>

class Config {
public:

	Config();

	~Config();

	double getDouble(const std::string & name) const;

	bool getBoolean(const std::string & name) const;

	float getFloat(const std::string & name) const;

	int getInteger(const std::string & name) const;

	void init(const std::string & currentDir, const std::string & filename);

	void set(const std::string & name, const ScriptObjectPtr & value);

	const std::string & getString(const std::string & name) const;

	static Config & getInstance();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
