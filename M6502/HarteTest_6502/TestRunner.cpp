#include "stdafx.h"
#include "TestRunner.h"

TestRunner::TestRunner() {}

EightBit::MemoryMapping TestRunner::mapping(const uint16_t address) noexcept {
    return { RAM(), 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };
}

void TestRunner::raisePOWER() {
    EightBit::Bus::raisePOWER();
    CPU().raisePOWER();
    CPU().raiseRESET();
    CPU().raiseINT();
    CPU().raiseNMI();
    CPU().raiseSO();
    CPU().raiseRDY();
}

void TestRunner::lowerPOWER() {
    CPU().lowerPOWER();
    EightBit::Bus::lowerPOWER();
}

void TestRunner::initialise() {}
