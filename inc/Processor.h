#pragma once

#include <cstdint>

#include "ClockedChip.h"
#include "Bus.h"
#include "Register.h"

#include "EightBitCompilerDefinitions.h"

namespace EightBit {
	class Processor : public ClockedChip {
	public:
		Signal<EventArgs> ExecutingInstruction;
		Signal<EventArgs> ExecutedInstruction;

		Signal<EventArgs> ReadingMemory;
		Signal<EventArgs> ReadMemory;

		Signal<EventArgs> WritingMemory;
		Signal<EventArgs> WrittenMemory;

		// http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
		// b: number of bits representing the number in x
		// x: sign extend this b-bit number to r
		[[nodiscard]] static constexpr int8_t signExtend(int b, uint8_t x) noexcept {
			const uint8_t m = bit(b - 1); // mask can be pre-computed if b is fixed
			x = x & (bit(b) - 1);  // (Skip this if bits in x above position b are already zero.)
			const auto result = (x ^ m) - m;
			return result;
		}

		Processor(const Processor& rhs) noexcept;
		bool operator==(const Processor& rhs) const noexcept;

		[[nodiscard]] constexpr auto& PC() noexcept { return m_pc; }
		[[nodiscard]] constexpr const auto& PC() const noexcept { return m_pc; }

		[[nodiscard]] constexpr auto& intermediate() noexcept { return m_intermediate; }
		[[nodiscard]] constexpr const auto& intermediate() const noexcept { return m_intermediate; }

		int run(int limit) noexcept;
		virtual int step() noexcept;
		virtual void poweredStep() noexcept = 0;
		virtual void execute() noexcept = 0;
		void execute(uint8_t value) noexcept;

		[[nodiscard]] virtual register16_t peekShort(uint16_t address) noexcept = 0;
		virtual void pokeShort(uint16_t address, register16_t value) noexcept = 0;

		DECLARE_PIN_INPUT(RESET)
		DECLARE_PIN_INPUT(INT)

	protected:
		Processor(Bus& memory) noexcept;

		[[nodiscard]] constexpr auto& opcode() noexcept { return m_opcode; }
		[[nodiscard]] constexpr auto& BUS() noexcept { return m_bus; }

		virtual void handleRESET() noexcept;
		virtual void handleINT() noexcept;

		void memoryWrite(register16_t address, uint8_t data) noexcept;
		void memoryWrite(register16_t address) noexcept;
		void memoryWrite(uint8_t data) noexcept;
		virtual void memoryWrite() noexcept;
		void busWrite() noexcept;

		void memoryRead(uint8_t low, uint8_t high) noexcept {
			BUS().ADDRESS() = { low, high };
			memoryRead();
		}

		void memoryRead(register16_t address) noexcept;
		virtual void memoryRead() noexcept;
		void busRead() noexcept;

		virtual register16_t& incrementPC() noexcept;

		virtual void immediateAddress() noexcept;

		void fetchByte() noexcept;
		virtual uint8_t fetchInstruction() noexcept;

		virtual void getInto(register16_t& into) noexcept = 0;
		virtual void getShort() noexcept { getInto(m_intermediate); }
		virtual void setShort(register16_t value) noexcept = 0;

		void getShortPaged(register16_t address) noexcept;
		void getShortPaged(uint8_t page, uint8_t offset) noexcept;
		void getPagedInto(uint8_t page, uint8_t offset, register16_t& into) noexcept;
		void getShortPaged() noexcept { getPagedInto(m_intermediate); }
		virtual void getPagedInto(register16_t& into) noexcept = 0;
		void setPaged(register16_t address, register16_t value) noexcept;
		void setPaged(uint8_t page, uint8_t offset, register16_t value) noexcept;
		virtual void setPaged(register16_t value) noexcept = 0;

		virtual void fetchInto(register16_t& into) noexcept = 0;
		void fetchShort() noexcept { fetchInto(m_intermediate); }
		void fetchShortAddress() noexcept;

		virtual void push(uint8_t value) noexcept = 0;
		virtual void pop() noexcept = 0;

		virtual void pushShort(register16_t value) noexcept = 0;
		virtual void popInto(register16_t& into) noexcept = 0;

		void getShort(register16_t address) noexcept;
		void setShort(register16_t address, register16_t value) noexcept;

		void jump(const register16_t destination) noexcept;
		void jump(uint16_t destination) noexcept;

		virtual void call(register16_t destination) noexcept;
		virtual void ret() noexcept;

	private:
		Bus& m_bus;
		uint8_t m_opcode = Mask8;
		register16_t m_pc;
		register16_t m_intermediate;
	};
}
