#pragma once

#include <cstdint>
#include <array>

#include "Bus.h"
#include "LittleEndianProcessor.h"
#include "Register.h"

#include "EightBitCompilerDefinitions.h"

namespace EightBit {
	class IntelProcessor : public LittleEndianProcessor
	{
	public:
		struct opcode_decoded_t {

			int x = 0;
			int y = 0;
			int z = 0;
			int p = 0;
			int q = 0;

			opcode_decoded_t() noexcept {}

			opcode_decoded_t(const uint8_t opcode) {
				x = (opcode & 0b11000000) >> 6;	// 0 - 3
				y = (opcode & 0b00111000) >> 3;	// 0 - 7
				z = (opcode & 0b00000111);		// 0 - 7
				p = (y & 0b110) >> 1;			// 0 - 3
				q = (y & 1);					// 0 - 1
			}
		};

		const opcode_decoded_t& getDecodedOpcode(const int i) const {
			return m_decodedOpcodes[i];
		}

		register16_t& MEMPTR() { return m_memptr; }

		register16_t& SP() { return m_sp; }

		virtual register16_t& AF() = 0;
		uint8_t& A() { return AF().high; }
		uint8_t& F() { return AF().low; }

		virtual register16_t& BC() = 0;
		uint8_t& B() { return BC().high; }
		uint8_t& C() { return BC().low; }

		virtual register16_t& DE() = 0;
		uint8_t& D() { return DE().high; }
		uint8_t& E() { return DE().low; }

		virtual register16_t& HL() = 0;
		uint8_t& H() { return HL().high; }
		uint8_t& L() { return HL().low; }

	protected:
		IntelProcessor(Bus& bus);
		virtual ~IntelProcessor() = default;

		virtual void reset() override;

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

		static int buildHalfCarryIndex(const uint8_t before, const uint8_t value, const int calculation) {
			return ((before & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
		}

		static bool calculateHalfCarryAdd(const uint8_t before, const uint8_t value, const int calculation) {
			static std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
			const auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableAdd[index & Mask3];
		}

		static bool calculateHalfCarrySub(const uint8_t before, const uint8_t value, const int calculation) {
			std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };
			const auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableSub[index & Mask3];
		}

		virtual void push(uint8_t value) final;
		virtual uint8_t pop() final;

		//

		virtual register16_t getWord() final;
		virtual void setWord(register16_t value) final;

		//

		void restart(const uint8_t address) {
			call(MEMPTR() = register16_t(address, 0));
		}

		bool callConditional(const int condition) {
			MEMPTR() = fetchWord();
			if (condition)
				call(MEMPTR());
			return !!condition;
		}

		bool jumpConditional(const int condition) {
			MEMPTR() = fetchWord();
			if (condition)
				jump(MEMPTR());
			return !!condition;
		}

		bool returnConditional(const int condition) {
			if (condition)
				ret();
			return !!condition;
		}

		void jr(const int8_t offset) {
			jump(MEMPTR() = PC() + offset);
		}

		bool jrConditional(const int condition) {
			const auto offset = fetchByte();
			if (condition)
				jr(offset);
			return !!condition;
		}

		virtual void ret() final {
			Processor::ret();
			MEMPTR() = PC();
		}

	private:
		std::array<opcode_decoded_t, 0x100> m_decodedOpcodes;
		register16_t m_sp = 0xffff;
		register16_t m_memptr;
	};
}
