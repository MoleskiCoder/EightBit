#pragma once

#include <vector>
#include <array>
#include <cstdint>

#include <gsl/gsl>

#include "ObjectAttribute.h"

namespace EightBit {

	class Ram;

	namespace GameBoy {

		class AbstractColourPalette;
		class CharacterDefinition;
		class Bus;

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

			Display(const gsl::not_null<AbstractColourPalette*> colours, Bus& bus, Ram& oam, Ram& vram);

			const std::vector<uint32_t>& pixels() const;

			void initialise();
			void render();
			void loadObjectAttributes();

		private:
			std::vector<uint32_t> m_pixels;
			Bus& m_bus;
			Ram& m_oam;
			Ram& m_vram;
			const AbstractColourPalette* m_colours;
			std::array<ObjectAttribute, 40> m_objectAttributes;
			uint8_t m_control = 0;
			uint8_t m_scanLine = 0;

			std::array<int, 4> createPalette(int address);

			void renderBackground();
			void renderBackground(
				int bgArea, int bgCharacters,
				int offsetX, int offsetY,
				const std::array<int, 4>& palette);

			void renderObjects();

			void renderTile(
				int height,
				int drawX, int drawY,
				bool flipX, bool flipY, bool allowTransparencies,
				const std::array<int, 4>& palette,
				const CharacterDefinition& definition);
		};
	}
}