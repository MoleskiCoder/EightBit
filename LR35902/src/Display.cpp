#include "stdafx.h"
#include "Display.h"
#include "Processor.h"
#include "CharacterDefinition.h"

EightBit::Display::Display(const AbstractColourPalette* colours, Bus& bus)
: m_bus(bus),
  m_colours(colours) {
}

const std::vector<uint32_t>& EightBit::Display::pixels() const {
	return m_pixels;
}

void EightBit::Display::initialise() {
	m_pixels.resize(RasterWidth * RasterHeight);
}

void EightBit::Display::render() {

	auto control = m_bus.peekRegister(Bus::LCDC);
	auto on = control & Bus::LcdEnable;
	if (on) {

		auto windowArea = (control & Bus::WindowCodeAreaSelection) ? 0x9c00 : 0x9800;
		auto window = (control & Bus::WindowEnable) != 0;
		auto bgCharacters = (control & Bus::BackgroundCharacterDataSelection) ? 0x8000 : 0x8800;
		auto bgArea = (control & Bus::BackgroundCodeAreaSelection) ? 0x9c00 : 0x9800;
		auto objBlockHeight = (control & Bus::ObjectBlockCompositionSelection) ? 16 : 8;
		auto objEnable = (control & Bus::ObjectEnable) != 0;
		auto bgDisplay = (control & Bus::DisplayBackground) != 0;

		auto scrollX = m_bus.peekRegister(Bus::SCX);
		auto scrollY = m_bus.peekRegister(Bus::SCY);

		auto paletteRaw = m_bus.peekRegister(Bus::BGP);
		std::array<int, 4> palette;
		palette[0] = paletteRaw & 0b11;
		palette[1] = (paletteRaw & 0b1100) >> 2;
		palette[2] = (paletteRaw & 0b110000) >> 4;
		palette[3] = (paletteRaw & 0b11000000) >> 6;

		auto wx = m_bus.peekRegister(Bus::WX);
		auto wy = m_bus.peekRegister(Bus::WY);

		auto offsetX = window ? wx - 7 : 0;
		auto offsetY = window ? wy : 0;

		std::map<int, CharacterDefinition> definitions;

		for (int row = 0; row < BufferCharacterHeight; ++row) {
			for (int column = 0; column < BufferCharacterWidth; ++column) {

				auto address = bgArea + row * BufferCharacterWidth + column;
				auto character = m_bus.peek(address);

				auto definitionPair = definitions.find(character);

				if (definitionPair == definitions.end()) {
					definitions[character] = CharacterDefinition(m_bus, bgCharacters + 16 * character);
					definitionPair = definitions.find(character);
				}

				auto definition = definitionPair->second;

				for (int cy = 0; cy < 8; ++cy) {
					for (int cx = 0; cx < 8; ++cx) {

						uint8_t x = column * 8 + cx + offsetX - scrollX;
						if (x >= RasterWidth)
							break;

						uint8_t y = row * 8 + cy + offsetY - scrollY;
						if (y >= RasterHeight)
							break;

						auto outputPixel = y * RasterWidth + x;

						auto colour = palette[definition.get()[cy * 8 + cx]];
						m_pixels[outputPixel] = m_colours->getColour(colour);
					}
				}
			}
		}
	}
}
