#pragma once

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

		static void match(PinLevel& line, int value);

		virtual ~Device() {};

		[[nodiscard]] auto& POWER() noexcept { return m_powerLine; }

		[[nodiscard]] auto powered() noexcept { return raised(POWER()); }
		virtual void powerOn();
		virtual void powerOff() { lower(POWER()); }

	protected:
		Device() {};

	private:
		PinLevel m_powerLine = PinLevel::Low;
	};
}
