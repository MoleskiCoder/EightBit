#pragma once

#include "Chip.h"
#include "EventArgs.h"
#include "Signal.h"

namespace EightBit {
	class ClockedChip : public Chip {
	public:
		virtual ~ClockedChip() noexcept {};

		bool operator==(const ClockedChip& rhs) const;

		Signal<EventArgs> Ticked;

		[[nodiscard]] constexpr auto cycles() const noexcept { return m_cycles; }

		void tick(int extra);
		void tick();

	protected:
		ClockedChip() noexcept = default;

		void resetCycles() noexcept;

	private:
		int m_cycles = 0;
	};
}
