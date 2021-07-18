#pragma once

namespace EightBit {
	class EventArgs {
	private:
		static EventArgs m_empty;

	public:
		[[nodiscard]] static constexpr auto& empty() noexcept { return m_empty; }
	};
}
