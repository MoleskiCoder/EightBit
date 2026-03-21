#include "stdafx.h"
#include "../inc/BigEndianProcessor.h"

EightBit::BigEndianProcessor::BigEndianProcessor(Bus& memory) noexcept
: Processor(memory) {}

EightBit::BigEndianProcessor::BigEndianProcessor(const BigEndianProcessor& rhs) noexcept
: Processor(rhs) {}

EightBit::register16_t EightBit::BigEndianProcessor::getWord() noexcept {
	const auto high = memoryRead();
	++BUS().ADDRESS();
	const auto low = memoryRead();
	return { low, high };
}

void EightBit::BigEndianProcessor::setWord(const register16_t value) noexcept {
	memoryWrite(value.high);
	++BUS().ADDRESS();
	memoryWrite(value.low);
}

EightBit::register16_t EightBit::BigEndianProcessor::getWordPaged() noexcept {
	const auto high = memoryRead();
	++BUS().ADDRESS().low;
	const auto low = memoryRead();
	return { low, high };
}

void EightBit::BigEndianProcessor::setWordPaged(const register16_t value) noexcept {
	memoryWrite(value.high);
	++BUS().ADDRESS().low;
	memoryWrite(value.low);
}

EightBit::register16_t EightBit::BigEndianProcessor::fetchWord() noexcept {
	intermediate().high = fetchByte();
	intermediate().low = fetchByte();
	return intermediate();
}

void EightBit::BigEndianProcessor::pushWord(const register16_t value) noexcept {
	push(value.low);
	push(value.high);
}

EightBit::register16_t EightBit::BigEndianProcessor::popWord() noexcept {
	const auto high = pop();
	const auto low = pop();
	return { low, high };
}

EightBit::register16_t EightBit::BigEndianProcessor::peekWord(const register16_t address) noexcept {
	const auto high = BUS().peek(address);
	const auto low = BUS().peek(address + 1);
	return { low, high };
}

void EightBit::BigEndianProcessor::pokeWord(const register16_t address, const register16_t value) noexcept {
	BUS().poke(address, value.high);
	BUS().poke(address + 1, value.low);
}
