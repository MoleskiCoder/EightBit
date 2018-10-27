#pragma once

namespace EightBit {
	class EventArgs {
	private:
		static EventArgs m_empty;

	public:
		static auto& empty() { return m_empty; }
	};
}
