#include "stdafx.h"
#include "Display.h"
#include "Processor.h"
#include "CharacterDefinition.h"
#include "ObjectAttribute.h"

#include <vector>

EightBit::GameBoy::Display::Display(const AbstractColourPalette* colours, Bus& bus)
: m_bus(bus),
  m_colours(colours),
  m_control(0),
  m_scanLine(0) {
}

const std::vector<uint32_t>& EightBit::GameBoy::Display::pixels() const {
	return m_pixels;
}

void EightBit::GameBoy::Display::initialise() {
	m_pixels.resize(RasterWidth * RasterHeight);
}

void EightBit::GameBoy::Display::render() {
	m_control = m_bus.peekRegister(Bus::LCDC);
	if (m_control & Bus::LcdEnable) {
		m_scanLine = m_bus.peekRegister(Bus::LY);
		if (m_control & Bus::DisplayBackground)
			renderBackground();
		if (m_control & Bus::ObjectEnable)
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
	const auto objBlockHeight = (m_control & Bus::ObjectBlockCompositionSelection) ? 16 : 8;
	renderObjects(objBlockHeight);
}

void EightBit::GameBoy::Display::loadObjectAttributes() {

	const auto oamAddress = 0xfe00;

	const auto objBlockHeight = (m_control & Bus::ObjectBlockCompositionSelection) ? 16 : 8;

	for (int i = 0; i < 40; ++i) {
		m_objectAttributes[i] = ObjectAttribute(m_bus, oamAddress + 4 * i, objBlockHeight);
	}
}

void EightBit::GameBoy::Display::renderObjects(int objBlockHeight) {
	
	std::vector<std::array<int, 4>> palettes(2);
	palettes[0] = createPalette(Bus::OBP0);
	palettes[1] = createPalette(Bus::OBP1);

	const auto objDefinitionAddress = 0x8000;

	for (int i = 0; i < 40; ++i) {

		const auto& current = m_objectAttributes[i];

		const auto sprite = current.pattern();
		const auto spriteX = current.positionX();
		const auto spriteY = current.positionY();

		const auto hidden = (spriteX == 0) && (spriteY == 0);

		if (!hidden) {

			const auto definition = CharacterDefinition(&m_bus, objDefinitionAddress + 16 * sprite, objBlockHeight);
			const auto& palette = palettes[current.palette()];
			const auto flipX = current.flipX();
			const auto flipY = current.flipY();

			renderTile(
				objBlockHeight,
				spriteX - 8, spriteY - 16,
				flipX, flipY, true,
				palette,
				definition);
		}
	}
}

void EightBit::GameBoy::Display::renderBackground() {

	auto palette = createPalette(Bus::BGP);

	const auto window = (m_control & Bus::WindowEnable) != 0;
	const auto bgArea = (m_control & Bus::BackgroundCodeAreaSelection) ? 0x9c00 : 0x9800;
	const auto bgCharacters = (m_control & Bus::BackgroundCharacterDataSelection) ? 0x8000 : 0x8800;

	const auto wx = m_bus.peekRegister(Bus::WX);
	const auto wy = m_bus.peekRegister(Bus::WY);

	const auto offsetX = window ? wx - 7 : 0;
	const auto offsetY = window ? wy : 0;

	const auto scrollX = m_bus.peekRegister(Bus::SCX);
	const auto scrollY = m_bus.peekRegister(Bus::SCY);

	renderBackground(bgArea, bgCharacters, offsetX - scrollX, offsetY - scrollY, palette);
}

void EightBit::GameBoy::Display::renderBackground(
		int bgArea, int bgCharacters,
		int offsetX, int offsetY,
		const std::array<int, 4>& palette) {

	const int row = (m_scanLine - offsetY) / 8;

	for (int column = 0; column < BufferCharacterWidth; ++column) {

		const auto address = bgArea + row * BufferCharacterWidth + column;
		const auto character = m_bus.peek(address);

		const auto definition = CharacterDefinition(&m_bus, bgCharacters + 16 * character, 8);
		renderTile(
			8,
			column * 8 + offsetX, row * 8 + offsetY,
			false, false, false,
			palette,
			definition);
	}
}

void EightBit::GameBoy::Display::renderTile(
		int height,
		int drawX, int drawY,
		bool flipX, bool flipY, bool allowTransparencies,
		const std::array<int, 4>& palette,
		const CharacterDefinition& definition) {

	const auto width = 8;

	const auto flipMaskX = width - 1;
	const auto flipMaskY = height - 1;

	for (int cy = 0; cy < height; ++cy) {

		const uint8_t y = drawY + (flipY ? ~cy & flipMaskY : cy);
		if (y >= RasterHeight)
			continue;

		if (y != m_currentScanLine)
			continue;

		const auto rowDefinition = definition.get(cy);

		for (int cx = 0; cx < width; ++cx) {

			const uint8_t x = drawX + (flipX ? ~cx & flipMaskX : cx);
			if (x >= RasterWidth)
				break;

			const auto colour = rowDefinition[cx];
			if (!allowTransparencies || (allowTransparencies && (colour > 0))) {
				const auto outputPixel = y * RasterWidth + x;
				m_pixels[outputPixel] = m_colours->getColour(palette[colour]);
			}
		}

		break;
	}
}
