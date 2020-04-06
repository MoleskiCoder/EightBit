#pragma once

#include "Processor.h"

namespace EightBit {

	class Bus;

	class LittleEndianProcessor : public Processor {
	public:
		~LittleEndianProcessor() = default;

		register16_t peekWord(register16_t address) final;
		void pokeWord(register16_t address, register16_t value) final;

	protected:
		LittleEndianProcessor(Bus& memory);

		register16_t getWord() override;
		void setWord(register16_t value) override;

		register16_t getWordPaged(uint8_t page, uint8_t offset) override;
		void setWordPaged(uint8_t page, uint8_t offset, register16_t value) override;

		register16_t fetchWord() final;

		void pushWord(register16_t value) override;
		register16_t popWord() override;
	};
}
