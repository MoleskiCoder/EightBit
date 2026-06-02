#include "stdafx.h"
#include "../inc/Processor.h"

EightBit::Processor::Processor(Bus& bus) noexcept
: m_bus(bus) {
}

EightBit::Processor::Processor(const Processor& rhs) noexcept
: ClockedChip(rhs),
  m_bus(rhs.m_bus),
  m_opcode(rhs.m_opcode),
  m_pc(rhs.m_pc),
  m_intermediate(rhs.m_intermediate) {
	RESET() = rhs.RESET();
	INT() = rhs.INT();
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

void EightBit::Processor::memoryWrite(const register16_t address) noexcept {
	BUS().ADDRESS() = address;
	memoryWrite();
}

void EightBit::Processor::memoryWrite(const uint8_t data) noexcept {
	BUS().DATA() = data;
	memoryWrite();
}

void EightBit::Processor::memoryWrite() noexcept {
	busWrite();
}

void EightBit::Processor::busWrite() noexcept {
	BUS().write();
}

void EightBit::Processor::memoryRead(const register16_t address) noexcept {
	BUS().ADDRESS() = address;
	memoryRead();
}

void EightBit::Processor::memoryRead() noexcept {
	busRead();
}

void EightBit::Processor::busRead() noexcept {
	BUS().read();
}

void EightBit::Processor::getShortPaged(register16_t address) noexcept {
	BUS().ADDRESS() = address;
	getShortPaged();
}

void EightBit::Processor::getShortPaged(const uint8_t page, const uint8_t offset) noexcept {
	BUS().ADDRESS() = { offset, page };
	getShortPaged();
}

void EightBit::Processor::getPagedInto(uint8_t page, uint8_t offset, register16_t& into) noexcept {
	BUS().ADDRESS() = { offset, page };
	getPagedInto(into);
}

void EightBit::Processor::setPaged(register16_t address, const register16_t value) noexcept {
	BUS().ADDRESS() = address;
	setPaged(value);
}

void EightBit::Processor::setPaged(const uint8_t page, const uint8_t offset, const register16_t value) noexcept {
	BUS().ADDRESS() = { offset, page };
	setPaged(value);
}

EightBit::register16_t& EightBit::Processor::incrementPC() noexcept {
	return ++PC();
}

void EightBit::Processor::immediateAddress() noexcept {
	BUS().ADDRESS() = PC();
	incrementPC();
}

void EightBit::Processor::fetchByte() noexcept {
	immediateAddress();
	memoryRead();
}

uint8_t EightBit::Processor::fetchInstruction() noexcept {
	fetchByte();
	return BUS().DATA();
}

void EightBit::Processor::fetchShortAddress() noexcept {
	fetchShort();
	BUS().ADDRESS() = m_intermediate;
}

void EightBit::Processor::getShort(const register16_t address) noexcept {
	BUS().ADDRESS() = address;
	getShort();
}

void EightBit::Processor::setShort(const register16_t address, const register16_t value) noexcept {
	BUS().ADDRESS() = address;
	setShort(value);
}

int EightBit::Processor::step() noexcept {
	resetCycles();
	ExecutingInstruction.fire();
	if (powered())
		poweredStep();
	ExecutedInstruction.fire();
	return cycles();
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

void EightBit::Processor::jump(const uint16_t destination) noexcept {
	PC() = destination;
}

void EightBit::Processor::call(const register16_t destination) noexcept {
	pushShort(PC());
	jump(destination);
}

void EightBit::Processor::ret() noexcept {
	popInto(PC());
}

bool EightBit::Processor::operator==(const EightBit::Processor& rhs) const noexcept {
	return
		ClockedChip::operator==(rhs)
		&& RESET() == rhs.RESET()
		&& INT() == rhs.INT()
		&& PC() == rhs.PC();
}
