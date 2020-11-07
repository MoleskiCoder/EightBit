#include "stdafx.h"
#include "Display.h"
#include "CharacterDefinition.h"
#include "ObjectAttribute.h"
#include "GameBoyBus.h"
#include "AbstractColourPalette.h"

#include <Processor.h>

EightBit::GameBoy::Display::Display(const AbstractColourPalette* colours, Bus& bus, Ram& oam, Ram& vram)
: m_bus(bus),
  m_oam(oam),
  m_vram(vram),
  m_colours(colours) {
}

void EightBit::GameBoy::Display::renderCurrentScanline() {
	m_scanLine = m_bus.IO().peek(IoRegisters::LY);
	if (m_scanLine < RasterHeight) {
		m_control = m_bus.IO().peek(IoRegisters::LCDC);
		assert(m_control & IoRegisters::LCD_EN);
		if (m_control & IoRegisters::BG_EN)
			renderBackground();
		if (m_control & IoRegisters::OBJ_EN)
			renderObjects();
	}
}

std::array<int, 4> EightBit::GameBoy::Display::createPalette(const int address) {
	const auto raw = m_bus.IO().peek(address);
	const std::array<int, 4> palette = {
		raw & 0b11,
		(raw & 0b1100) >> 2,
		(raw & 0b110000) >> 4,
		(raw & 0b11000000) >> 6,
	};
	return palette;
}

void EightBit::GameBoy::Display::loadObjectAttributes() {
	for (int i = 0; i < 40; ++i)
		m_objectAttributes[i] = ObjectAttribute(m_oam, 4 * i);
}

void EightBit::GameBoy::Display::renderObjects() {
	
	const auto objBlockHeight = (m_control & IoRegisters::OBJ_SIZE) ? 16 : 8;

	const std::array<std::array<int, 4>, 2> palettes = {
		createPalette(IoRegisters::OBP0),
		createPalette(IoRegisters::OBP1)
	};

	const auto characterAddressMultiplier = objBlockHeight == 8 ? 16 : 8;

	for (int i = 0; i < 40; ++i) {

		const auto& current = m_objectAttributes[i];

		const auto spriteY = current.positionY();
		const auto drawY = spriteY - 16;

		if ((m_scanLine >= drawY) && (m_scanLine < (drawY + objBlockHeight))) {

			const auto spriteX = current.positionX();
			const auto drawX = spriteX - 8;

			const auto sprite = current.pattern();
			const auto definition = CharacterDefinition(m_vram, characterAddressMultiplier * sprite);
			const auto& palette = palettes[current.palette()];
			const auto flipX = current.flipX();
			const auto flipY = current.flipY();

			renderSpriteTile(
				objBlockHeight,
				drawX, drawY,
				flipX, flipY,
				palette,
				definition);
		}
	}
}

void EightBit::GameBoy::Display::renderBackground() {

	const auto palette = createPalette(IoRegisters::BGP);

	const auto window = !!(m_control & IoRegisters::WIN_EN);
	const auto bgArea = (m_control & IoRegisters::BG_MAP) ? 0x1c00 : 0x1800;
	const auto bgCharacters = (m_control & IoRegisters::TILE_SEL) ? 0 : 0x1000;

	const auto wx = m_bus.IO().peek(IoRegisters::WX);
	const auto wy = m_bus.IO().peek(IoRegisters::WY);

	const auto offsetX = window ? wx - 7 : 0;
	const auto offsetY = window ? wy : 0;

	const auto scrollX = m_bus.IO().peek(IoRegisters::SCX);
	const auto scrollY = m_bus.IO().peek(IoRegisters::SCY);

	const auto offsetType = bgCharacters == 0 ? tile_offset_t::Unsigned : tile_offset_t::Signed;
	renderBackground(bgArea, bgCharacters, offsetType, offsetX - scrollX, offsetY - scrollY, palette);
}

void EightBit::GameBoy::Display::renderBackground(
		int bgArea, int bgCharacters, tile_offset_t offsetType,
		int offsetX, int offsetY,
		const std::array<int, 4>& palette) {

	const int row = (m_scanLine - offsetY) / 8;
	auto address = bgArea + row * BufferCharacterWidth;

	for (int column = 0; column < BufferCharacterWidth; ++column) {

		const auto character = m_vram.peek(address++);

		const auto definitionOffset = offsetType == tile_offset_t::Signed ? 16 * (int8_t)character : 16 * character;
		const auto definition = CharacterDefinition(m_vram, bgCharacters + definitionOffset);
		renderBackgroundTile(
			column * 8 + offsetX, row * 8 + offsetY,
			palette,
			definition);
	}
}

void EightBit::GameBoy::Display::renderSpriteTile(
		const int height,
		const int drawX, const int drawY,
		const bool flipX, const bool flipY,
		const std::array<int, 4>& palette,
		const CharacterDefinition& definition) {
	renderTile(
		height,
		drawX, drawY,
		flipX, flipY, true,
		palette,
		definition);
}

void EightBit::GameBoy::Display::renderBackgroundTile(
		const int drawX, const int drawY,
		const std::array<int, 4>& palette,
		const CharacterDefinition& definition) {
	renderTile(
		8,
		drawX, drawY,
		false, false, false,
		palette,
		definition);
}

void EightBit::GameBoy::Display::renderTile(
		const int height,
		const int drawX, const int drawY,
		const bool flipX, const bool flipY, const bool allowTransparencies,
		const std::array<int, 4>& palette,
		const CharacterDefinition& definition) {

	const auto width = 8;

	const auto flipMaskX = width - 1;
	const auto flipMaskY = height - 1;

	const auto y = m_scanLine;

	auto cy = y - drawY;
	if (flipY)
		cy = ~cy & flipMaskY;

	const auto rowDefinition = definition.get(cy);

	const auto lineAddress = y * RasterWidth;
	for (int cx = 0; cx < width; ++cx) {

		const uint8_t x = drawX + (flipX ? ~cx & flipMaskX : cx);
		if (x >= RasterWidth)
			break;

		const auto colour = rowDefinition[cx];
		if (!allowTransparencies || (allowTransparencies && (colour > 0))) {
			const auto outputPixel = lineAddress + x;
			m_pixels[outputPixel] = m_colours->colour(palette[colour]);
		}
	}
}
