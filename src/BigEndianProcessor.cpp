#include "stdafx.h"
#include "BigEndianProcessor.h"

EightBit::BigEndianProcessor::BigEndianProcessor(Bus& memory)
: Processor(memory) {}

EightBit::register16_t EightBit::BigEndianProcessor::getWord() {
	const auto high = BUS().read();
	++BUS().ADDRESS();
	const auto low = BUS().read();
	return register16_t(low, high);
}

void EightBit::BigEndianProcessor::setWord(const register16_t value) {
	BUS().write(value.high);
	++BUS().ADDRESS();
	BUS().write(value.low);
}

EightBit::register16_t EightBit::BigEndianProcessor::getWordPaged(uint8_t page, uint8_t offset) {
	const auto high = getBytePaged(page, offset);
	++BUS().ADDRESS().low;
	const auto low = BUS().read();
	return register16_t(low, high);
}

void EightBit::BigEndianProcessor::setWordPaged(uint8_t page, uint8_t offset, register16_t value) {
	setBytePaged(page, offset, value.high);
	++BUS().ADDRESS().low;
	BUS().read(value.low);
}

EightBit::register16_t EightBit::BigEndianProcessor::fetchWord() {
	const auto high = fetchByte();
	const auto low = fetchByte();
	return register16_t(low, high);
}

void EightBit::BigEndianProcessor::pushWord(const register16_t value) {
	push(value.low);
	push(value.high);
}

EightBit::register16_t EightBit::BigEndianProcessor::popWord() {
	const auto high = pop();
	const auto low = pop();
	return register16_t(low, high);
}