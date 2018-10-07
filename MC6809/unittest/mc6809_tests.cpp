#include "pch.h"
#include "Board.h"

// Using examples from 6809 Assembly Language Programming, by Lance A. Leventhal
// Just test the basics...

TEST_CASE("Add Accumulator B to Index Register X Unsigned", "[ABX]") {

	Board board;
	board.initialise();
	auto& cpu = board.CPU();
	cpu.step();	// Step over the reset

    SECTION("Inherent") {
		board.poke(0, 0x3a);
		cpu.B() = 0x84;
		cpu.X() = 0x1097;
		cpu.step();
		REQUIRE(cpu.X() == 0x111b);
		REQUIRE(cpu.cycles() == 3);
    }
}

TEST_CASE("Add Memory Plus Carry to Accumulator A", "[ADC][ADCA]") {

	Board board;
	board.initialise();
	auto& cpu = board.CPU();
	cpu.step();	// Step over the reset

    SECTION("Immediate (byte)") {
		board.poke(0, 0x89);
		board.poke(1, 0x7c);
		EightBit::Chip::setFlag(cpu.CC(), EightBit::mc6809::CF);
		cpu.A() = 0x3a;
		cpu.step();
		REQUIRE(cpu.A() == 0xb7);
		REQUIRE((cpu.CC() & EightBit::mc6809::ZF) == 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::HF) != 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::VF) != 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::NF) != 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::CF) == 0);
		REQUIRE(cpu.cycles() == 2);
    }
}

TEST_CASE("Add Memory to Accumulator A", "[ADD][ADDA]") {

	Board board;
	board.initialise();
	auto& cpu = board.CPU();
	cpu.step();	// Step over the reset

    SECTION("Immediate (byte)") {
		board.poke(0, 0x8b);
		board.poke(1, 0x8b);
		cpu.A() = 0x24;
		cpu.step();
		REQUIRE(cpu.A() == 0xaf);
		REQUIRE((cpu.CC() & EightBit::mc6809::ZF) == 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::HF) == 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::VF) == 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::NF) != 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::CF) == 0);
		REQUIRE(cpu.cycles() == 2);
    }
}

TEST_CASE("Logical AND Accumulator", "[AND][ANDA]") {

	Board board;
	board.initialise();
	auto& cpu = board.CPU();
	cpu.step();	// Step over the reset

    SECTION("Immediate (byte)") {
		board.poke(0, 0x84);
		board.poke(1, 0x13);
		cpu.A() = 0xfc;
		cpu.step();
		REQUIRE(cpu.A() == 0x10);
		REQUIRE((cpu.CC() & EightBit::mc6809::ZF) == 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::VF) == 0);
		REQUIRE((cpu.CC() & EightBit::mc6809::NF) == 0);
		REQUIRE(cpu.cycles() == 2);
    }
}
