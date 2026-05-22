#include "stdafx.h"
#include "../inc/LittleEndianProcessor.h"

EightBit::LittleEndianProcessor::LittleEndianProcessor(Bus& memory) noexcept
: Processor(memory) {}

EightBit::LittleEndianProcessor::LittleEndianProcessor(const LittleEndianProcessor& rhs) noexcept
: Processor(rhs) {}

EightBit::register16_t EightBit::LittleEndianProcessor::peekShort(uint16_t address) noexcept {
	const auto low = BUS().peek(address);
	const auto high = BUS().peek(++address);
	return { low, high };
}

void EightBit::LittleEndianProcessor::pokeShort(uint16_t address, const register16_t value) noexcept {
	BUS().poke(address, value.low);
	BUS().poke(++address, value.high);
}

void EightBit::LittleEndianProcessor::fetchInto(register16_t& into) noexcept {
	fetchByte();
	into.low = BUS().DATA();
	fetchByte();
	into.high = BUS().DATA();
}

void EightBit::LittleEndianProcessor::getInto(register16_t& into) noexcept {
	memoryRead();
	into.low = BUS().DATA();
	++BUS().ADDRESS();
	memoryRead();
	into.high = BUS().DATA();
}

void EightBit::LittleEndianProcessor::getPagedInto(register16_t& into) noexcept {
	memoryRead();
	into.low = BUS().DATA();
	++BUS().ADDRESS().low;
	memoryRead();
	into.high = BUS().DATA();
}

void EightBit::LittleEndianProcessor::popInto(register16_t& into) noexcept {
	pop();
	into.low = BUS().DATA();
	pop();
	into.high = BUS().DATA();
}

void EightBit::LittleEndianProcessor::pushShort(register16_t value) noexcept {
	push(value.high);
	push(value.low);
}

void EightBit::LittleEndianProcessor::setShort(register16_t value) noexcept {
	memoryWrite(value.low);
	++BUS().ADDRESS();
	memoryWrite(value.high);
}

void EightBit::LittleEndianProcessor::setPaged(register16_t value) noexcept {
	memoryWrite(value.low);
	++BUS().ADDRESS().low;
	memoryWrite(value.high);
}
