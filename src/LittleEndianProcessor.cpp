#include "stdafx.h"
#include "LittleEndianProcessor.h"

EightBit::LittleEndianProcessor::LittleEndianProcessor(Bus& memory)
: Processor(memory) {}

EightBit::register16_t EightBit::LittleEndianProcessor::getWord() {
	const auto low = BUS().read();
	++BUS().ADDRESS();
	const auto high = BUS().read();
	return { low, high };
}

void EightBit::LittleEndianProcessor::setWord(const register16_t value) {
	BUS().write(value.low);
	++BUS().ADDRESS();
	BUS().write(value.high);
}

EightBit::register16_t EightBit::LittleEndianProcessor::getWordPaged(const uint8_t page, const uint8_t offset) {
	const auto low = getBytePaged(page, offset);
	++BUS().ADDRESS().low;
	const auto high = BUS().read();
	return { low, high };
}

void EightBit::LittleEndianProcessor::setWordPaged(const uint8_t page, const uint8_t offset, const register16_t value) {
	setBytePaged(page, offset, value.low);
	++BUS().ADDRESS().low;
	BUS().write(value.high);
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

EightBit::register16_t EightBit::LittleEndianProcessor::peekWord(const register16_t address) {
	const auto low = BUS().peek(address);
	const auto high = BUS().peek(address + 1);
	return { low, high };
}

void EightBit::LittleEndianProcessor::pokeWord(const register16_t address, const register16_t value) {
	BUS().poke(address, value.low);
	BUS().poke(address + 1, value.high);
}
