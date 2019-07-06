#include "stdafx.h"
#include "Bus.h"
#include "Ram.h"
#include "IntelHexFile.h"
#include "EightBitCompilerDefinitions.h"

void EightBit::Bus::raisePOWER() {}

void EightBit::Bus::lowerPOWER() {}

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
	if (mapped.access == MemoryMapping::AccessLevel::ReadOnly)
		return DATA() = mapped.memory.peek(offset);
	return mapped.memory.reference(offset);
}
