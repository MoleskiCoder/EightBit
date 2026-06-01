#include "pch.h"
#include <Chip.h>

BOOST_AUTO_TEST_SUITE(Chip)

BOOST_AUTO_TEST_CASE(bit_returns_correct_bit_mask) {
    BOOST_CHECK_EQUAL(EightBit::Chip::bit(0), 0x01);
    BOOST_CHECK_EQUAL(EightBit::Chip::bit(1), 0x02);
    BOOST_CHECK_EQUAL(EightBit::Chip::bit(7), 0x80);
    BOOST_CHECK_EQUAL(EightBit::Chip::bit(3), 0x08);
}

BOOST_AUTO_TEST_CASE(setBit_sets_bits_correctly) {
    BOOST_CHECK_EQUAL(EightBit::Chip::setBit((uint8_t)0b00001001, 0b00000100),        0b00001101);
    BOOST_CHECK_EQUAL(EightBit::Chip::setBit((uint8_t)0b00001101, 0b00000100),        0b00001101);
    BOOST_CHECK_EQUAL(EightBit::Chip::setBit((uint8_t)0b00001101, 0b00000100, true),  0b00001101);
    BOOST_CHECK_EQUAL(EightBit::Chip::setBit((uint8_t)0b00001101, 0b00000100, false), 0b00001001);
}

BOOST_AUTO_TEST_CASE(clearBit_clears_bits_correctly) {
    BOOST_CHECK_EQUAL(EightBit::Chip::clearBit((uint8_t)0b00001101, 0b00000100),        0b00001001);
    BOOST_CHECK_EQUAL(EightBit::Chip::clearBit((uint8_t)0b00001101, 0b00000100, false), 0b00001101);
    BOOST_CHECK_EQUAL(EightBit::Chip::clearBit((uint8_t)0b00001101, 0b00000100, true),  0b00001001);
}

BOOST_AUTO_TEST_CASE(highByte_and_lowByte_work_correctly) {
    const uint16_t u16 = 0xABCD;
    BOOST_CHECK_EQUAL(EightBit::Chip::highByte(u16), 0xAB);
    BOOST_CHECK_EQUAL(EightBit::Chip::lowByte(u16),  0xCD);

    const int i32 = 0x1234;
    BOOST_CHECK_EQUAL(EightBit::Chip::highByte(i32), 0x12);
    BOOST_CHECK_EQUAL(EightBit::Chip::lowByte(i32),  0x34);
}

BOOST_AUTO_TEST_CASE(promoteByte_and_demoteByte_work_correctly) {
    BOOST_CHECK_EQUAL(EightBit::Chip::promoteByte((uint8_t)0x34),   0x3400);
    BOOST_CHECK_EQUAL(EightBit::Chip::demoteByte((uint16_t)0x1234), 0x12);
}

BOOST_AUTO_TEST_CASE(higherPart_and_lowerPart_work_correctly) {
    const uint16_t value = 0xABCD;
    BOOST_CHECK_EQUAL(EightBit::Chip::higherPart(value), 0xAB00);
    BOOST_CHECK_EQUAL(EightBit::Chip::lowerPart(value),  0xCD);
}

BOOST_AUTO_TEST_CASE(makeWord_creates_correct_word) {
    BOOST_CHECK_EQUAL(EightBit::Chip::makeWord((uint8_t)0x34, (uint8_t)0x12), 0x1234);
}

BOOST_AUTO_TEST_CASE(nibble_methods_work_correctly) {
    const uint8_t value = 0xAB;
    BOOST_CHECK_EQUAL(EightBit::Chip::highNibble(value),    0x0A);
    BOOST_CHECK_EQUAL(EightBit::Chip::lowNibble(value),     0x0B);
    BOOST_CHECK_EQUAL(EightBit::Chip::higherNibble(value),  0xA0);
    BOOST_CHECK_EQUAL(EightBit::Chip::lowerNibble(value),   0x0B);
    BOOST_CHECK_EQUAL(EightBit::Chip::promoteNibble(value), 0xB0);
    BOOST_CHECK_EQUAL(EightBit::Chip::demoteNibble(value),  0x0A);
}

BOOST_AUTO_TEST_CASE(countBits_returns_correct_count) {
    BOOST_CHECK_EQUAL(EightBit::Chip::countBits(0U),    0);
    BOOST_CHECK_EQUAL(EightBit::Chip::countBits(1U),    1);
    BOOST_CHECK_EQUAL(EightBit::Chip::countBits(0xFFU), 8);
}

BOOST_AUTO_TEST_CASE(evenParity_returns_correct_parity) {
    BOOST_CHECK( EightBit::Chip::evenParity(0U));  // 0 bits set — even
    BOOST_CHECK(!EightBit::Chip::evenParity(1U));  // 1 bit set  — odd
    BOOST_CHECK( EightBit::Chip::evenParity(3U));  // 2 bits set — even
}

BOOST_AUTO_TEST_CASE(findFirstSet_returns_correct_index) {
    BOOST_CHECK_EQUAL(EightBit::Chip::findFirstSet(0UL),        0);
    BOOST_CHECK_EQUAL(EightBit::Chip::findFirstSet(1UL),        1);
    BOOST_CHECK_EQUAL(EightBit::Chip::findFirstSet(2UL),        2);
    BOOST_CHECK_EQUAL(EightBit::Chip::findFirstSet(4UL),        3);
    BOOST_CHECK_EQUAL(EightBit::Chip::findFirstSet(0b10000UL),  5);
}

BOOST_AUTO_TEST_SUITE_END()
