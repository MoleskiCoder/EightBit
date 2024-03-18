#include "stdafx.h"
#include "../inc/Processor.h"

EightBit::Processor::Processor(Bus& bus) noexcept
: m_bus(bus) {
}

EightBit::Processor::Processor(const Processor& rhs) noexcept
: ClockedChip(rhs),
  m_bus(rhs.m_bus) {
	RESET() = rhs.RESET();
	INT() = rhs.INT();
	PC() = rhs.PC();
}

DEFINE_PIN_LEVEL_CHANGERS(RESET, Processor)
DEFINE_PIN_LEVEL_CHANGERS(INT, Processor)

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

EightBit::register16_t EightBit::Processor::getWordPaged(register16_t address) {
	BUS().ADDRESS() = address;
	return getWordPaged();
}

EightBit::register16_t EightBit::Processor::getWordPaged(const uint8_t page, const uint8_t offset) {
	BUS().ADDRESS() = { offset, page };
	return getWordPaged();
}

void EightBit::Processor::setWordPaged(register16_t address, const register16_t value) {
	BUS().ADDRESS() = address;
	setWordPaged(value);
}

void EightBit::Processor::setWordPaged(const uint8_t page, const uint8_t offset, const register16_t value) {
	BUS().ADDRESS() = { offset, page };
	setWordPaged(value);
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

int EightBit::Processor::run(const int limit) noexcept {
	int current = 0;
	while (LIKELY(powered() && (current < limit)))
		current += step();
	return current;
}

void EightBit::Processor::execute(const uint8_t value) noexcept {
	opcode() = value;
	execute();
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

bool EightBit::Processor::operator==(const EightBit::Processor& rhs) const noexcept {
	return
		ClockedChip::operator==(rhs)
		&& RESET() == rhs.RESET()
		&& INT() == rhs.INT()
		&& PC() == rhs.PC();
}
