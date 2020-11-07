#pragma once

#include <array>
#include <cstdint>
#include <cassert>

namespace EightBit {
	namespace GameBoy {
		class AbstractColourPalette {
		public:
			enum {
				Off,
				Light,
				Medium,
				Dark
			};

			AbstractColourPalette() = default;

			[[nodiscard]] auto colour(size_t index) const noexcept {
				assert(index < m_colours.size());
				return m_colours[index];
			}

		protected:
			std::array<uint32_t, 4> m_colours;
		};
	}
}