#pragma once

#include <vector>
#include <array>
#include <cstdint>

#include "GameBoyBus.h"
#include "AbstractColourPalette.h"

namespace EightBit {
	namespace GameBoy {

		class CharacterDefinition;

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

			std::array<int, 4> createPalette(int address);

			void renderBackground();
			void renderBackground(
				int bgArea, int bgCharacters,
				int offsetX, int offsetY,
				int scrollX, int scrollY,
				const std::array<int, 4>& palette);

			void renderObjects();
			void renderObjects(int objBlockHeight);

			void renderTile(
				int drawX, int drawY, int offsetX, int offsetY,
				bool flipX, bool flipY, bool allowTransparencies,
				const std::array<int, 4>& palette,
				const CharacterDefinition& definition);
		};
	}
}