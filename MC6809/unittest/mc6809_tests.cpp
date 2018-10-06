#include "pch.h"
#include "Board.h"

// Using examples from 6809 Assembly Language Programming, by Lance A. Leventhal
// Just test the basics...

TEST_CASE("Add Accumulator B to Index Register X Unsigned ", "[ABX]") {

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
    }
}
