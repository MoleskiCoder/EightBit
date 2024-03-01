#include "stdafx.h"
#include "../inc/LittleEndianProcessor.h"

EightBit::LittleEndianProcessor::LittleEndianProcessor(Bus& memory) noexcept
: Processor(memory) {}

EightBit::LittleEndianProcessor::LittleEndianProcessor(const LittleEndianProcessor& rhs) noexcept
: Processor(rhs) {}

EightBit::register16_t EightBit::LittleEndianProcessor::getWord() {
	const auto low = memoryRead();
	++BUS().ADDRESS();
	const auto high = memoryRead();
	return { low, high };
}

void EightBit::LittleEndianProcessor::setWord(const register16_t value) {
	memoryWrite(value.low);
	++BUS().ADDRESS();
	memoryWrite(value.high);
}

EightBit::register16_t EightBit::LittleEndianProcessor::getWordPaged() {
	const auto low = getBytePaged();
	++BUS().ADDRESS().low;
	const auto high = memoryRead();
	return { low, high };
}

void EightBit::LittleEndianProcessor::setWordPaged(register16_t value) {
	setBytePaged(value.low);
	++BUS().ADDRESS().low;
	memoryWrite(value.high);
}

EightBit::register16_t EightBit::LittleEndianProcessor::fetchWord() {
	const auto low = fetchByte();
	const auto high = fetchByte();
	return { low, high };
}

void EightBit::LittleEndianProcessor::pushWord(const register16_t value) {
	push(value.high);
	push(value.low);
}

EightBit::register16_t EightBit::LittleEndianProcessor::popWord() {
	const auto low = pop();
	const auto high = pop();
	return { low, high };
}

EightBit::register16_t EightBit::LittleEndianProcessor::peekWord(const register16_t address) noexcept {
	const auto low = BUS().peek(address);
	const auto high = BUS().peek(address + 1);
	return { low, high };
}

void EightBit::LittleEndianProcessor::pokeWord(const register16_t address, const register16_t value) noexcept {
	BUS().poke(address, value.low);
	BUS().poke(address + 1, value.high);
}
