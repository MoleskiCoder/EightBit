#include "pch.h"
#include "Board.h"

// Some tests from:
// 6809 Assembly Language Programming, by Lance A. Leventhal
// https://github.com/sorenroug/osnine-java/blob/master/core/cputests/
// void setRegs(a, b, x, y, u)

TEST_CASE("Add Accumulator B to Index Register X Unsigned", "[ABX]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Inherent") {
		board.poke(0, 0x3a);
		cpu.B() = 0x84;
		cpu.X() = 0x1097;
		cycles = cpu.step();
		REQUIRE(cpu.X() == 0x111b);
		REQUIRE(cycles == 3);
    }

    SECTION("Inherent test (ABX1)") {
		cpu.A() = 0;
		cpu.B() = 0xce;
		cpu.X() = 0x8006;
		cpu.Y() = 0;
		cpu.U() = 0;
		cpu.CC() = 0;
		board.poke(0, 0x3a);
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0);
		REQUIRE(cpu.B() == 0xce);
		REQUIRE(cpu.X() == 0x80d4);
		REQUIRE(cpu.Y() == 0);
		REQUIRE(cpu.U() == 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.halfCarry() == 0);
		REQUIRE(cycles == 3);
    }

    SECTION("Inherent test (ABX2)") {
		cpu.A() = 0;
		cpu.B() = 0xd6;
		cpu.X() = 0x7ffe;
		cpu.Y() = 0;
		cpu.U() = 0;
		cpu.CC() = EightBit::mc6809::CF | EightBit::mc6809::VF | EightBit::mc6809::ZF;
		board.poke(0, 0x3a);
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0);
		REQUIRE(cpu.B() == 0xd6);
		REQUIRE(cpu.X() == 0x80d4);
		REQUIRE(cpu.Y() == 0);
		REQUIRE(cpu.U() == 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.zero() != 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.halfCarry() == 0);
		REQUIRE(cycles == 3);
    }
}

TEST_CASE("Add Memory Plus Carry to Accumulator", "[ADC][ADCA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Immediate (byte)") {
		board.poke(0, 0x89);
		board.poke(1, 0x7c);
		cpu.CC() = EightBit::Chip::setBit(cpu.CC(), EightBit::mc6809::CF);
		cpu.A() = 0x3a;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xb7);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.halfCarry() != 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }

	SECTION("Immediate (byte) ADCANoC1") {
		cpu.A() = 0x5;
		cpu.CC() = 0;
		board.poke(0, 0x89);
		board.poke(1, 0x02);
		cycles = cpu.step();
		REQUIRE(cpu.A() == 7);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.halfCarry() == 0);
		REQUIRE(cycles == 2);
    }

	/* Test half-carry $E + $2 = $10 */
	SECTION("Immediate (byte) ADCANoC2") {
		cpu.A() = 0xe;
		cpu.CC() = 0;
		board.poke(0, 0x89);
		board.poke(1, 0x02);
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x10);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.halfCarry() != 0);
		REQUIRE(cycles == 2);
    }

	/* Add $22 and carry to register A ($14) */
	SECTION("Immediate (byte) ADCAWiC") {
		cpu.A() = 0x14;
		cpu.CC() = EightBit::mc6809::CF | EightBit::mc6809::HF;
		board.poke(0, 0x89);
		board.poke(1, 0x22);
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x37);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.halfCarry() == 0);
		REQUIRE(cycles == 2);
    }

	/* Test that half-carry is set when adding with a carry */
	SECTION("Immediate (byte) ADCAWiHC") {
		cpu.A() = 0x14;
		cpu.CC() = EightBit::mc6809::CF;
		board.poke(0, 0x89);
		board.poke(1, 0x2B);
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x40);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.halfCarry() != 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Add Memory to Accumulator", "[ADD][ADDA][ADDB][ADDD]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Immediate (byte)") {
		board.poke(0, 0x8b);
		board.poke(1, 0x8b);
		cpu.A() = 0x24;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xaf);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.halfCarry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }

	// Add 0x02 to A=0x04.
	SECTION("Immediate (byte) (ADDANoC)") {
		board.poke(0, 0x8b);
		board.poke(1, 0x02);
		cpu.CC() = 0;
		cpu.A() = 4;
		cpu.B() = 5;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 6);
		REQUIRE(cpu.B() == 5);
		REQUIRE(cpu.halfCarry() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }

	// The overflow (V) bit indicates signed two�s complement overflow, which occurs when the
	// sign bit differs from the carry bit after an arithmetic operation.
	// A=0x03 + 0xFF becomes 0x02
	SECTION("Immediate (byte) (ADDAWiC)") {
		board.poke(0, 0x8b);
		board.poke(1, 0xff);
		cpu.CC() = 0;
		cpu.A() = 3;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 2);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 2);
    }

	// positive + positive with overflow.
	// B=0x40 + 0x41 becomes 0x81 or -127
    SECTION("Immediate (byte) ADDB1") {
		board.poke(0, 0xcb);
		board.poke(1, 0x41);
		cpu.B() = 0x40;
		cpu.CC() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.B() == 0x81);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.halfCarry() == 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }

	// negative + negative.
	// B=0xFF + 0xFF becomes 0xFE or -2
    SECTION("Immediate (byte) ADDB2") {
		board.poke(0, 0xcb);
		board.poke(1, 0xff);
		cpu.B() = 0xff;
		cpu.CC() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.B() == 0xfe);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 2);
    }

	// negative + negative with overflow.
	// B=0xC0 + 0xBF becomes 0x7F or 127
    SECTION("Immediate (byte) ADDB3") {
		board.poke(0, 0xcb);
		board.poke(1, 0xbf);
		cpu.B() = 0xc0;
		cpu.CC() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.B() == 0x7f);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 2);
    }

	// positive + negative with negative result.
	// B=0x02 + 0xFC becomes 0xFE or -2
    SECTION("Immediate (byte) ADDB4") {
		board.poke(0, 0xcb);
		board.poke(1, 0xfc);
		cpu.B() = 0x02;
		cpu.CC() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.B() == 0xfe);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }

	// Add 0x02B0 to D=0x0405 becomes 0x6B5.
	// positive + positive = positive
	SECTION("Immediate (word) (ADDDNoC)") {
		board.poke(0, 0xc3);
		board.poke(1, 0x02);
		board.poke(2, 0xb0);
		cpu.CC() = 0;
		cpu.A() = 4;
		cpu.B() = 5;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x06);
		REQUIRE(cpu.B() == 0xb5);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 4);
    }

	// Add 0xE2B0 to D=0x8405 becomes 0x66B5.
	// negative + negative = positive + overflow
	SECTION("Immediate (word) (ADDD1)") {
		board.poke(0, 0xc3);
		board.poke(1, 0xe2);
		board.poke(2, 0xb0);
		cpu.CC() = 0;
		cpu.A() = 0x84;
		cpu.B() = 5;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x66);
		REQUIRE(cpu.B() == 0xb5);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 4);
    }

	// negative + negative = negative.
	// Add 0xE000 to D=0xD000 becomes 0xB000
	SECTION("Immediate (word) (ADDD2)") {
		board.poke(0, 0xc3);
		board.poke(1, 0xe0);
		board.poke(2, 0);
		cpu.CC() = 0;
		cpu.A() = 0xd0;
		cpu.B() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xb0);
		REQUIRE(cpu.B() == 0x00);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cpu.cycles() == 4);
    }

	// positive + positive = negative + overflow.
	// Add 0x7000 to D=0x7000 becomes 0xE000
	SECTION("Immediate (word) (ADDD3)") {
		board.poke(0, 0xc3);
		board.poke(1, 0x70);
		board.poke(2, 0);
		cpu.CC() = 0;
		cpu.A() = 0x70;
		cpu.B() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xe0);
		REQUIRE(cpu.B() == 0x00);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.cycles() == 4);
    }
}

TEST_CASE("Logical AND Accumulator", "[AND][ANDA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Immediate (byte)") {
		board.poke(0, 0x84);
		board.poke(1, 0x13);
		cpu.A() = 0xfc;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x10);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Shift Accumulator or Memory Byte Left", "[ASL][ASLA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Inherent") {
		board.poke(0, 0x48);
		cpu.A() = 0x7a;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xf4);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Shift Accumulator or Memory Byte Right", "[ASR][ASRA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Inherent") {
		board.poke(0, 0x47);
		cpu.A() = 0xcb;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xe5);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Bit Test", "[BIT][BITA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Immediate (byte)") {
		board.poke(0, 0x85);
		board.poke(1, 0xe0);
		cpu.A() = 0xa6;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xa6);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Clear Accumulator or Memory", "[CLR][CLRA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Implied") {
		board.poke(0, 0x4f);
		cpu.A() = 0x43;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x00);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.zero() != 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Compare Memory with a Register", "[CMP][CMPA][CMPB][CMPX]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

	SECTION("Immediate (byte)") {
		board.poke(0, 0x81);
		board.poke(1, 0x18);
		cpu.A() = 0xf6;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xf6);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
	}

	SECTION("Indirect mode: CMPA ,Y+ (CMP1)") {
		board.poke(0, 0xa1);
		board.poke(1, 0xa0);
		board.poke(0x205, 0xff);
		cpu.CC() = 0;
		cpu.A() = 0xff;
		cpu.B() = 0;
		cpu.X() = 0;
		cpu.Y() = 0x205;
		cpu.U() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xff);
		REQUIRE(cpu.B() == 0x00);
		REQUIRE(cpu.X() == 0x00);
		REQUIRE(cpu.Y() == 0x206);
		REQUIRE(cpu.U() == 0);
		REQUIRE(cpu.zero() != 0);
		REQUIRE(cycles == 6);
	}

	// B = 0xA0, CMPB with 0xA0
    SECTION("Immediate (CMP2)") {
		board.poke(0, 0xc1);
		board.poke(1, 0xa0);
		cpu.CC() = EightBit::mc6809::NF;
		cpu.B() = 0xa0;
		cycles = cpu.step();
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.zero() != 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }

	// B = 0x70, CMPB with 0xA0
    SECTION("Immediate (CMP3)") {
		board.poke(0, 0xc1);
		board.poke(1, 0xa0);
		cpu.CC() = EightBit::mc6809::NF;
		cpu.B() = 0x70;
		cycles = cpu.step();
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cycles == 2);
    }

	// Compare 0x5410 with 0x5410
    SECTION("Immediate (word) (CMP16)") {
		board.poke(0, 0xc1);
		board.poke(1, 0xa0);
		cpu.CC() = EightBit::mc6809::NF;
		cpu.B() = 0x70;
		cycles = cpu.step();
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cycles == 2);
    }

    SECTION("Immediate (word)") {
		board.poke(0, 0x8c);
		board.poke(1, 0x1b);
		board.poke(2, 0xb0);
		cpu.X() = 0x1ab0;
		cycles = cpu.step();
		REQUIRE(cpu.X() == 0x1ab0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 4);
    }
}

TEST_CASE("Decrement Accumulator or Memory", "[DEC][DECA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Inherent (DECA0x32)") {
		board.poke(0, 0x4a);
		cpu.CC() = 0;
		cpu.A() = 0x32;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x31);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cycles == 2);
    }

	// Test 0x80 - special case
    SECTION("Inherent (DECA0x80)") {
		board.poke(0, 0x4a);
		cpu.CC() = 0;
		cpu.A() = 0x80;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x7f);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cycles == 2);
    }

	// Test 0x00 - special case
    SECTION("Inherent (DECA0x00)") {
		board.poke(0, 0x4a);
		cpu.CC() = 0;
		cpu.A() = 0x00;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xff);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Increment Accumulator or Memory Location by 1", "[INC][INCA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Inherent (INCA1)") {
		board.poke(0, 0x4c);
		cpu.CC() = 0;
		cpu.A() = 0x32;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x33);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }

    SECTION("Inherent (INCA2)") {
		board.poke(0, 0x4c);
		cpu.CC() = 0;
		cpu.A() = 0x7f;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0x80);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }

    SECTION("Inherent (INCA3)") {
		board.poke(0, 0x4c);
		cpu.CC() = 0;
		cpu.A() = 0xff;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.zero() != 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Subtract Memory from Accumulator with Borrow (8-bit)", "[SBC][SBCA][SBCB]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

    SECTION("Immediate (byte)") {
		board.poke(0, 0x82);
		board.poke(1, 0x34);
		cpu.A() = 0x14;
		cpu.CC() = cpu.setBit(cpu.CC(), EightBit::mc6809::CF);
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xdf);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 2);
    }

	// Test the subtraction with carry instruction.
	// B=0x35 - addr(0x503)=0x3 - C=1 becomes 0x31
	// SBCB dp+03
    SECTION("Direct (SBCB)") {
		board.poke(0, 0xd2);
		board.poke(1, 0x03);
		board.poke(0x503, 0x03);
		cpu.DP() = 5;
		cpu.B() = 0x35;
		cpu.CC() = EightBit::mc6809::CF;
		cycles = cpu.step();
		REQUIRE(cpu.B() == 0x31);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cycles == 4);
    }

	// Test the SBCA instruction.
	// A=0xFF - 0xFE - C=1 becomes 0x00
    SECTION("Immediate (SBCA) (SBCA1)") {
		board.poke(0, 0x82);
		board.poke(1, 0xfe);
		board.poke(0x503, 0x03);
		cpu.CC() = EightBit::mc6809::CF | EightBit::mc6809::NF;
		cpu.A() = 0xff;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0);
		REQUIRE(cpu.carry() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() != 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cycles == 2);
    }

	// Test the SBCA instruction.
	// A=0x00 - 0xFF - C=0 becomes 0x01
    SECTION("Immediate (SBCA) (SBCA2)") {
		board.poke(0, 0x82);
		board.poke(1, 0xff);
		cpu.CC() = EightBit::mc6809::NF | EightBit::mc6809::VF;
		cpu.A() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 1);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cycles == 2);
    }

	// Test the SBCA instruction.
	// A=0x00 - 0x01 - C=0 becomes 0xFF
    SECTION("Immediate (SBCA) (SBCA3)") {
		board.poke(0, 0x82);
		board.poke(1, 0x01);
		cpu.CC() = EightBit::mc6809::NF | EightBit::mc6809::VF;
		cpu.A() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xff);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cycles == 2);
    }
}

TEST_CASE("Subtract Memory from Register", "[SUB][SUBA]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

	// Test the SUBA instruction.
	// The overflow (V) bit indicates signed two�s complement overflow, which
	// occurs when the sign bit differs from the carry bit after an arithmetic
	// operation.
	// A=0x00 - 0xFF becomes 0x01
	// positive - negative = positive
	SECTION("Immediate (SUBA) (SUBA1)") {
		board.poke(0, 0x80);
		board.poke(1, 0xff);
		cpu.CC() = EightBit::mc6809::CF | EightBit::mc6809::NF | EightBit::mc6809::VF;
		cpu.A() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 1);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cycles == 2);
    }

	// A=0x00 - 0x01 becomes 0xFF
	// positive - positive = negative
	SECTION("Immediate (SUBA) (SUBA2)") {
		board.poke(0, 0x80);
		board.poke(1, 1);
		cpu.CC() = EightBit::mc6809::CF | EightBit::mc6809::NF | EightBit::mc6809::VF;
		cpu.A() = 0;
		cycles = cpu.step();
		REQUIRE(cpu.A() == 0xff);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 2);
    }

	// Test the subtraction instruction.
	// IMMEDIATE mode:   B=0x02 - 0xB3  becomes 0x4F
	// positive - negative = positive
	SECTION("Immediate (SUBB) (SUBB1)") {
		board.poke(0, 0xc0);
		board.poke(1, 0xb3);
		cpu.CC() = 0;
		cpu.B() = 2;
		cycles = cpu.step();
		REQUIRE(cpu.B() == 0x4f);
		REQUIRE(cpu.negative() == 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 2);
    }

	// Test the subtraction instruction.
	// IMMEDIATE mode:   B=0x02 - 0x81  becomes 0x81
	// positive - negative = negative + overflow
	SECTION("Immediate (SUBB) (SUBB2)") {
		board.poke(0, 0xc0);
		board.poke(1, 0x81);
		cpu.CC() = 0;
		cpu.B() = 2;
		cycles = cpu.step();
		REQUIRE(cpu.B() == 0x81);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() != 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 2);
    }

	// Example from Programming the 6809.
	// 0x03 - 0x21 = 0xE2
	// positive - positive = negative
	SECTION("Immediate (SUBB) (SUBBY)") {
		board.poke(0, 0xe0);
		board.poke(1, 0xa4);
		board.poke(0x21, 0x21);
		cpu.CC() = EightBit::mc6809::ZF;
		cpu.B() = 3;
		cpu.Y() = 0x21;
		cycles = cpu.step();
		REQUIRE(cpu.B() == 0xe2);
		REQUIRE(cpu.negative() != 0);
		REQUIRE(cpu.zero() == 0);
		REQUIRE(cpu.overflow() == 0);
		REQUIRE(cpu.carry() != 0);
		REQUIRE(cycles == 4);
    }
}

TEST_CASE(" Branch if Greater Than Zero", "[BGT]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

	board.poke(0, 0x2e);	// BGT
	board.poke(1, 0x03);
	board.poke(2, 0x86);	// LDA	#1
	board.poke(3, 0x01);
	board.poke(4, 0x12);	// NOP
	board.poke(5, 0x86);	// LDA	#2
	board.poke(6, 0x02);
	board.poke(7, 0x12);	// NOP

	SECTION("BGT1") {
		cpu.A() = 0;
 		cpu.CC() = EightBit::mc6809::ZF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 1);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("BGT2") {
		REQUIRE(cpu.PC() == 0);
 		cpu.CC() = 0;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 2);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("BGT3") {
		REQUIRE(cpu.PC() == 0);
 		cpu.CC() = EightBit::mc6809::NF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 1);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("BGT4") {
		REQUIRE(cpu.PC() == 0);
 		cpu.CC() = EightBit::mc6809::NF | EightBit::mc6809::VF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 2);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("BGT5") {
		REQUIRE(cpu.PC() == 0);
 		cpu.CC() = EightBit::mc6809::ZF | EightBit::mc6809::NF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 1);
	}
}

TEST_CASE(" Branch if Higher", "[BHI]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

	board.poke(0, 0x22);	// BHI
	board.poke(1, 0x03);
	board.poke(2, 0x86);	// LDA	#1
	board.poke(3, 0x01);
	board.poke(4, 0x12);	// NOP
	board.poke(5, 0x86);	// LDA	#2
	board.poke(6, 0x02);
	board.poke(7, 0x12);	// NOP

	SECTION("BHI1") {
		cpu.A() = 0;
 		cpu.CC() = EightBit::mc6809::ZF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 1);
	}
}

TEST_CASE("Branch on Less than or Equal to Zero", "[BLE]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

	board.poke(0, 0x2f);	// BLE
	board.poke(1, 0x03);
	board.poke(2, 0x86);	// LDA	#1
	board.poke(3, 0x01);
	board.poke(4, 0x12);	// NOP
	board.poke(5, 0x86);	// LDA	#2
	board.poke(6, 0x02);
	board.poke(7, 0x12);	// NOP

	SECTION("BLE1") {
		cpu.A() = 0;
		cpu.CC() = EightBit::mc6809::ZF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 2);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("BLE2") {
		cpu.A() = 0;
		cpu.CC() = 0;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 1);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("BLE3") {
		cpu.A() = 0;
		cpu.CC() = EightBit::mc6809::NF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 2);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("BLE4") {
		cpu.A() = 0;
		cpu.CC() = EightBit::mc6809::NF | EightBit::mc6809::VF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 1);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("BLE5") {
		cpu.A() = 0;
		cpu.CC() = EightBit::mc6809::ZF | EightBit::mc6809::NF;
		cycles = cpu.step();
		cycles = cpu.step();
		REQUIRE(cpu.A() == 2);
	}
}

TEST_CASE("Subroutines", "[JSR][RTS]") {

	Board board;
	board.raisePOWER();
	auto& cpu = board.CPU();
	auto cycles = cpu.step();	// Step over the reset

	// Test the JSR - Jump to Subroutine - instruction.
	// INDEXED mode:   JSR   D,Y
	SECTION("JSR") {

		// Set up a word to test at address 0x205
		cpu.pokeWord(0x205, 0x03ff);

		// Set register D
		cpu.D() = 0x105;

		// Set register Y to point to that location minus 5
		cpu.Y() = 0x200;

		// Set register S to point to 0x915
		cpu.S() = 0x915;

		// Two bytes of instruction
		board.poke(0xB00, 0xAD);
		board.poke(0xB01, 0xAB);
		board.poke(0xB02, 0x11); // Junk
		board.poke(0xB03, 0x22); // Junk

		cpu.PC() = 0xB00;
		cpu.CC() = 0;

		cycles = cpu.step();

		REQUIRE(cpu.CC() == 0);
		REQUIRE(cpu.A() == 1);
		REQUIRE(cpu.B() == 5);
		REQUIRE(cpu.DP() == 0);
		REQUIRE(cpu.X() == 0);
		REQUIRE(cpu.Y() == 0x200);
		REQUIRE(cpu.D() == 0x105);
		REQUIRE(cpu.S() == 0x913);
		REQUIRE(cpu.U() == 0);
		REQUIRE(cpu.PC() == 0x305);

		REQUIRE(board.peek(0x914) == 2);
		REQUIRE(board.peek(0x913) == 0xb);

		REQUIRE(cycles == 11);
	}

	cpu.lowerRESET();
	cycles = cpu.step();

	SECTION("RTS") {

		cpu.S().word = 0x300;

		cpu.pokeWord(0x300, 0x102C); // Write return address
		board.poke(0xB00, 0x39); // RTS

		cpu.PC().word = 0xB00;

		cycles = cpu.step();

		REQUIRE(cpu.PC().word == 0x102C);
		REQUIRE(cpu.S().word == 0x302);
		REQUIRE(cycles == 5);
	}
}
