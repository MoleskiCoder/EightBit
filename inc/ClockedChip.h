#pragma once

#include "Chip.h"
#include "EventArgs.h"
#include "Signal.h"

namespace EightBit {
	class ClockedChip : public Chip {
	public:
		ClockedChip(const ClockedChip& rhs) noexcept;
		bool operator==(const ClockedChip& rhs) const noexcept;

		Signal<EventArgs> Ticked;

		[[nodiscard]] constexpr auto cycles() const noexcept { return m_cycles; }

		void tick(int extra = 1) noexcept {
			for (int i = 0; i < extra; ++i) {
				++m_cycles;
				Ticked.fire();
			}
		}

	protected:
		ClockedChip() noexcept = default;

		constexpr void resetCycles() noexcept { m_cycles = 0; }

	private:
		int m_cycles = 0;
	};
}
