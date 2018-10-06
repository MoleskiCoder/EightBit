#include "pch.h"
#include "Board.h"

// Using examples from 6809 Assembly Language Programming, by Lance A. Leventhal

TEST_CASE("Add Accumulator B to Index Register X Unsigned ", "[ABX]") {

	Board board;
	board.initialise();
	auto& cpu = board.CPU();
	cpu.step();	// Jump over the reset

    SECTION("Inherent") {
		board.poke(0, 0x3a);
		cpu.B() = 0x84;
		cpu.X() = 0x1097;
		cpu.step();
		REQUIRE(cpu.X() == 0x111b);
    }
}


//TEST_CASE( "vectors can be sized and resized", "[vector]" ) {
//
//    std::vector<int> v( 5 );
//    
//    REQUIRE( v.size() == 5 );
//    REQUIRE( v.capacity() >= 5 );
//    
//    SECTION( "resizing bigger changes size and capacity" ) {
//        v.resize( 10 );
//        
//        REQUIRE( v.size() == 10 );
//        REQUIRE( v.capacity() >= 10 );
//    }
//    SECTION( "resizing smaller changes size but not capacity" ) {
//        v.resize( 0 );
//        
//        REQUIRE( v.size() == 0 );
//        REQUIRE( v.capacity() >= 5 );
//    }
//    SECTION( "reserving bigger changes capacity but not size" ) {
//        v.reserve( 10 );
//        
//        REQUIRE( v.size() == 5 );
//        REQUIRE( v.capacity() >= 10 );
//    }
//    SECTION( "reserving smaller does not change size or capacity" ) {
//        v.reserve( 0 );
//        
//        REQUIRE( v.size() == 5 );
//        REQUIRE( v.capacity() >= 5 );
//    }
//}
