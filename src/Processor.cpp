#include "stdafx.h"
#include "../inc/Processor.h"

EightBit::Processor::Processor(Bus& bus) noexcept
: m_bus(bus) {
}

EightBit::Processor::Processor(const Processor& rhs)
: ClockedChip(rhs),
  m_bus(rhs.m_bus) {
	RESET() = rhs.RESET();
	INT() = rhs.INT();
	PC() = rhs.PC();
}

DEFINE_PIN_LEVEL_CHANGERS(RESET, Processor)
DEFINE_PIN_LEVEL_CHANGERS(INT, Processor)

void EightBit::Processor::handleRESET() noexcept {
	raiseRESET();
}

void EightBit::Processor::handleINT() noexcept {
	raiseINT();
}

void EightBit::Processor::memoryWrite(const register16_t address, const uint8_t data) noexcept {
	BUS().ADDRESS() = address;
	memoryWrite(data);
}

void EightBit::Processor::memoryWrite(const register16_t address)  noexcept {
	BUS().ADDRESS() = address;
	memoryWrite();
}

void EightBit::Processor::memoryWrite(const uint8_t data)  noexcept {
	BUS().DATA() = data;
	memoryWrite();
}

void EightBit::Processor::memoryWrite()  noexcept {
	busWrite();
}

void EightBit::Processor::busWrite() noexcept {
	BUS().write();
}

uint8_t EightBit::Processor::memoryRead(const register16_t address) noexcept {
	BUS().ADDRESS() = address;
	return memoryRead();
}

uint8_t EightBit::Processor::memoryRead() noexcept {
	return busRead();
}

uint8_t EightBit::Processor::busRead() noexcept {
	return BUS().read();
}

uint8_t EightBit::Processor::getBytePaged(const uint8_t page, const uint8_t offset) noexcept {
	return memoryRead(register16_t(offset, page));
}

void EightBit::Processor::setBytePaged(const uint8_t page, const uint8_t offset, const uint8_t value) noexcept {
	memoryWrite(register16_t(offset, page), value);
}

uint8_t EightBit::Processor::fetchByte() noexcept {
	return memoryRead(PC()++);
}

EightBit::register16_t EightBit::Processor::getWord(const register16_t address) noexcept {
	BUS().ADDRESS() = address;
	return getWord();
}

void EightBit::Processor::setWord(const register16_t address, const register16_t value) noexcept {
	BUS().ADDRESS() = address;
	setWord(value);
}

int EightBit::Processor::run(const int limit) noexcept {
	int current = 0;
	while (LIKELY(powered() && (current < limit)))
		current += step();
	return current;
}

int EightBit::Processor::execute(const uint8_t value) noexcept {
	opcode() = value;
	return execute();
}

void EightBit::Processor::jump(const register16_t destination) noexcept {
	PC() = destination;
}

void EightBit::Processor::call(const register16_t destination) noexcept {
	pushWord(PC());
	jump(destination);
}

void EightBit::Processor::ret() noexcept {
	jump(popWord());
}

bool EightBit::Processor::operator==(const EightBit::Processor& rhs) const {
	return
		ClockedChip::operator==(rhs)
		&& RESET() == rhs.RESET()
		&& INT() == rhs.INT()
		&& PC() == rhs.PC();
}
