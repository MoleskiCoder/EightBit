#pragma once

#include <cstdint>
#include <array>

#include "LittleEndianProcessor.h"
#include "Register.h"
#include "EventArgs.h"
#include "Signal.h"
#include "EightBitCompilerDefinitions.h"

#define NON_CONST_ACCESSOR(accessor, type) \
	[[nodiscard]] constexpr auto& accessor() noexcept { \
		const auto& consted = *this; \
		const auto& reference = consted.accessor(); \
		return const_cast<type&>(reference); \
	}

#define NON_CONST_REGISTOR_ACCESSOR(accessor) \
	NON_CONST_ACCESSOR(accessor, register16_t)

namespace EightBit {

	class Bus;

	class IntelProcessor : public LittleEndianProcessor {
	public:
		struct opcode_decoded_t {

			int x = 0;
			int y = 0;
			int z = 0;
			int p = 0;
			int q = 0;

			constexpr opcode_decoded_t() noexcept {}

			constexpr opcode_decoded_t(const uint8_t opcode) noexcept {
				x = (opcode & 0b11000000) >> 6;	// 0 - 3
				y = (opcode & 0b00111000) >> 3;	// 0 - 7
				z = (opcode & 0b00000111);		// 0 - 7
				p = (y & 0b110) >> 1;			// 0 - 3
				q = (y & 1);					// 0 - 1
			}
		};

		IntelProcessor(const IntelProcessor& rhs) noexcept;
		bool operator==(const IntelProcessor& rhs) const noexcept;

		[[nodiscard]] constexpr const auto& getDecodedOpcode(const size_t i) const noexcept {
			return m_decodedOpcodes[i];
		}

		[[nodiscard]] constexpr auto& MEMPTR() noexcept { return m_memptr; }
		[[nodiscard]] constexpr auto MEMPTR() const noexcept { return m_memptr; }

		[[nodiscard]] constexpr auto& SP() noexcept { return m_sp; }
		[[nodiscard]] register16_t SP() const noexcept { return m_sp; }

		[[nodiscard]] virtual const register16_t& AF() const noexcept = 0;
		NON_CONST_REGISTOR_ACCESSOR(AF);
		[[nodiscard]] auto& A() noexcept { return AF().high; }
		[[nodiscard]] auto& F() noexcept { return AF().low; }

		[[nodiscard]] virtual const register16_t& BC() const noexcept = 0;
		NON_CONST_REGISTOR_ACCESSOR(BC);
		[[nodiscard]] auto& B() noexcept { return BC().high; }
		[[nodiscard]] auto& C() noexcept { return BC().low; }

		[[nodiscard]] virtual const register16_t& DE() const noexcept = 0;
		NON_CONST_REGISTOR_ACCESSOR(DE);
		[[nodiscard]] auto& D() noexcept { return DE().high; }
		[[nodiscard]] auto& E() noexcept { return DE().low; }

		[[nodiscard]] virtual const register16_t& HL() const noexcept = 0;
		NON_CONST_REGISTOR_ACCESSOR(HL);
		[[nodiscard]] auto& H() noexcept { return HL().high; }
		[[nodiscard]] auto& L() noexcept { return HL().low; }

		DECLARE_PIN_OUTPUT(HALT)

	protected:
		IntelProcessor(Bus& bus);

		template<class T> [[nodiscard]] static constexpr uint8_t adjustSign(uint8_t f, const uint8_t value) noexcept {
			return setBit(f, T::SF, value & T::SF);
		}

		template<class T> [[nodiscard]] static constexpr uint8_t adjustZero(uint8_t f, const uint8_t value) noexcept {
			return clearBit(f, T::ZF, value);
		}

		template<class T> [[nodiscard]] static constexpr uint8_t adjustParity(uint8_t f, const uint8_t value) noexcept {
			return clearBit(f, T::PF, PARITY(value));
		}

		template<class T> [[nodiscard]] static constexpr uint8_t adjustSZ(uint8_t f, const uint8_t value) noexcept {
			const auto intermediate = adjustSign<T>(f, value);
			return adjustZero<T>(intermediate, value);
		}

		template<class T> [[nodiscard]] static constexpr uint8_t adjustSZP(uint8_t f, const uint8_t value) noexcept {
			const auto intermediate = adjustSZ<T>(f, value);
			return adjustParity<T>(intermediate, value);
		}

		template<class T> [[nodiscard]] static constexpr uint8_t adjustXY(uint8_t f, const uint8_t value) noexcept {
			const auto intermediate = setBit(f, T::XF, value & T::XF);
			return setBit(intermediate, T::YF, value & T::YF);
		}

		template<class T> [[nodiscard]] static constexpr uint8_t adjustSZPXY(uint8_t f, const uint8_t value) noexcept {
			const auto intermediate = adjustSZP<T>(f, value);
			return adjustXY<T>(intermediate, value);
		}

		template<class T> [[nodiscard]] static constexpr uint8_t adjustSZXY(uint8_t f, const uint8_t value) noexcept {
			const auto intermediate = adjustSZ<T>(f, value);
			return adjustXY<T>(intermediate, value);
		}

		//

		[[nodiscard]] static constexpr auto buildHalfCarryIndex(const uint8_t before, const uint8_t value, const int calculation) noexcept {
			return ((before & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
		}

		[[nodiscard]] static constexpr auto calculateHalfCarry(const std::array<int, 8>& table, const uint8_t before, const uint8_t value, const int calculation) noexcept {
			const auto index = buildHalfCarryIndex(before, value, calculation);
			return table[index & Mask3];
		}

		[[nodiscard]] static constexpr auto calculateHalfCarryAdd(const uint8_t before, const uint8_t value, const int calculation) noexcept {
			return calculateHalfCarry(m_halfCarryTableAdd, before, value, calculation);
		}

		[[nodiscard]] static constexpr auto calculateHalfCarrySub(const uint8_t before, const uint8_t value, const int calculation) noexcept {
			return calculateHalfCarry(m_halfCarryTableSub, before, value, calculation);
		}

		void handleRESET() override;

		void push(uint8_t value) final;
		[[nodiscard]] uint8_t pop() final;

		//

		[[nodiscard]] register16_t getWord() final;
		void setWord(register16_t value) final;

		//

		virtual void restart(uint8_t address);
		virtual int callConditional(int condition);
		virtual int jumpConditional(int condition);
		virtual int returnConditional(int condition);
		virtual void jr(int8_t offset) noexcept;
		virtual int jrConditional(int condition);
		void ret() override;

		virtual void fetchWordMEMPTR();

		void jumpIndirect();
		void callIndirect();

		void resetWorkingRegisters() noexcept;

	private:
		static std::array<int, 8> m_halfCarryTableAdd;
		static std::array<int, 8> m_halfCarryTableSub;

		std::array<opcode_decoded_t, 0x100> m_decodedOpcodes;
		register16_t m_sp = Mask16;
		register16_t m_memptr;
	};
}
