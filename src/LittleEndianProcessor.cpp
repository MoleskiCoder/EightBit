#include "stdafx.h"
#include "LittleEndianProcessor.h"

EightBit::LittleEndianProcessor::LittleEndianProcessor(Bus& memory)
: Processor(memory) {}

EightBit::register16_t EightBit::LittleEndianProcessor::getWord() {
	const auto low = BUS().read();
	++BUS().ADDRESS();
	const auto high = BUS().read();
	return register16_t(low, high);
}

void EightBit::LittleEndianProcessor::setWord(const register16_t value) {
	BUS().write(value.low);
	++BUS().ADDRESS();
	BUS().write(value.high);
}

EightBit::register16_t EightBit::LittleEndianProcessor::getWordPaged(uint8_t page, uint8_t offset) {
	const auto low = getBytePaged(page, offset);
	++BUS().ADDRESS().low;
	const auto high = BUS().read();
	return register16_t(low, high);
}

void EightBit::LittleEndianProcessor::setWordPaged(uint8_t page, uint8_t offset, register16_t value) {
	setBytePaged(page, offset, value.low);
	++BUS().ADDRESS().low;
	BUS().read(value.high);
}

EightBit::register16_t EightBit::LittleEndianProcessor::fetchWord() {
	const auto low = fetchByte();
	const auto high = fetchByte();
	return register16_t(low, high);
}

void EightBit::LittleEndianProcessor::pushWord(const register16_t value) {
	push(value.high);
	push(value.low);
}

EightBit::register16_t EightBit::LittleEndianProcessor::popWord() {
	const auto low = pop();
	const auto high = pop();
	return register16_t(low, high);
}
