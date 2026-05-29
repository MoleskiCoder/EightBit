#pragma once

#include "Processor.h"

namespace EightBit {

	class Bus;

	class LittleEndianProcessor : public Processor {
	public:
		virtual ~LittleEndianProcessor() noexcept = default;
		LittleEndianProcessor(const LittleEndianProcessor& rhs) noexcept;

		[[nodiscard]] register16_t peekShort(uint16_t address) noexcept final;
		void pokeShort(uint16_t address, register16_t value) noexcept final;

	protected:
		LittleEndianProcessor(Bus& memory) noexcept;

		void getInto(register16_t& into) noexcept final;
		void setShort(register16_t value) noexcept override;

		void getPagedInto(register16_t& into) noexcept final;
		void setPaged(register16_t value) noexcept final;

		void fetchInto(register16_t& into) noexcept final;

		void pushShort(register16_t value) noexcept final;
		void popInto(register16_t& into) noexcept final;
	};
}
