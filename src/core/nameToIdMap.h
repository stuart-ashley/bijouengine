#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class NameToIdMap {
public:
	const std::string & getName(size_t id) {
		std::lock_guard<std::mutex> locker(lock);

		return names.at(id);
	}

	bool contains(const std::string & name) {
		std::lock_guard<std::mutex> locker(lock);

		auto it = nameToId.find(name);
		return it != nameToId.end();
	}

	size_t getId(const std::string & name) {
		std::lock_guard<std::mutex> locker(lock);

		auto it = nameToId.find(name);
		if (it == nameToId.end()) {
			auto id = names.size();
			names.emplace_back(name);
			nameToId.emplace(name, id);
			return id;
		}
		return it->second;
	}
private:
	std::mutex lock;
	/** map from name to id */
	std::unordered_map<std::string, unsigned> nameToId;
	/** list of names by indexed by id */
	std::vector<std::string> names;
};

