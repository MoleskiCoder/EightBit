#pragma once

#include "Bus.h"
#include "Register.h"
#include "Processor.h"

namespace EightBit {
	class LittleEndianProcessor : public Processor {
	protected:
		LittleEndianProcessor(Bus& memory) : Processor(memory) {}
		virtual ~LittleEndianProcessor() = default;

		virtual register16_t getWord() {
			const auto low = BUS().read();
			++BUS().ADDRESS();
			const auto high = BUS().read();
			return register16_t(low, high);
		}

		virtual void setWord(const register16_t value) {
			BUS().write(value.low);
			++BUS().ADDRESS();
			BUS().write(value.high);
		}

		virtual register16_t getWordPaged(uint8_t page, uint8_t offset) override {
			const auto low = getBytePaged(page, offset);
			++BUS().ADDRESS().low;
			const auto high = BUS().read();
			return register16_t(low, high);
		}

		virtual void setWordPaged(uint8_t page, uint8_t offset, register16_t value) override {
			setBytePaged(page, offset, value.low);
			++BUS().ADDRESS().low;
			BUS().read(value.high);
		}

		virtual register16_t fetchWord() final {
			const auto low = fetchByte();
			const auto high = fetchByte();
			return register16_t(low, high);
		}

		virtual void pushWord(const register16_t value) final {
			push(value.high);
			push(value.low);
		}

		virtual register16_t popWord() final {
			const auto low = pop();
			const auto high = pop();
			return register16_t(low, high);
		}
	};
}
