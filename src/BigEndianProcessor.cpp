#include "stdafx.h"
#include "../inc/BigEndianProcessor.h"

EightBit::BigEndianProcessor::BigEndianProcessor(Bus& memory) noexcept
: Processor(memory) {}

EightBit::BigEndianProcessor::BigEndianProcessor(const BigEndianProcessor& rhs) noexcept
: Processor(rhs) {}

EightBit::register16_t EightBit::BigEndianProcessor::peekShort(uint16_t address) noexcept {
	const auto high = BUS().peek(address);
	const auto low = BUS().peek(++address);
	return { low, high };
}

void EightBit::BigEndianProcessor::pokeShort(uint16_t address, const register16_t value) noexcept {
	BUS().poke(address, value.high);
	BUS().poke(++address, value.low);
}

void EightBit::BigEndianProcessor::fetchInto(register16_t& into) noexcept {
	fetchByte();
	into.high = BUS().DATA();
	fetchByte();
	into.low = BUS().DATA();
}

void EightBit::BigEndianProcessor::getInto(register16_t& into) noexcept {
	memoryRead();
	into.high = BUS().DATA();
	++BUS().ADDRESS();
	memoryRead();
	into.low = BUS().DATA();
}

void EightBit::BigEndianProcessor::getPagedInto(register16_t& into) noexcept {
	memoryRead();
	into.high = BUS().DATA();
	++BUS().ADDRESS().low;
	memoryRead();
	into.low = BUS().DATA();
}

void EightBit::BigEndianProcessor::popInto(register16_t& into) noexcept {
	pop();
	into.high = BUS().DATA();
	pop();
	into.low = BUS().DATA();
}

void EightBit::BigEndianProcessor::pushShort(register16_t value) noexcept {
	push(value.low);
	push(value.high);
}

void EightBit::BigEndianProcessor::setShort(register16_t value) noexcept {
	memoryWrite(value.high);
	++BUS().ADDRESS();
	memoryWrite(value.low);
}

void EightBit::BigEndianProcessor::setPaged(register16_t value) noexcept {
	memoryWrite(value.high);
	++BUS().ADDRESS().low;
	memoryWrite(value.low);
}
