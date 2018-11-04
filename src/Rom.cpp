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
