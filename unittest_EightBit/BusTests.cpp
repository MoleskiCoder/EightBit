#include "pch.h"
#include <Bus.h>
#include <Ram.h>
#include <Chip.h>

// Minimal Bus whose entire address space maps to a single read-only RAM region.
// seed()/rawPeek() reach the RAM directly, bypassing the access-level check,
// so they can be used to set up and verify the underlying memory independently
// of whatever the Bus write path does.
namespace {
    class ReadOnlyBus : public EightBit::Bus {
        EightBit::Ram m_ram{ 0x10000 };
        EightBit::MemoryMapping m_mapping;
    public:
        ReadOnlyBus()
            : m_mapping{ m_ram, 0x0000, EightBit::Chip::Mask16,
                         EightBit::MemoryMapping::AccessLevel::ReadOnly } {}

        /// Writes directly to the backing RAM, bypassing the access-level guard.
        void seed(uint16_t address, uint8_t value) { m_ram.poke(address, value); }

        /// Reads directly from the backing RAM, bypassing the access-level guard.
        uint8_t rawPeek(uint16_t address) const { return m_ram.peek(address); }

        const EightBit::MemoryMapping& mapping(uint16_t) noexcept final { return m_mapping; }
        void initialise() noexcept final {}
    };
}

BOOST_AUTO_TEST_SUITE(Bus)

BOOST_AUTO_TEST_CASE(read_from_ReadOnly_returns_memory_value) {
    ReadOnlyBus bus;
    bus.seed(0x1000, 0xAB);
    bus.ADDRESS() = 0x1000;
    bus.read();
    BOOST_CHECK_EQUAL(bus.DATA(), 0xAB);
}

BOOST_AUTO_TEST_CASE(write_to_ReadOnly_does_not_modify_memory) {
    ReadOnlyBus bus;
    bus.seed(0x2000, 0x55);
    bus.ADDRESS() = 0x2000;
    bus.DATA() = 0xFF;
    bus.write();
    // Memory must be unchanged.
    BOOST_CHECK_EQUAL(bus.rawPeek(0x2000), 0x55);
    // Write to read-only mapping must leave the written value on the bus, not the memory value
    BOOST_CHECK_EQUAL(bus.DATA(), 0xff);
}

BOOST_AUTO_TEST_CASE(read_after_write_to_ReadOnly_returns_original_value) {
    ReadOnlyBus bus;
    bus.seed(0x3000, 0x42);
    bus.ADDRESS() = 0x3000;
    bus.DATA() = 0xFF;
    bus.write();        // silently discarded — memory unchanged
    bus.DATA() = 0x00;  // clear DATA to prove the subsequent read loads from memory
    bus.read();
    BOOST_CHECK_EQUAL(bus.DATA(), 0x42);
}

BOOST_AUTO_TEST_SUITE_END()
