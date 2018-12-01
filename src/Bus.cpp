#include "stdafx.h"
#include "Bus.h"
#include "Ram.h"
#include "EightBitCompilerDefinitions.h"

#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include <cassert>

void EightBit::Bus::powerOn() {
	initialise();
}

void EightBit::Bus::powerOff() {}

uint8_t EightBit::Bus::read() {
	ReadingByte.fire(EventArgs::empty());
	const auto returned = DATA() = reference();
	ReadByte.fire(EventArgs::empty());
	return returned;
}

void EightBit::Bus::write() {
	WritingByte.fire(EventArgs::empty());
	reference() = DATA();
	WrittenByte.fire(EventArgs::empty());
}

void EightBit::Bus::write(const uint8_t value) {
	DATA() = value;
	write();
}

void EightBit::Bus::loadHexFile(const std::string path) {
	const auto chunks = parseHexFile(path);
	for (const auto& chunk : chunks) {
		const auto address = chunk.first;
		const auto content = chunk.second;
		const auto mapped = mapping(address);
		const uint16_t offset = address - mapped.begin;
		mapped.memory.load(content, offset);
	}
}

std::map<uint16_t, std::vector<uint8_t>> EightBit::Bus::parseHexFile(const std::string path) {

	std::ifstream file;
	file.open(path);

	std::map<uint16_t, std::vector<uint8_t>> returned;

	bool eof = false;
	while (!file.eof() && !eof) {

		std::string line;
		std::getline(file, line);

		const auto colon = line.substr(0, 1);
		if (colon != ":")
			throw std::out_of_range("Invalid hex file: line does not begin with a colon");

		const auto countString = line.substr(1, 2);
		const auto count = (uint8_t)strtoul(countString.c_str(), nullptr, 16); 

		const auto addressString = line.substr(3, 4);
		const auto address = (uint16_t)strtoul(addressString.c_str(), nullptr, 16); 

		const auto recordTypeString = line.substr(7, 2);
		const auto recordType = strtoul(recordTypeString.c_str(), nullptr, 16);

		switch (recordType) {
		case 0x00: {
				std::vector<uint8_t> data(count);
				const auto requiredLength = 9 + 2 + (count * 2);
				if (line.length() != requiredLength)
					throw std::out_of_range("Invalid hex file: line is not the required length");
				for (int i = 0; i < count; ++i) {
					const auto position = 9 + i * 2;
					const auto datumString = line.substr(position, 2);
					const auto datum = (uint8_t)strtoul(datumString.c_str(), nullptr, 16);
					data[i] = datum;
				}
				returned[address] = data;
			}
			break;
		case 0x01:
			eof = true;
			break;
		default:
			throw std::out_of_range("Unhandled hex file record.");
		}
	}
	return returned;
}

uint8_t& EightBit::Bus::reference(const uint16_t address) {
	const auto mapped = mapping(address);
	const uint16_t offset = (address - mapped.begin) & mapped.mask;
	if (mapped.access == MemoryMapping::AccessLevel::ReadOnly)
		return DATA() = mapped.memory.peek(offset);
	return mapped.memory.reference(offset);
}
