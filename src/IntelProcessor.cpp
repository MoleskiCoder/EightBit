#include "stdafx.h"
#include "../inc/IntelProcessor.h"

std::array<int, 8> EightBit::IntelProcessor::m_halfCarryTableAdd = { { 0, 0, 1, 0, 1, 0, 1, 1 } };
std::array<int, 8> EightBit::IntelProcessor::m_halfCarryTableSub = { { 0, 1, 1, 1, 0, 0, 0, 1 } };

EightBit::IntelProcessor::IntelProcessor(Bus& bus) noexcept
: LittleEndianProcessor(bus) {
	for (int i = 0; i < 0x100; ++i)
		m_decodedOpcodes.at(i) = i;

	RaisedPOWER.connect([this](EventArgs) {
		PC() = SP() = Mask16;
		resetRegisterSet();
		raiseHALT();
	});
}

EightBit::IntelProcessor::IntelProcessor(const IntelProcessor& rhs) noexcept
: LittleEndianProcessor(rhs),
  m_sp(rhs.m_sp),
  m_memptr(rhs.m_memptr) {
	HALT() = rhs.HALT();
}

void EightBit::IntelProcessor::resetRegisterSet() noexcept {
	AF() = BC() = DE() = HL() = Mask16;
}

DEFINE_PIN_LEVEL_CHANGERS(HALT, IntelProcessor);

void EightBit::IntelProcessor::handleRESET() noexcept {
	Processor::handleRESET();
	disableInterrupts();
	Processor::jump(0);
}

void EightBit::IntelProcessor::handleINT() noexcept {
	Processor::handleINT();
	raiseHALT();
	disableInterrupts();
}

void EightBit::IntelProcessor::push(const uint8_t value) noexcept {
	memoryWrite(--SP(), value);
}

void EightBit::IntelProcessor::pop() noexcept {
	memoryRead(SP()++);
}

EightBit::register16_t& EightBit::IntelProcessor::incrementPC() noexcept {
	if (proceeding())
		Processor::incrementPC();
	return PC();
}

uint8_t EightBit::IntelProcessor::fetchInstruction() noexcept {
	fetchByte();
	return proceeding() ? BUS().DATA() : (uint8_t)0;
}

void EightBit::IntelProcessor::getShort() noexcept {
	LittleEndianProcessor::getShort();
	MEMPTR() = BUS().ADDRESS();
}

void EightBit::IntelProcessor::setShort(const register16_t value) noexcept {
	LittleEndianProcessor::setShort(value);
	MEMPTR() = BUS().ADDRESS();
}

void EightBit::IntelProcessor::restart(const uint8_t address) noexcept {
	MEMPTR() = { address, 0 };
	call();
}

void EightBit::IntelProcessor::callConditional(bool condition) noexcept {
	fetchInto(MEMPTR());
	if (condition)
		call();
}

void EightBit::IntelProcessor::jumpConditional(bool condition) noexcept {
	fetchInto(MEMPTR());
	if (condition)
		jump();
}

void EightBit::IntelProcessor::jumpRelativeConditional(bool condition) noexcept {
	fetchByte();
	if (condition)
		jumpRelative(BUS().DATA());
}

void EightBit::IntelProcessor::returnConditional(bool condition) noexcept {
	if (condition)
		ret();
}

void EightBit::IntelProcessor::jumpRelative(const int8_t offset) noexcept {
	MEMPTR() = PC() + offset;
	jump();
}

void EightBit::IntelProcessor::ret() noexcept {
	LittleEndianProcessor::ret();
	MEMPTR() = PC();
}

void EightBit::IntelProcessor::jumpIndirect() noexcept {
	fetchInto(MEMPTR());
	jump();
}

void EightBit::IntelProcessor::jump() noexcept {
	Processor::jump(MEMPTR());
}

void EightBit::IntelProcessor::callIndirect() noexcept {
	fetchInto(MEMPTR());
	call();
}

void EightBit::IntelProcessor::call() noexcept {
	LittleEndianProcessor::call(MEMPTR());
}

void EightBit::IntelProcessor::jumpConditionalFlag(int flag) noexcept {
	jumpConditional(convertCondition(flag));
}

void EightBit::IntelProcessor::jumpRelativeConditionalFlag(int flag) noexcept {
	jumpRelativeConditional(convertCondition(flag));
}

void EightBit::IntelProcessor::returnConditionalFlag(int flag) noexcept {
	returnConditional(convertCondition(flag));
}

void EightBit::IntelProcessor::callConditionalFlag(int flag) noexcept {
	callConditional(convertCondition(flag));
}

void EightBit::IntelProcessor::cpl() noexcept {
	A() = ~A();
}

bool EightBit::IntelProcessor::operator==(const EightBit::IntelProcessor& rhs) const noexcept {
	auto& left = const_cast<IntelProcessor&>(*this);
	auto& right = const_cast<IntelProcessor&>(rhs);
	return
		LittleEndianProcessor::operator==(rhs)
		&& left.HALT() == right.HALT()
		&& left.MEMPTR() == right.MEMPTR()
		&& left.SP() == right.SP()
		&& left.AF() == right.AF()
		&& left.BC() == right.BC()
		&& left.DE() == right.DE()
		&& left.HL() == right.HL();
}

void EightBit::IntelProcessor::readMemoryIndirect(register16_t via) noexcept {
	MEMPTR() = via;
	readMemoryIndirect();
}

void EightBit::IntelProcessor::readMemoryIndirect() noexcept {
	BUS().ADDRESS() = MEMPTR();
	++MEMPTR();
	memoryRead();
}

void EightBit::IntelProcessor::writeMemoryIndirect(register16_t via, uint8_t data) noexcept {
	MEMPTR() = via;
	writeMemoryIndirect(data);
}

void EightBit::IntelProcessor::writeMemoryIndirect(uint8_t data) noexcept {
	BUS().ADDRESS() = MEMPTR();
	++MEMPTR();
	MEMPTR().high = BUS().DATA() = data;
	memoryWrite();
}

void EightBit::IntelProcessor::R(int r, uint8_t value, int ticks) noexcept {
	R(r, MemoryMapping::AccessLevel::WriteOnly) = value;
	if (r == 6)
		memoryUpdate(ticks);
}
