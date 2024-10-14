#pragma once

#include <array>
#include <cstdint>

#include "ObjectAttribute.h"

namespace EightBit {

	class Ram;

	namespace GameBoy {

		class AbstractColourPalette;
		class CharacterDefinition;
		class Bus;

		class Display final {
		public:
			enum {
				BufferWidth = 256,
				BufferHeight = 256,
				BufferCharacterWidth = BufferWidth / 8,
				BufferCharacterHeight = BufferHeight / 8,
				RasterWidth = 160,
				RasterHeight = 144,
				PixelCount = RasterWidth * RasterHeight,
			};

			Display(const AbstractColourPalette* colours, Bus& bus, Ram& oam, Ram& vram) noexcept;

			[[nodiscard]] const auto& pixels() const noexcept { return m_pixels; }

			void renderCurrentScanline() noexcept;
			void loadObjectAttributes() noexcept;

		private:
			enum class tile_offset_t  {
				Signed, Unsigned,
			};

			std::array<uint32_t, PixelCount> m_pixels = { 0 };
			Bus& m_bus;
			Ram& m_oam;
			Ram& m_vram;
			const AbstractColourPalette* m_colours;
			std::array<ObjectAttribute, 40> m_objectAttributes = { ObjectAttribute() };
			uint8_t m_control = 0;
			uint8_t m_scanLine = 0;

			[[nodiscard]] std::array<int, 4> createPalette(int address) noexcept;

			void renderBackground() noexcept;
			void renderBackground(
				int bgArea, int bgCharacters,
				tile_offset_t offsetType,
				int offsetX, int offsetY,
				const std::array<int, 4>& palette) noexcept;

			void renderBackgroundTile(
				int definitionOffset,
				int row, int column,
				int bgCharacters, tile_offset_t offsetType,
				int offsetX, int offsetY,
				const std::array<int, 4>& palette) noexcept;

			void renderObjects() noexcept;

			void renderSpriteTile(
				int height,
				int drawX, int drawY,
				bool flipX, bool flipY,
				const std::array<int, 4>& palette,
				const CharacterDefinition& definition) noexcept;

			void renderBackgroundTile(
				int drawX, int drawY,
				const std::array<int, 4>& palette,
				const CharacterDefinition& definition) noexcept;

			void renderTile(
				int height,
				int drawX, int drawY,
				bool flipX, bool flipY, bool allowTransparencies,
				const std::array<int, 4>& palette,
				const CharacterDefinition& definition) noexcept;
		};
	}
}