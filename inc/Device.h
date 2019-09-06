#pragma once

#include "EventArgs.h"
#include "Signal.h"

#define DECLARE_PIN_SIGNALS(name) \
	Signal<EventArgs> Raising ## name; \
	Signal<EventArgs> Raised ## name; \
	Signal<EventArgs> Lowering ## name; \
	Signal<EventArgs> Lowered ## name;

#define DECLARE_PIN_LEVEL_RAISE(name) \
	virtual void raise ## name();

#define DECLARE_PIN_LEVEL_LOWER(name) \
	virtual void lower ## name();

#define DECLARE_PIN_LEVEL_CHANGERS(name) \
	DECLARE_PIN_LEVEL_RAISE(name) \
	DECLARE_PIN_LEVEL_LOWER(name)

#define DEFINE_PIN_LEVEL_RAISE(name, within) \
	void EightBit:: within ::raise ## name() { \
		if (lowered( name ())) { \
			Raising ## name.fire(EventArgs::empty()); \
			raise( name ()); \
			Raised ## name.fire(EventArgs::empty()); \
		} \
	}

#define DEFINE_PIN_LEVEL_LOWER(name, within) \
	void EightBit:: within ::lower ## name() { \
		if (raised( name ())) { \
			Lowering ## name.fire(EventArgs::empty()); \
			lower( name ()); \
			Lowered ## name.fire(EventArgs::empty()); \
		} \
	}

#define DEFINE_PIN_LEVEL_CHANGERS(name, within) \
	DEFINE_PIN_LEVEL_RAISE(name, within) \
	DEFINE_PIN_LEVEL_LOWER(name, within)

#define DECLARE_PIN_MEMBER(name) \
	PinLevel m_## name ## _Line = PinLevel::Low;

#define DECLARE_PIN(name, visibility) \
	public: DECLARE_PIN_SIGNALS(name) \
	[[nodiscard]] PinLevel& name () noexcept { return m_## name ## _Line; } \
	visibility : DECLARE_PIN_LEVEL_CHANGERS(name) \
	private: DECLARE_PIN_MEMBER(name)

// Input pins have a degree of external control
#define DECLARE_PIN_INPUT(name) DECLARE_PIN(name, public)

// Output pins may only be internally controlled
#define DECLARE_PIN_OUTPUT(name) DECLARE_PIN(name, protected)

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

		static void match(PinLevel& line, int condition) {
			match(line, condition != 0);
		}

		static void match(PinLevel& line, bool condition) {
			condition ? raise(line) : lower(line);
		}

		virtual ~Device() {};

		[[nodiscard]] bool powered() noexcept { return raised(POWER()); }

		DECLARE_PIN_INPUT(POWER)

	protected:
		Device() {};
	};
}
