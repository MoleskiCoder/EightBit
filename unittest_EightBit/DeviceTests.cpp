#include "pch.h"
#include <Device.h>

// Device has a protected constructor, so we expose it via a minimal subclass.
namespace {
    class TestDevice : public EightBit::Device {
    public:
        TestDevice() = default;
    };
}

BOOST_AUTO_TEST_SUITE(Device)

BOOST_AUTO_TEST_CASE(initial_state_is_not_powered) {
    TestDevice device;
    BOOST_CHECK(!device.powered());
    BOOST_CHECK(device.POWER() == EightBit::Device::PinLevel::Low);
}

BOOST_AUTO_TEST_CASE(raisePOWER_sets_POWER_high_and_fires_events) {
    TestDevice device;
    bool raising = false, raised = false;

    device.RaisingPOWER.connect([&raising](EightBit::EventArgs&) { raising = true; });
    device.RaisedPOWER.connect([&raised](EightBit::EventArgs&)   { raised  = true; });

    device.raisePOWER();

    BOOST_CHECK(device.powered());
    BOOST_CHECK(device.POWER() == EightBit::Device::PinLevel::High);
    BOOST_CHECK(raising);
    BOOST_CHECK(raised);
}

BOOST_AUTO_TEST_CASE(lowerPOWER_sets_POWER_low_and_fires_events) {
    TestDevice device;
    device.raisePOWER();  // start high

    bool lowering = false, lowered = false;
    device.LoweringPOWER.connect([&lowering](EightBit::EventArgs&) { lowering = true; });
    device.LoweredPOWER.connect([&lowered](EightBit::EventArgs&)   { lowered  = true; });

    device.lowerPOWER();

    BOOST_CHECK(!device.powered());
    BOOST_CHECK(device.POWER() == EightBit::Device::PinLevel::Low);
    BOOST_CHECK(lowering);
    BOOST_CHECK(lowered);
}

BOOST_AUTO_TEST_CASE(raisePOWER_does_nothing_if_already_high) {
    TestDevice device;
    device.raisePOWER();

    bool raising = false, raised = false;
    device.RaisingPOWER.connect([&raising](EightBit::EventArgs&) { raising = true; });
    device.RaisedPOWER.connect([&raised](EightBit::EventArgs&)   { raised  = true; });

    device.raisePOWER();  // already high — should be a no-op

    BOOST_CHECK(device.powered());
    BOOST_CHECK(!raising);
    BOOST_CHECK(!raised);
}

BOOST_AUTO_TEST_CASE(lowerPOWER_does_nothing_if_already_low) {
    TestDevice device;  // starts low

    bool lowering = false, lowered = false;
    device.LoweringPOWER.connect([&lowering](EightBit::EventArgs&) { lowering = true; });
    device.LoweredPOWER.connect([&lowered](EightBit::EventArgs&)   { lowered  = true; });

    device.lowerPOWER();  // already low — should be a no-op

    BOOST_CHECK(!device.powered());
    BOOST_CHECK(!lowering);
    BOOST_CHECK(!lowered);
}

BOOST_AUTO_TEST_SUITE_END()
