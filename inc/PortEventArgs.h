#pragma once

#include "EventArgs.h"
#include "Register.h"

namespace EightBit {
	class PortEventArgs final : public EventArgs {
	private:
		register16_t _port;

	public:
		PortEventArgs(register16_t port) noexcept
		: _port(port) {}

		[[nodiscard]] constexpr auto port() const noexcept { return _port; }
	};
}
