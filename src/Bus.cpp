#include "stdafx.h"
#include "../inc/Bus.h"
#include "../inc/Ram.h"
#include "../inc/IntelHexFile.h"
#include "../inc/EightBitCompilerDefinitions.h"

void EightBit::Bus::raisePOWER() noexcept {}

void EightBit::Bus::lowerPOWER() noexcept {}

void EightBit::Bus::read() noexcept {
	DATA() = reference();
}

void EightBit::Bus::write() noexcept {
	assert(!m_writing);
	m_writing = true;
	reference() = DATA();
	m_writing = false;
	assert(!m_writing);
}

void EightBit::Bus::loadHexFile(const std::string& path) {
	IntelHexFile file(path);
	const auto chunks = file.parse();
	for (const auto& chunk : chunks) {
		const auto& [address, content] = chunk;
		const auto mapped = mapping(address);
		const uint16_t offset = address - mapped.begin;
		mapped.memory.load(content, offset);
	}
}

uint8_t& EightBit::Bus::reference(const uint16_t address) noexcept {
	const auto& mapped = mapping(address);
	const auto offset = mapped.offset(address);
	if (mapped.access != MemoryMapping::AccessLevel::ReadOnly)
		return mapped.memory.reference(offset);
	if (!m_writing)
		DATA() = mapped.memory.peek(offset);
	return DATA();
}
