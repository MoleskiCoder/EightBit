#pragma once

#include "Bus.h"
#include "Register.h"
#include "Processor.h"

namespace EightBit {
	class BigEndianProcessor : public Processor {
	protected:
		BigEndianProcessor(Bus& memory) : Processor(memory) {}
		virtual ~BigEndianProcessor() = default;

		virtual register16_t getWord() {
			const auto high = BUS().read();
			++BUS().ADDRESS();
			const auto low = BUS().read();
			return register16_t(low, high);
		}

		virtual void setWord(const register16_t value) {
			BUS().write(value.high);
			++BUS().ADDRESS();
			BUS().write(value.low);
		}

		virtual register16_t getWordPaged(uint8_t page, uint8_t offset) override {
			const auto high = getBytePaged(page, offset);
			++BUS().ADDRESS().low;
			const auto low = BUS().read();
			return register16_t(low, high);
		}

		virtual void setWordPaged(uint8_t page, uint8_t offset, register16_t value) override {
			setBytePaged(page, offset, value.high);
			++BUS().ADDRESS().low;
			BUS().read(value.low);
		}

		virtual register16_t fetchWord() final {
			const auto high = fetchByte();
			const auto low = fetchByte();
			return register16_t(low, high);
		}

		virtual void pushWord(const register16_t value) final {
			push(value.low);
			push(value.high);
		}

		virtual register16_t popWord() final {
			const auto high = pop();
			const auto low = pop();
			return register16_t(low, high);
		}
	};
}
