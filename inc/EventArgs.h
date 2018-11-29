#pragma once

namespace EightBit {
	class EventArgs {
	private:
		static EventArgs m_empty;

	public:
		[[nodiscard]] static auto& empty() noexcept { return m_empty; }
	};
}
