#pragma once

#include "Processor.h"

namespace EightBit {

	class Bus;

	class BigEndianProcessor : public Processor {
	public:
		virtual ~BigEndianProcessor() noexcept {};

		[[nodiscard]] register16_t peekWord(register16_t address) noexcept final;
		void pokeWord(register16_t address, register16_t value) noexcept final;

	protected:
		BigEndianProcessor(Bus& memory);

		[[nodiscard]] register16_t getWord() noexcept override;
		void setWord(register16_t value) noexcept override;

		[[nodiscard]] register16_t getWordPaged(uint8_t page, uint8_t offset) noexcept override;
		void setWordPaged(uint8_t page, uint8_t offset, register16_t value) noexcept override;

		[[nodiscard]] register16_t fetchWord() noexcept final;

		void pushWord(register16_t value) noexcept final;
		[[nodiscard]] register16_t popWord() noexcept final;
	};
}
