#include "stdafx.h"
#include "Rom.h"

#include <iostream>

int EightBit::Rom::load(std::ifstream& file, std::vector<uint8_t>& output, const int writeOffset, const int readOffset, int limit, const int maximumSize) {

	file.seekg(0, std::ios::end);

	const auto size = (int)file.tellg();
	if ((maximumSize > 0) && ((size - readOffset) > maximumSize))
		throw std::runtime_error("Binary cannot fit");

	file.seekg(readOffset, std::ios::beg);

	if ((limit < 0) || (limit > size))
		limit = size;

	const size_t extent = limit + writeOffset;
	if (output.size() < extent)
		output.resize(extent);

	file.read((char*)&output[writeOffset], limit);

	return size;
}

int EightBit::Rom::load(const std::string& path, std::vector<uint8_t>& output, const int writeOffset, const int readOffset, const int limit, const int maximumSize) {

	std::ifstream file;
	file.exceptions(std::ios::failbit | std::ios::badbit);

	file.open(path, std::ios::binary | std::ios::ate);
	const auto size = load(file, output, writeOffset, readOffset, limit, maximumSize);
	file.close();

	return size;
}

void EightBit::Rom::poke(const uint16_t address, const uint8_t value) {
	BYTES()[address] = value;
}

EightBit::Rom::Rom(const size_t size)
: m_bytes(size) {}

size_t EightBit::Rom::size() const {
	return m_bytes.size();
}

int EightBit::Rom::load(std::ifstream& file, const int writeOffset, const int readOffset, const int limit) {
	const auto maximumSize = size() - writeOffset;
	return load(file, m_bytes, writeOffset, readOffset, limit, maximumSize);
}

int EightBit::Rom::load(const std::string& path, const int writeOffset, const int readOffset, const int limit) {
	const auto maximumSize = size() - writeOffset;
	return load(path, m_bytes, writeOffset, readOffset, limit, maximumSize);
}

int EightBit::Rom::load(const std::vector<uint8_t>& bytes, const int writeOffset, const int readOffset, int limit) {
	if (limit < 0)
		limit = bytes.size() - readOffset;
	std::copy(bytes.cbegin() + readOffset, bytes.cbegin() + limit, m_bytes.begin() + writeOffset);
	return limit;
}

uint8_t EightBit::Rom::peek(const uint16_t address) const {
	return BYTES()[address];
}
