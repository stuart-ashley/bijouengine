#pragma once

#include <memory>
#include <string>

namespace render {
	class ImageFile {
	public:
		ImageFile(const std::string & currentDir, const std::string & filename);
		~ImageFile();

		unsigned getPixelWidth() const;

		unsigned getPixelHeight() const;

		unsigned getPixelFormat() const;

		const unsigned char * getPixelData() const;

		bool isLuminance() const;

		bool valid() const;
	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

