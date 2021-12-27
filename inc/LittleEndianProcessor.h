#pragma once

#include "Processor.h"

namespace EightBit {

	class Bus;

	class LittleEndianProcessor : public Processor {
	public:
		virtual ~LittleEndianProcessor() {};
		LittleEndianProcessor(const LittleEndianProcessor& rhs);

		[[nodiscard]] register16_t peekWord(register16_t address) noexcept final;
		void pokeWord(register16_t address, register16_t value) noexcept final;

	protected:
		LittleEndianProcessor(Bus& memory);

		[[nodiscard]] register16_t getWord() override;
		void setWord(register16_t value) override;

		[[nodiscard]] register16_t getWordPaged(uint8_t page, uint8_t offset) override;
		void setWordPaged(uint8_t page, uint8_t offset, register16_t value) override;

		[[nodiscard]] register16_t fetchWord() final;

		void pushWord(register16_t value) override;
		[[nodiscard]] register16_t popWord() override;
	};
}
