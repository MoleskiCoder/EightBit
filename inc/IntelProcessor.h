#pragma once

#include <cstdint>
#include <array>

#include "LittleEndianProcessor.h"
#include "Register.h"
#include "EventArgs.h"
#include "Signal.h"
#include "EightBitCompilerDefinitions.h"

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

		virtual ~IntelProcessor() {};

		[[nodiscard]] constexpr const auto& getDecodedOpcode(const size_t i) const noexcept {
			return m_decodedOpcodes[i];
		}

		[[nodiscard]] constexpr auto& MEMPTR() noexcept { return m_memptr; }

		[[nodiscard]] constexpr auto& SP() noexcept { return m_sp; }

		[[nodiscard]] virtual register16_t& AF() noexcept = 0;
		[[nodiscard]] auto& A() noexcept { return AF().high; }
		[[nodiscard]] auto& F() noexcept { return AF().low; }

		[[nodiscard]] virtual register16_t& BC() noexcept = 0;
		[[nodiscard]] auto& B() noexcept { return BC().high; }
		[[nodiscard]] auto& C() noexcept { return BC().low; }

		[[nodiscard]] virtual register16_t& DE() noexcept = 0;
		[[nodiscard]] auto& D() noexcept { return DE().high; }
		[[nodiscard]] auto& E() noexcept { return DE().low; }

		[[nodiscard]] virtual register16_t& HL() noexcept = 0;
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

		[[nodiscard]] static auto calculateHalfCarryAdd(const uint8_t before, const uint8_t value, const int calculation) noexcept {
			static std::array<int, 8> halfCarryTableAdd = { { 0, 0, 1, 0, 1, 0, 1, 1} };
			const auto index = buildHalfCarryIndex(before, value, calculation);
			return halfCarryTableAdd[index & Mask3];
		}

		[[nodiscard]] static constexpr auto calculateHalfCarrySub(const uint8_t before, const uint8_t value, const int calculation) noexcept {
			std::array<int, 8> halfCarryTableSub = { { 0, 1, 1, 1, 0, 0, 0, 1 } };
			const auto index = buildHalfCarryIndex(before, value, calculation);
			return halfCarryTableSub[index & Mask3];
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
		virtual void jr(int8_t offset);
		virtual int jrConditional(int condition);
		void ret() override;

	private:
		std::array<opcode_decoded_t, 0x100> m_decodedOpcodes;
		register16_t m_sp = Mask16;
		register16_t m_memptr;
	};
}
