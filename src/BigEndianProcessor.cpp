#include "stdafx.h"
#include "../inc/BigEndianProcessor.h"

EightBit::BigEndianProcessor::BigEndianProcessor(Bus& memory) noexcept
: Processor(memory) {}

EightBit::register16_t EightBit::BigEndianProcessor::getWord() {
	const auto high = memoryRead();
	++BUS().ADDRESS();
	const auto low = memoryRead();
	return { low, high };
}

void EightBit::BigEndianProcessor::setWord(const register16_t value) {
	memoryWrite(value.high);
	++BUS().ADDRESS();
	memoryWrite(value.low);
}

EightBit::register16_t EightBit::BigEndianProcessor::getWordPaged(const uint8_t page, const uint8_t offset) {
	const auto high = getBytePaged(page, offset);
	++BUS().ADDRESS().low;
	const auto low = memoryRead();
	return { low, high };
}

void EightBit::BigEndianProcessor::setWordPaged(const uint8_t page, const uint8_t offset, const register16_t value) {
	setBytePaged(page, offset, value.high);
	++BUS().ADDRESS().low;
	memoryWrite(value.low);
}

EightBit::register16_t EightBit::BigEndianProcessor::fetchWord() {
	const auto high = fetchByte();
	const auto low = fetchByte();
	return { low, high };
}

void EightBit::BigEndianProcessor::pushWord(const register16_t value) {
	push(value.low);
	push(value.high);
}

EightBit::register16_t EightBit::BigEndianProcessor::popWord() {
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
