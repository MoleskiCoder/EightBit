#pragma once

#include <vector>
#include <cstdint>

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

			AbstractColourPalette() noexcept
			: m_colours(4) {
			}

			uint32_t getColour(size_t index) const {
				return m_colours[index];
			}

		protected:
			std::vector<uint32_t> m_colours;
		};
	}
}