#pragma once

#include <vector>
#include <cstdint>

#include "Bus.h"
#include "AbstractColourPalette.h"

namespace EightBit {
	class Display {
	public:
		enum {
			BufferWidth = 256,
			BufferHeight = 256,
			BufferCharacterWidth = BufferWidth / 8,
			BufferCharacterHeight = BufferHeight / 8,
			RasterWidth = 160,
			RasterHeight = 144,
		};

		Display(const AbstractColourPalette* colours, Bus& bus);

		const std::vector<uint32_t>& pixels() const;

		void initialise();
		void render();

	private:
		std::vector<uint32_t> m_pixels;
		Bus& m_bus;
		const AbstractColourPalette* m_colours;
	};
}