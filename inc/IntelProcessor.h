#pragma once

#include <cstdint>
#include <array>

#include "Bus.h"
#include "LittleEndianProcessor.h"
#include "Register.h"

#include "EightBitCompilerDefinitions.h"

namespace EightBit {
	class IntelProcessor : public LittleEndianProcessor {
	public:
		struct opcode_decoded_t {

			int x = 0;
			int y = 0;
			int z = 0;
			int p = 0;
			int q = 0;

			opcode_decoded_t() {}

			opcode_decoded_t(const uint8_t opcode) {
				x = (opcode & 0b11000000) >> 6;	// 0 - 3
				y = (opcode & 0b00111000) >> 3;	// 0 - 7
				z = (opcode & 0b00000111);		// 0 - 7
				p = (y & 0b110) >> 1;			// 0 - 3
				q = (y & 1);					// 0 - 1
			}
		};

		~IntelProcessor() = default;

		[[nodiscard]] const auto& getDecodedOpcode(const size_t i) const noexcept {
			return m_decodedOpcodes[i];
		}

		[[nodiscard]] auto& MEMPTR() noexcept { return m_memptr; }

		[[nodiscard]] auto& SP() noexcept { return m_sp; }

		[[nodiscard]] virtual register16_t& AF() = 0;
		[[nodiscard]] auto& A() { return AF().high; }
		[[nodiscard]] auto& F() { return AF().low; }

		[[nodiscard]] virtual register16_t& BC() = 0;
		[[nodiscard]] auto& B() { return BC().high; }
		[[nodiscard]] auto& C() { return BC().low; }

		[[nodiscard]] virtual register16_t& DE() = 0;
		[[nodiscard]] auto& D() { return DE().high; }
		[[nodiscard]] auto& E() { return DE().low; }

		[[nodiscard]] virtual register16_t& HL() = 0;
		[[nodiscard]] auto& H() { return HL().high; }
		[[nodiscard]] auto& L() { return HL().low; }

		void powerOn() override;

	protected:
		IntelProcessor(Bus& bus);

		template<class T> static void adjustSign(uint8_t& f, const uint8_t value) {
			setFlag(f, T::SF, value & T::SF);
		}

		template<class T> static void adjustZero(uint8_t& f, const uint8_t value) {
			clearFlag(f, T::ZF, value);
		}

		template<class T> static void adjustParity(uint8_t& f, const uint8_t value) {
			clearFlag(f, T::PF, PARITY(value));
		}

		template<class T> static void adjustSZ(uint8_t& f, const uint8_t value) {
			adjustSign<T>(f, value);
			adjustZero<T>(f, value);
		}

		template<class T> static void adjustSZP(uint8_t& f, const uint8_t value) {
			adjustSZ<T>(f, value);
			adjustParity<T>(f, value);
		}

		template<class T> static void adjustXY(uint8_t& f, const uint8_t value) {
			setFlag(f, T::XF, value & T::XF);
			setFlag(f, T::YF, value & T::YF);
		}

		template<class T> static void adjustSZPXY(uint8_t& f, const uint8_t value) {
			adjustSZP<T>(f, value);
			adjustXY<T>(f, value);
		}

		template<class T> static void adjustSZXY(uint8_t& f, const uint8_t value) {
			adjustSZ<T>(f, value);
			adjustXY<T>(f, value);
		}

		//

		static constexpr auto buildHalfCarryIndex(const uint8_t before, const uint8_t value, const int calculation) {
			return ((before & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
		}

		[[nodiscard]] static auto calculateHalfCarryAdd(const uint8_t before, const uint8_t value, const int calculation) noexcept {
			static std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
			const auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableAdd[index & Mask3];
		}

		[[nodiscard]] static auto calculateHalfCarrySub(const uint8_t before, const uint8_t value, const int calculation) noexcept {
			std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };
			const auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableSub[index & Mask3];
		}

		void push(uint8_t value) final;
		[[nodiscard]] uint8_t pop() final;

		//

		[[nodiscard]] register16_t getWord() final;
		void setWord(register16_t value) final;

		//

		void restart(const uint8_t address) {
			call(MEMPTR() = { address, 0 });
		}

		auto callConditional(const int condition) {
			MEMPTR() = fetchWord();
			if (condition)
				call(MEMPTR());
			return !!condition;
		}

		auto jumpConditional(const int condition) {
			MEMPTR() = fetchWord();
			if (condition)
				jump(MEMPTR());
			return !!condition;
		}

		auto returnConditional(const int condition) {
			if (condition)
				ret();
			return !!condition;
		}

		void jr(const int8_t offset) {
			jump(MEMPTR() = PC() + offset);
		}

		auto jrConditional(const int condition) {
			const int8_t offset = fetchByte();
			if (condition)
				jr(offset);
			return !!condition;
		}

		void ret() final;

	private:
		std::array<opcode_decoded_t, 0x100> m_decodedOpcodes;
		register16_t m_sp = Mask16;
		register16_t m_memptr;
	};
}
