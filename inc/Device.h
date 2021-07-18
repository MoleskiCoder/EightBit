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
			Raising ## name.fire(); \
			raise( name ()); \
			Raised ## name.fire(); \
		} \
	}

#define DEFINE_PIN_LEVEL_LOWER(name, within) \
	void EightBit:: within ::lower ## name() { \
		if (raised( name ())) { \
			Lowering ## name.fire(); \
			lower( name ()); \
			Lowered ## name.fire(); \
		} \
	}

#define DEFINE_PIN_LEVEL_CHANGERS(name, within) \
	DEFINE_PIN_LEVEL_RAISE(name, within) \
	DEFINE_PIN_LEVEL_LOWER(name, within)

#define DECLARE_PIN_MEMBER(name) \
	PinLevel m_## name ## _Line = PinLevel::Low;

#define DEFINE_PIN_ACTIVATOR(name, on, off) \
	template <class T> class _Activate ## name final { \
		T& m_parent; \
	public: \
		_Activate ## name(T& parent) noexcept \
		: m_parent(parent) { \
			m_parent. on ## name(); \
		 } \
		~_Activate ## name() noexcept { \
			m_parent. off ## name(); \
		} \
	};

#define DEFINE_PIN_ACTIVATOR_LOW(name) \
	DEFINE_PIN_ACTIVATOR(name, lower, raise)

#define DEFINE_PIN_ACTIVATOR_HIGH(name) \
	DEFINE_PIN_ACTIVATOR(name, raise, lower)

#define DECLARE_PIN(name, visibility) \
	public: \
		DECLARE_PIN_SIGNALS(name) \
		[[nodiscard]] constexpr auto& name () noexcept { \
			return m_## name ## _Line; \
		} \
		[[nodiscard]] constexpr const auto& name () const noexcept { \
			return m_## name ## _Line; \
		} \
	visibility : \
		DECLARE_PIN_LEVEL_CHANGERS(name) \
	private: \
		DECLARE_PIN_MEMBER(name)

// Input pins may be external controlled
#define DECLARE_PIN_INPUT(name) \
	DECLARE_PIN(name, public)

// Output pins can only be internally controlled
#define DECLARE_PIN_OUTPUT(name) \
	DECLARE_PIN(name, protected)

namespace EightBit {
	class Device {
	public:
		enum class PinLevel {
			Low, High
		};

		DECLARE_PIN_INPUT(POWER)

	public:
		[[nodiscard]] static constexpr auto raised(const PinLevel line) noexcept { return line == PinLevel::High; }
		static constexpr void raise(PinLevel& line) noexcept { line = PinLevel::High; }
		[[nodiscard]] static constexpr auto lowered(const PinLevel line) noexcept { return line == PinLevel::Low; }
		static constexpr void lower(PinLevel& line) noexcept { line = PinLevel::Low; }

		static constexpr void match(PinLevel& line, int condition) noexcept { match(line, condition != 0); }
		static constexpr void match(PinLevel& line, bool condition) noexcept { condition ? raise(line) : lower(line); }
		static constexpr void match(PinLevel& out, PinLevel in) noexcept { out = in; }

		virtual ~Device() noexcept {};

		[[nodiscard]] constexpr bool powered() const noexcept { return raised(POWER()); }

	protected:
		Device() noexcept {};
	};
}
