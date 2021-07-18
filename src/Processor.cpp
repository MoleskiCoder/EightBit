#include "stdafx.h"
#include "../inc/Processor.h"

EightBit::Processor::Processor(Bus& bus) noexcept
: m_bus(bus) {
}

DEFINE_PIN_LEVEL_CHANGERS(RESET, Processor);
DEFINE_PIN_LEVEL_CHANGERS(INT, Processor);

void EightBit::Processor::handleRESET() {
	raiseRESET();
}

void EightBit::Processor::handleINT() {
	raiseINT();
}

void EightBit::Processor::memoryWrite(const register16_t address, const uint8_t data) {
	BUS().ADDRESS() = address;
	memoryWrite(data);
}

void EightBit::Processor::memoryWrite(const register16_t address) {
	BUS().ADDRESS() = address;
	memoryWrite();
}

void EightBit::Processor::memoryWrite(const uint8_t data) {
	BUS().DATA() = data;
	memoryWrite();
}

void EightBit::Processor::memoryWrite() {
	busWrite();
}

void EightBit::Processor::busWrite() {
	BUS().write();
}

uint8_t EightBit::Processor::memoryRead(const register16_t address) {
	BUS().ADDRESS() = address;
	return memoryRead();
}

uint8_t EightBit::Processor::memoryRead() {
	return busRead();
}

uint8_t EightBit::Processor::busRead() {
	return BUS().read();
}

uint8_t EightBit::Processor::getBytePaged(const uint8_t page, const uint8_t offset) {
	return memoryRead(register16_t(offset, page));
}

void EightBit::Processor::setBytePaged(const uint8_t page, const uint8_t offset, const uint8_t value) {
	memoryWrite(register16_t(offset, page), value);
}

uint8_t EightBit::Processor::fetchByte() {
	return memoryRead(PC()++);
}

EightBit::register16_t EightBit::Processor::getWord(const register16_t address) {
	BUS().ADDRESS() = address;
	return getWord();
}

void EightBit::Processor::setWord(const register16_t address, const register16_t value) {
	BUS().ADDRESS() = address;
	setWord(value);
}

int EightBit::Processor::run(const int limit) {
	int current = 0;
	while (LIKELY(powered() && (current < limit)))
		current += step();
	return current;
}

int EightBit::Processor::execute(const uint8_t value) {
	opcode() = value;
	return execute();
}

void EightBit::Processor::jump(const register16_t destination) noexcept {
	PC() = destination;
}

void EightBit::Processor::call(const register16_t destination) {
	pushWord(PC());
	jump(destination);
}

void EightBit::Processor::ret() {
	jump(popWord());
}
