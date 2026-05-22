#pragma once

#include "Processor.h"

namespace EightBit {

	class Bus;

	class BigEndianProcessor : public Processor {
	public:
		virtual ~BigEndianProcessor() noexcept = default;
		BigEndianProcessor(const BigEndianProcessor& rhs) noexcept;

		[[nodiscard]] register16_t peekShort(uint16_t address) noexcept final;
		void pokeShort(uint16_t address, register16_t value) noexcept final;

	protected:
		BigEndianProcessor(Bus& memory) noexcept;

		void getInto(register16_t& into) noexcept override;
		void setShort(register16_t value) noexcept override;
		
		void getPagedInto(register16_t& into) noexcept override;
		void setPaged(register16_t value) noexcept override;

		void fetchInto(register16_t& into) noexcept override;

		void pushShort(register16_t value) noexcept override;
		void popInto(register16_t& into) noexcept override;
	};
}
