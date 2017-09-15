#include "stdafx.h"
#include "Display.h"
#include "Processor.h"
#include "CharacterDefinition.h"
#include "ObjectAttribute.h"

#include <vector>

EightBit::GameBoy::Display::Display(const AbstractColourPalette* colours, Bus& bus)
: m_bus(bus),
  m_colours(colours) {
}

const std::vector<uint32_t>& EightBit::GameBoy::Display::pixels() const {
	return m_pixels;
}

void EightBit::GameBoy::Display::initialise() {
	m_pixels.resize(RasterWidth * RasterHeight);
}

void EightBit::GameBoy::Display::render() {

	auto control = m_bus.peekRegister(Bus::LCDC);
	auto on = control & Bus::LcdEnable;
	if (on) {

		const auto bgDisplay = (control & Bus::DisplayBackground) != 0;
		if (bgDisplay)
			renderBackground();

		const auto objEnable = (control & Bus::ObjectEnable) != 0;
		if (objEnable)
			renderObjects();
	}
}

std::array<int, 4> EightBit::GameBoy::Display::createPalette(const int address) {
	const auto raw = m_bus.peekRegister(address);
	std::array<int, 4> palette;
	palette[0] = raw & 0b11;
	palette[1] = (raw & 0b1100) >> 2;
	palette[2] = (raw & 0b110000) >> 4;
	palette[3] = (raw & 0b11000000) >> 6;
	return palette;
}

void EightBit::GameBoy::Display::renderObjects() {
	const auto control = m_bus.peekRegister(Bus::LCDC);
	const auto objBlockHeight = (control & Bus::ObjectBlockCompositionSelection) ? 16 : 8;
	renderObjects(objBlockHeight);
}

void EightBit::GameBoy::Display::renderObjects(int objBlockHeight) {
	
	std::vector<std::array<int, 4>> palettes(2);
	palettes[0] = createPalette(Bus::OBP0);
	palettes[1] = createPalette(Bus::OBP1);

	auto objDefinitionAddress = 0x8000;
	auto oamAddress = 0xfe00;

	for (int i = 0; i < 40; ++i) {

		const auto current = ObjectAttribute(m_bus, oamAddress + 4 * i, objBlockHeight);
		const auto sprite = current.pattern();
		const auto definition = CharacterDefinition(m_bus, objDefinitionAddress + 16 * sprite);
		const auto spriteX = current.positionX();
		const auto spriteY = current.positionY();
		const auto& palette = palettes[current.palette()];
		const auto flipX = current.flipX();
		const auto flipY = current.flipY();

		renderTile(
			spriteX, spriteY, -8, -16,
			flipX, flipY, true,
			palette,
			definition);
	}
}

void EightBit::GameBoy::Display::renderBackground() {

	const auto control = m_bus.peekRegister(Bus::LCDC);

	auto palette = createPalette(Bus::BGP);

	const auto window = (control & Bus::WindowEnable) != 0;
	const auto bgArea = (control & Bus::BackgroundCodeAreaSelection) ? 0x9c00 : 0x9800;
	const auto bgCharacters = (control & Bus::BackgroundCharacterDataSelection) ? 0x8000 : 0x8800;

	const auto wx = m_bus.peekRegister(Bus::WX);
	const auto wy = m_bus.peekRegister(Bus::WY);

	const auto offsetX = window ? wx - 7 : 0;
	const auto offsetY = window ? wy : 0;

	const auto scrollX = m_bus.peekRegister(Bus::SCX);
	const auto scrollY = m_bus.peekRegister(Bus::SCY);

	renderBackground(bgArea, bgCharacters, offsetX, offsetY, scrollX, scrollY, palette);
}

void EightBit::GameBoy::Display::renderBackground(
		int bgArea, int bgCharacters,
		int offsetX, int offsetY,
		int scrollX, int scrollY,
		const std::array<int, 4>& palette) {

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

			renderTile(
				column * 8, row * 8, offsetX - scrollX, offsetY - scrollY,
				false, false, false,
				palette,
				definition);
		}
	}
}

void EightBit::GameBoy::Display::renderTile(
		int drawX, int drawY, int offsetX, int offsetY,
		bool flipX, bool flipY, bool allowTransparencies,
		const std::array<int, 4>& palette,
		const CharacterDefinition& definition) {

	for (int cy = 0; cy < 8; ++cy) {

		for (int cx = 0; cx < 8; ++cx) {

			uint8_t y = drawY + cy + offsetY;
			if (y >= RasterHeight)
				break;

			uint8_t x = drawX + cx + offsetX;
			if (x >= RasterWidth)
				break;

			auto outputPixel = y * RasterWidth + x;

			auto colour = palette[definition.get()[cy * 8 + cx]];
			m_pixels[outputPixel] = m_colours->getColour(colour);
		}
	}
}
