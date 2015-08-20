#include "imageFile.h"

#include "../core/loadingCallback.h"
#include "../core/loadManager.h"

#include <cassert>
#include <mutex>

#include <IL/il.h>
#include <IL/ilu.h>

using namespace render;

struct ImageFile::impl: public LoadingCallback {
	std::mutex m_lock;
	std::unique_ptr<char> m_data;
	size_t m_dataSize;

	ILuint m_id;
	ILint m_width;
	ILint m_height;
	ILint m_format;
	ILubyte * m_pixelData;

	/*
	 *
	 */
	impl(const std::string & currentDir, const std::string & filename) :
					LoadingCallback(currentDir, filename),
					m_data(nullptr),
					m_dataSize(0),
					m_id(0),
					m_width(0),
					m_height(0),
					m_format(0),
					m_pixelData(nullptr) {
		LoadManager::getInstance()->load(*this);
	}

	/*
	 *
	 */
	~impl() {
		ilDeleteImages(1, &m_id);
	}

	/*
	 *
	 */
	void load(std::istream & stream) {
		std::lock_guard<std::mutex> locker(m_lock);

		stream.seekg(0, std::ios::end);
		m_dataSize = stream.tellg();

		m_data = std::unique_ptr<char>(new char[m_dataSize]);

		stream.seekg(0, std::ios::beg);
		stream.read(m_data.get(), m_dataSize);
	}
};

/*
 *
 */
ImageFile::ImageFile(const std::string & currentDir,
		const std::string & filename) :
		pimpl(new impl(currentDir, filename)) {
}

/*
 *
 */
ImageFile::~ImageFile() {
}

/*
 *
 */
unsigned ImageFile::getPixelWidth() const {
	assert(valid());
	return pimpl->m_width;
}

/*
 *
 */
unsigned ImageFile::getPixelHeight() const {
	assert(valid());
	return pimpl->m_height;
}

/*
 *
 */
unsigned ImageFile::getPixelFormat() const {
	assert(valid());
	return pimpl->m_format;
}

/*
 *
 */
const unsigned char * ImageFile::getPixelData() const {
	assert(valid());
	return pimpl->m_pixelData;
}

/*
 *
 */
bool ImageFile::isLuminance() const {
	assert(valid());
	return pimpl->m_format == IL_LUMINANCE;
}

/*
 *
 */
bool ImageFile::valid() const {
	std::lock_guard<std::mutex> locker(pimpl->m_lock);

	if (pimpl->m_pixelData != nullptr) {
		return true;
	}

	if (pimpl->m_data != nullptr) {
		// create image from loaded image data
		ilGenImages(1, &pimpl->m_id);
		ilBindImage(pimpl->m_id);
		ILenum type = ilDetermineTypeL(pimpl->m_data.get(),
				static_cast<ILuint>(pimpl->m_dataSize));
		ILboolean loaded = ilLoadL(type, pimpl->m_data.get(),
				static_cast<ILuint>(pimpl->m_dataSize));
		assert(loaded == IL_TRUE);

		// fix format
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		// fix size
		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
		iluScale((ilGetInteger(IL_IMAGE_WIDTH) >> 2) << 2,
				(ilGetInteger(IL_IMAGE_HEIGHT) >> 2) << 2,
				ilGetInteger(IL_IMAGE_DEPTH));

		pimpl->m_width = ilGetInteger(IL_IMAGE_WIDTH);
		pimpl->m_height = ilGetInteger(IL_IMAGE_HEIGHT);
		pimpl->m_format = ilGetInteger(IL_IMAGE_FORMAT);
		pimpl->m_pixelData = ilGetData();

		pimpl->m_data = nullptr;

		return true;
	}

	return false;
}
