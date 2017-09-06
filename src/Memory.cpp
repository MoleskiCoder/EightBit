#include "stdafx.h"
#include "Memory.h"

#include <iostream>
#include <fstream>

int EightBit::Memory::loadBinary(
		const std::string& path,
		std::vector<uint8_t>& output,
		int writeOffset,
		int readOffset,
		int limit,
		int maximumSize) {

	std::ifstream file;
	file.exceptions(std::ios::failbit | std::ios::badbit);

	file.open(path, std::ios::binary | std::ios::ate);
	const auto size = (int)file.tellg();
	if ((maximumSize > 0) && ((size - readOffset) > maximumSize))
		throw std::runtime_error("Binary cannot fit");

	size_t extent = size + writeOffset;
	if (output.size() < extent)
		output.resize(extent);

	file.seekg(readOffset, std::ios::beg);

	if ((limit < 0) || (limit > size))
		limit = size;

	file.read((char*)&output[writeOffset], limit);
	file.close();

	return size;
}