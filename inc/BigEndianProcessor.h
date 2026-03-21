#pragma once

#include "Processor.h"

namespace EightBit {

	class Bus;

	class BigEndianProcessor : public Processor {
	public:
		virtual ~BigEndianProcessor() noexcept = default;
		BigEndianProcessor(const BigEndianProcessor& rhs) noexcept;

		[[nodiscard]] register16_t peekWord(register16_t address) noexcept final;
		void pokeWord(register16_t address, register16_t value) noexcept final;

	protected:
		BigEndianProcessor(Bus& memory) noexcept;

		[[nodiscard]] register16_t getWord() noexcept override;
		void setWord(register16_t value) noexcept override;

		[[nodiscard]] register16_t getWordPaged() noexcept override;
		void setWordPaged(register16_t value) noexcept override;

		[[nodiscard]] register16_t fetchWord() noexcept override;

		void pushWord(register16_t value) noexcept override;
		[[nodiscard]] register16_t popWord() noexcept override;
	};
}
