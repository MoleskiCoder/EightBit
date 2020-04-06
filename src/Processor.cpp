#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Bus& bus)
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

// http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
int8_t EightBit::Processor::signExtend(const int b, uint8_t x) noexcept {
	const uint8_t m = bit(b - 1); // mask can be pre-computed if b is fixed
	x = x & (bit(b) - 1);  // (Skip this if bits in x above position b are already zero.)
	const auto result = (x ^ m) - m;
	return result;
}

void EightBit::Processor::ret() {
	jump(popWord());
}
