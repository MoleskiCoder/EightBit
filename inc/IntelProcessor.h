#pragma once

#include <cstdint>
#include <array>

#include "Processor.h"

namespace EightBit {
	class IntelProcessor : public Processor
	{
	public:
		struct opcode_decoded_t {

			int x;
			int y;
			int z;
			int p;
			int q;

			opcode_decoded_t() {
				x = y = z = p = q = 0;
			}

			opcode_decoded_t(uint8_t opcode) {
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

		virtual void initialise() override;
		virtual void reset() override;

		register16_t& SP() { return sp; }

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
		IntelProcessor(Memory& memory);

		template<class T> static void adjustSign(uint8_t& f, uint8_t value) {
			setFlag(f, T::SF, value & T::SF);
		}

		template<class T> static void adjustZero(uint8_t& f, uint8_t value) {
			clearFlag(f, T::ZF, value);
		}

		template<class T> static void adjustParity(uint8_t& f, uint8_t value) {
			clearFlag(f, T::PF, __popcnt(value) % 2);
		}

		template<class T> static void adjustSZ(uint8_t& f, uint8_t value) {
			adjustSign<T>(f, value);
			adjustZero<T>(f, value);
		}

		template<class T> static void adjustSZP(uint8_t& f, uint8_t value) {
			adjustSZ<T>(f, value);
			adjustParity<T>(f, value);
		}

		template<class T> static void adjustXY(uint8_t& f, uint8_t value) {
			setFlag(f, T::XF, value & T::XF);
			setFlag(f, T::YF, value & T::YF);
		}

		template<class T> static void adjustSZPXY(uint8_t& f, uint8_t value) {
			adjustSZP<T>(f, value);
			adjustXY<T>(f, value);
		}

		template<class T> static void adjustSZXY(uint8_t& f, uint8_t value) {
			adjustSZ<T>(f, value);
			adjustXY<T>(f, value);
		}

		//

		static int buildHalfCarryIndex(uint8_t before, uint8_t value, int calculation) {
			return ((before & 0x88) >> 1) | ((value & 0x88) >> 2) | ((calculation & 0x88) >> 3);
		}

		static bool calculateHalfCarryAdd(uint8_t before, uint8_t value, int calculation) {
			static std::array<bool, 8> m_halfCarryTableAdd = { { false, false, true, false, true, false, true, true } };
			auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableAdd[index & Mask3];
		}

		static bool calculateHalfCarrySub(uint8_t before, uint8_t value, int calculation) {
			std::array<bool, 8> m_halfCarryTableSub = { { false, true, true, true, false, false, false, true } };
			auto index = buildHalfCarryIndex(before, value, calculation);
			return m_halfCarryTableSub[index & Mask3];
		}

		virtual void push(uint8_t value) {
			setByte(--SP().word, value);
		}

		virtual uint8_t pop() {
			return getByte(SP().word++);
		}

		//

		void memptrReference() {
			m_memory.ADDRESS() = MEMPTR();
			MEMPTR().word++;
		}

		virtual void getWordViaMemptr(register16_t& value) {
			memptrReference();
			value.low = getByte();
			m_memory.ADDRESS().word++;
			value.high = getByte();
		}

		virtual void setWordViaMemptr(register16_t value) {
			memptrReference();
			setByte(value.low);
			m_memory.ADDRESS().word++;
			setByte(value.high);
		}

		//

		void restart(uint8_t address) {
			MEMPTR().low = address;
			MEMPTR().high = 0;
			call();
		}

		bool callConditional(int condition) {
			fetchWord();
			if (condition)
				call();
			return condition != 0;
		}

		bool jumpConditional(int conditional) {
			fetchWord();
			if (conditional)
				jump();
			return conditional != 0;
		}

		bool returnConditional(int condition) {
			if (condition)
				ret();
			return condition != 0;
		}

		void jr(int8_t offset) {
			MEMPTR().word = PC().word + offset;
			jump();
		}

		bool jrConditional(int conditional) {
			auto offset = fetchByte();
			if (conditional)
				jr(offset);
			return conditional != 0;
		}

	private:
		std::array<opcode_decoded_t, 0x100> m_decodedOpcodes;
		register16_t sp;
	};
}