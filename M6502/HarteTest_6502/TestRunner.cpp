#include "stdafx.h"
#include "TestRunner.h"

TestRunner::TestRunner() {}

void TestRunner::raisePOWER() noexcept {
    EightBit::Bus::raisePOWER();
    CPU().raisePOWER();
    CPU().raiseRESET();
    CPU().raiseINT();
    CPU().raiseNMI();
    CPU().raiseSO();
    CPU().raiseRDY();
}

void TestRunner::lowerPOWER() noexcept {
    CPU().lowerPOWER();
    EightBit::Bus::lowerPOWER();
}

void TestRunner::initialise() noexcept {}
