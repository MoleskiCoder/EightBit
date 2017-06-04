#pragma once

namespace EightBit {
	class EventArgs {
	private:
		static EventArgs m_empty;

	public:
		static EventArgs& empty() { return m_empty; }

		EventArgs() {}
	};
}