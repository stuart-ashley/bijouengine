#include "binaryFile.h"

#include "loadManager.h"

/*
 *
 */
BinaryFile::BinaryFile(const std::string & currentDir, const std::string & name) :
				LoadingCallback(currentDir, name),
				m_data(nullptr),
				m_dataSize(0),
				m_valid(false) {
	LoadManager::getInstance()->load(*this);
}

/*
 *
 */
BinaryFile::~BinaryFile() {
	delete[] m_data;
}

/*
 *
 */
const char * BinaryFile::getData() const {
	return m_data;
}

/*
 *
 */
size_t BinaryFile::getSize() const {
	return m_dataSize;
}

/*
 *
 */
PRIVATE void BinaryFile::load(std::istream & stream) {
	stream.seekg(0, std::ios::end);
	m_dataSize = stream.tellg();

	m_data = new char[m_dataSize];

	stream.seekg(0, std::ios::beg);
	stream.read(m_data, m_dataSize);

	m_valid = true;
}

/*
 *
 */
bool BinaryFile::valid() const {
	return m_valid;
}
