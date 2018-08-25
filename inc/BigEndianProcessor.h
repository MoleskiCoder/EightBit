#pragma once

#include "Bus.h"
#include "Register.h"
#include "Processor.h"

namespace EightBit {
	class BigEndianProcessor : public Processor {
	protected:
		BigEndianProcessor(Bus& memory);
		virtual ~BigEndianProcessor() = default;

		virtual register16_t getWord() override;
		virtual void setWord(register16_t value) override;

		virtual register16_t getWordPaged(uint8_t page, uint8_t offset) override;
		virtual void setWordPaged(uint8_t page, uint8_t offset, register16_t value) override;

		virtual register16_t fetchWord() final;

		virtual void pushWord(register16_t value) final;
		virtual register16_t popWord() final;
	};
}
