#pragma once

#include "EventArgs.h"
#include "Signal.h"

namespace EightBit {
	class Device {
	public:
		enum class PinLevel {
			Low, High
		};

		static constexpr auto raised(const PinLevel line) { return line == PinLevel::High; }
		static void raise(PinLevel& line) noexcept { line = PinLevel::High; }
		static constexpr auto lowered(const PinLevel line) { return line == PinLevel::Low; }
		static void lower(PinLevel& line) noexcept { line = PinLevel::Low; }

		virtual ~Device() {};

		Signal<EventArgs> RaisedPOWER;
		Signal<EventArgs> LoweredPOWER;

		[[nodiscard]] auto& POWER() noexcept { return m_powerLine; }

		[[nodiscard]] auto powered() noexcept { return raised(POWER()); }
		virtual void raisePOWER();
		virtual void lowerPOWER();

	protected:
		Device() {};

	private:
		PinLevel m_powerLine = PinLevel::Low;	// In
	};
}
