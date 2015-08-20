#include "loadManagerUtils.h"

#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace {
	uint32_t s[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
			5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 4, 11, 16,
			23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 6, 10, 15, 21, 6,
			10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };

	uint32_t K[] = { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf,
			0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af,
			0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e,
			0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
			0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6,
			0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
			0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
			0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
			0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039,
			0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97,
			0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d,
			0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
			0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

	void initMd5(uint32_t * hash) {
		hash[0] = 0x67452301;
		hash[1] = 0xefcdab89;
		hash[2] = 0x98badcfe;
		hash[3] = 0x10325476;
	}

	uint32_t rol(uint32_t x, uint32_t c) {
		return (x << c) | (x >> (32 - c));
	}

	void updateMd5(uint32_t * hash, const char * buf) {
		uint32_t M[16];
		for (unsigned i = 0; i < 64; i += 4) {
			M[i / 4] = (buf[i] & 0xff) + ((buf[i + 1] & 0xff) << 8)
					+ ((buf[i + 2] & 0xff) << 16) + (buf[i + 3] << 24);
		}

		uint32_t A = hash[0];
		uint32_t B = hash[1];
		uint32_t C = hash[2];
		uint32_t D = hash[3];
		uint32_t F;

		for (unsigned i = 0; i < 16; ++i) {
			F = (B & C) | ((~ B) & D);
			uint32_t tmp = D;
			D = C;
			C = B;
			B = B + rol(A + F + K[i] + M[i], s[i]);
			A = tmp;
		}

		for (unsigned i = 16; i < 32; ++i) {
			F = (D & B) | ((~ D) & C);
			uint32_t tmp = D;
			D = C;
			C = B;
			B += rol(A + F + K[i] + M[(5 * i + 1) % 16], s[i]);
			A = tmp;
		}

		for (unsigned i = 32; i < 48; ++i) {
			F = B ^ C ^ D;
			uint32_t tmp = D;
			D = C;
			C = B;
			B += rol(A + F + K[i] + M[(3 * i + 5) % 16], s[i]);
			A = tmp;
		}

		for (unsigned i = 48; i < 64; ++i) {
			F = C ^ (B | (~ D));
			uint32_t tmp = D;
			D = C;
			C = B;
			B += rol(A + F + K[i] + M[(7 * i) % 16], s[i]);
			A = tmp;
		}

		hash[0] += A;
		hash[1] += B;
		hash[2] += C;
		hash[3] += D;
	}

	void writeLength(char * buf, std::streampos bitLength) {
		buf[56] = static_cast<char>(bitLength & 0xff);
		buf[57] = static_cast<char>((bitLength >> 8) & 0xff);
		buf[58] = static_cast<char>((bitLength >> 16) & 0xff);
		buf[59] = static_cast<char>((bitLength >> 24) & 0xff);
		buf[60] = static_cast<char>((bitLength >> 32) & 0xff);
		buf[61] = static_cast<char>((bitLength >> 40) & 0xff);
		buf[62] = static_cast<char>((bitLength >> 48) & 0xff);
		buf[63] = static_cast<char>(bitLength >> 56);
	}

	bool fileGood(const std::string & filename) {
#ifdef WIN32
		return GetFileAttributes(filename.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
		struct stat attribs;
		if (stat(filename.c_str(), &attribs) == 0) {
			return S_ISDIR(attribs.st_mode) || S_ISREG(attribs.st_mode);
		}
		return false;
#endif
	}


	std::string getFilename(const std::string & name) {
#ifdef WIN32
		char buffer[_MAX_PATH];
		GetFullPathName(name.c_str(), _MAX_PATH, buffer, NULL);
#else
		char buffer[PATH_MAX];
		realpath(name.c_str(), buffer);
#endif
		return buffer;
	}
}

std::string LoadManagerUtils::getCanonicalName(const std::string & currentDir,
		const std::string & filename, const std::string & rootDir) {
	// filename
	auto canonicalFilename = getFilename(filename);
	if (fileGood(canonicalFilename)) {
		return canonicalFilename;
	}

	// currentDir + filename
	canonicalFilename = getFilename(currentDir + filename);
	if (fileGood(canonicalFilename)) {
		return canonicalFilename;
	}

	// rootDir + filename
	canonicalFilename = getFilename(rootDir + filename);
	if (fileGood(canonicalFilename)) {
		return canonicalFilename;
	}

	// rootDir + currentDir + filename
	canonicalFilename = getFilename(rootDir + currentDir + filename);
	if (fileGood(canonicalFilename)) {
		return canonicalFilename;
	}

	std::cerr << "ERROR: Can't find file '" + filename + "'" << std::endl;
	assert(false);
	return "";
}

/*
 *
 */
std::string LoadManagerUtils::getDirectory(const std::string & filename) {
#ifdef WIN32
	char path[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath_s(filename.c_str(), drive, dir, fname, ext);
	_makepath_s(path, drive, dir, NULL, NULL);

	return path;
#else
	auto idx = filename.find_last_of('/');
	if (idx == std::string::npos) {
		return "";
	}
	return filename.substr(0, idx + 1);
#endif
}

/**
 * calculate md5 hash of file
 *
 * @param canonicalFilename
 *            name of file to calculate md5 hash
 * @return string version of md5 hash
 */
std::string LoadManagerUtils::md5sum(const std::string & canonicalFilename) {
	// calculate md5sum
	uint32_t hash[4];
	initMd5(hash);

	std::ifstream in(canonicalFilename, std::ios::binary | std::ios::ate);
	auto length = in.tellg();
	in.seekg(std::ios::beg);

	char buf[64];

	while (in.good()) {
		in.read(buf, 64);
		auto n = in.gcount();
		if (n < 64) {
			if (n < 56) {
				buf[n] = -128;
				for (auto i = n + 1; i < 56; ++i) {
					buf[i] = 0;
				}
				writeLength(buf, length * 8);
			} else {
				buf[n] = -128;
				for (auto i = n + 1; i < 64; ++i) {
					buf[i] = 0;
				}
				updateMd5(hash, buf);
				for (auto i = 0; i < 56; ++i) {
					buf[i] = 0;
				}
				writeLength(buf, length * 8);
			}
		}
		updateMd5(hash, buf);
	}

	// convert to string
	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (unsigned i = 0; i < 4; ++i) {
		ss << std::setw(2) << (hash[i] & 0xff);
		ss << std::setw(2) << ((hash[i] >> 8) & 0xff);
		ss << std::setw(2) << ((hash[i] >> 16) & 0xff);
		ss << std::setw(2) << (hash[i] >> 24);
	}

	return ss.str();
}
