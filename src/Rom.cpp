#include "stdafx.h"
#include "../inc/Rom.h"

#include <iostream>

bool EightBit::Rom::operator==(const Rom& rhs) const {
	return BYTES() == rhs.BYTES();
}

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

int EightBit::Rom::load(const std::string path, std::vector<uint8_t>& output, const int writeOffset, const int readOffset, const int limit, const int maximumSize) {

	std::ifstream file;
	file.exceptions(std::ios::failbit | std::ios::badbit);

	file.open(path, std::ios::binary | std::ios::ate);
	const auto size = load(file, output, writeOffset, readOffset, limit, maximumSize);
	file.close();

	return size;
}

void EightBit::Rom::poke(const uint16_t address, const uint8_t value) noexcept {
	BYTES()[address] = value;
}

EightBit::Rom::Rom(const size_t size) noexcept
: m_bytes(size) {}

EightBit::Rom::Rom(const Rom& rhs)
: m_bytes(rhs.m_bytes) {}

EightBit::Rom& EightBit::Rom::operator=(const Rom& rhs) {
	if (this != &rhs)
		m_bytes = rhs.m_bytes;
	return *this;
}

uint16_t EightBit::Rom::size() const noexcept {
	const auto size = BYTES().size();
	assert(size <= 0x10000);
	return static_cast<uint16_t>(size);
}

int EightBit::Rom::load(std::ifstream& file, const int writeOffset, const int readOffset, const int limit) {
	const auto maximumSize = size() - writeOffset;
	return load(file, m_bytes, writeOffset, readOffset, limit, maximumSize);
}

int EightBit::Rom::load(const std::string path, const int writeOffset, const int readOffset, const int limit) {
	const auto maximumSize = size() - writeOffset;
	return load(path, m_bytes, writeOffset, readOffset, limit, maximumSize);
}

int EightBit::Rom::load(const std::vector<uint8_t>& bytes, const int writeOffset, const int readOffset, int limit) {
	return load(bytes.cbegin(), bytes.cend(), writeOffset, readOffset, limit);
}

uint8_t EightBit::Rom::peek(const uint16_t address) const noexcept {
	return BYTES()[address];
}
