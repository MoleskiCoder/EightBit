#include "stdafx.h"
#include "Memory.h"

#include <iostream>
#include <fstream>

int EightBit::Memory::loadBinary(const std::string& path, std::vector<uint8_t>& output, int offset, int maximumSize) {
	std::ifstream file;
	file.exceptions(std::ios::failbit | std::ios::badbit);

	file.open(path, std::ios::binary | std::ios::ate);
	auto size = (int)file.tellg();
	if ((maximumSize > 0) && (size > maximumSize))
		throw std::runtime_error("Binary cannot fit");

	size_t extent = size + offset;
	if (output.size() < extent)
		output.resize(extent);

	file.seekg(0, std::ios::beg);

	file.read((char*)&output[offset], size);
	file.close();

	return size;
}