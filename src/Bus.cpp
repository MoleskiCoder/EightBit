#include "stdafx.h"
#include "../inc/Bus.h"
#include "../inc/Ram.h"
#include "../inc/IntelHexFile.h"
#include "../inc/EightBitCompilerDefinitions.h"

void EightBit::Bus::raisePOWER() {}

void EightBit::Bus::lowerPOWER() {}

uint8_t EightBit::Bus::read() {
	ReadingByte.fire();
	const auto returned = DATA() = reference();
	ReadByte.fire();
	return returned;
}

void EightBit::Bus::write() {
	WritingByte.fire();
	reference() = DATA();
	WrittenByte.fire();
}

void EightBit::Bus::write(const uint8_t value) {
	DATA() = value;
	write();
}

void EightBit::Bus::loadHexFile(const std::string path) {
	IntelHexFile file(path);
	const auto chunks = file.parse();
	for (const auto& chunk : chunks) {
		const auto address = chunk.first;
		const auto content = chunk.second;
		const auto mapped = mapping(address);
		const uint16_t offset = address - mapped.begin;
		mapped.memory.load(content, offset);
	}
}

uint8_t& EightBit::Bus::reference(const uint16_t address) {
	const auto mapped = mapping(address);
	const uint16_t offset = (address - mapped.begin) & mapped.mask;
	if (mapped.access == MemoryMapping::AccessLevel::ReadOnly) {
		DATA() = mapped.memory.peek(offset);
		return DATA();
	}
	return mapped.memory.reference(offset);
}
