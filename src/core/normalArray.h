#pragma once

#include "../scripting/scriptObject.h"

#include <memory>
#include <vector>

class BinaryFile;
class Mat3;
class Normal;

class NormalArray: public ScriptObject {
public:

	/**
	 * construct from initializer list
	 */
	NormalArray(const std::initializer_list<Normal> & list);

	/**
	 * Construct from Normal list
	 */
	NormalArray(const std::vector<Normal> & normals);

	/**
	 * Construct from BinaryFile
	 */
	NormalArray(const std::shared_ptr<BinaryFile> & binaryFile, int offset,
			int count);

	/**
	 * destructor
	 */
	~NormalArray();

	/**
	 * add normal to array
	 *
	 * @param n  normal to add
	 */
	void add(const Normal & n);

	std::vector<Normal>::const_iterator begin() const;

	std::vector<Normal>::const_iterator end() const;

	/**
	 * Get normal at specified index
	 *
	 * @param index  index to fetch
	 *
	 * @return       normal at index
	 */
	const Normal & get(size_t index) const;

	/**
	 * get number of normals in array
	 *
	 * @return  size of array
	 */
	size_t size() const;

	/**
	 * Produce a rotated version of this array
	 *
	 * @param r  rotation matrix
	 *
	 * @return   rotated normal array
	 */
	NormalArray transformed(const Mat3 & r) const;

	/**
	 * If the array is from a file make sure it's loaded
	 *
	 * @return  true if loaded, false if not
	 */
	bool validate() const;

	/**
	 * get script object factory for NormalArray
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            NormalArray factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

