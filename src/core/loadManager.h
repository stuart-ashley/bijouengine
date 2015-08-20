#pragma once

#include <memory>
#include <string>

class LoadingCallback;
class LoadedResource;

class LoadManager {
public:
	LoadManager();
	virtual ~LoadManager();

	/**
	 * get filename that will be used by loader
	 *
	 * @param name
	 *            name of file
	 * @return path fixed filename
	 */
	std::string getName(const std::string & currentPath,
			const std::string & name) const;

	/**
	 * initial loader, with base directory
	 *
	 * @param dir
	 *            base directory
	 */
	virtual void start(const std::string & dir) = 0;

	/** stop loader */
	virtual void stop() = 0;

	/**
	 * add loader
	 *
	 * @param cb
	 *            callback to process file input stream
	 */
	virtual void load(LoadingCallback & cb) = 0;

	/** flush any pending files */
	virtual void flush() = 0;

	/**
	 * instance
	 *
	 * @return unique instance
	 */
	static const std::shared_ptr<LoadManager> & getInstance();

	static void setInstance(const std::shared_ptr<LoadManager> & loadManager);

	void addLoaded(const LoadedResource & resource);

	bool isLoaded(const std::string & canonicalFilename);

protected:
	void setRootPath(const std::string & rootPath);

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

