#pragma once

#include "Processor.h"

namespace EightBit {

	class Bus;

	class LittleEndianProcessor : public Processor {
	public:
		LittleEndianProcessor(const LittleEndianProcessor& rhs) noexcept;

		[[nodiscard]] register16_t peekWord(register16_t address) noexcept final;
		void pokeWord(register16_t address, register16_t value) noexcept final;

	protected:
		LittleEndianProcessor(Bus& memory) noexcept;

		[[nodiscard]] register16_t getWord() override;
		void setWord(register16_t value) override;

		[[nodiscard]] register16_t getWordPaged() override;
		void setWordPaged(register16_t value) override;

		[[nodiscard]] register16_t fetchWord() final;

		void pushWord(register16_t value) override;
		[[nodiscard]] register16_t popWord() override;
	};
}
