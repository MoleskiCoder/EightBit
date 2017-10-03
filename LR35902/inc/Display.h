#pragma once

#include <vector>
#include <array>
#include <cstdint>

#include "GameBoyBus.h"
#include "AbstractColourPalette.h"
#include "ObjectAttribute.h"

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
			void loadObjectAttributes();

		private:
			std::vector<uint32_t> m_pixels;
			Bus& m_bus;
			const AbstractColourPalette* m_colours;
			std::array<ObjectAttribute, 40> m_objectAttributes;
			uint8_t m_currentScanLine;

			std::array<int, 4> createPalette(int address);

			void renderBackground();
			void renderBackground(
				int bgArea, int bgCharacters,
				int offsetX, int offsetY,
				const std::array<int, 4>& palette);

			void renderObjects();
			void renderObjects(int objBlockHeight);

			void renderTile(
				int height,
				int drawX, int drawY,
				bool flipX, bool flipY, bool allowTransparencies,
				const std::array<int, 4>& palette,
				const CharacterDefinition& definition);
		};
	}
}