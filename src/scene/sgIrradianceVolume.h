#pragma once

#include "taskInitNode.h"

#include "../core/endEffector.h"

#include "../scripting/scriptObject.h"

#include <string>

namespace render {
	class AbstractIrradianceVolume;
}

class BinaryFile;

class SgIrradianceVolume: public ScriptObject, public TaskInitNode {
public:

	/**
	 * constructor
	 *
	 * @param name
	 * @param binFile
	 * @param extents
	 * @param size
	 */
	SgIrradianceVolume(const std::string & name,
			const std::shared_ptr<BinaryFile> & binFile, const Vec3 & extents,
			int sizeX, int sizeY, int sizeZ);

	/**
	 * constructor
	 *
	 * @param name
	 * @param buffer
	 * @param extents
	 * @param size
	 */
	SgIrradianceVolume(const std::string & name, std::unique_ptr<uint8_t[]> & buffer,
			const Vec3 & extents, int sizeX, int sizeY, int sizeZ);

	/**
	 * destructor
	 */
	~SgIrradianceVolume();

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
	 * @return
	 */
	const std::unique_ptr<render::AbstractIrradianceVolume> & getVolume() const;

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 * get script object factory for SgIrradianceVolume
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            SgIrradianceVolume factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

