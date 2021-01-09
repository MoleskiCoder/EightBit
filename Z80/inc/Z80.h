#pragma once

#include <cstdint>
#include <array>
#include <functional>

#include <IntelProcessor.h>
#include <EventArgs.h>
#include <Signal.h>
#include <Register.h>

namespace EightBit {

	class Bus;

	class Z80 final : public IntelProcessor {
	public:
		struct refresh_t {

			bool high : 1;
			uint8_t variable : 7;

			refresh_t(const uint8_t value)
			: high(!!(value & Bit7)),
			  variable(value & Mask7)
			{ }

			operator uint8_t() const {
				return (high << 7) | variable;
			}

			auto& operator++() {
				++variable;
				return *this;
			}
		};

		enum StatusBits {
			SF = Bit7,
			ZF = Bit6,
			YF = Bit5,
			HC = Bit4,
			XF = Bit3,
			PF = Bit2,
			VF = Bit2,
			NF = Bit1,
			CF = Bit0,
		};

		Z80(Bus& bus);

		Signal<Z80> ExecutingInstruction;
		Signal<Z80> ExecutedInstruction;

		Signal<EventArgs> ReadingMemory;
		Signal<EventArgs> ReadMemory;

		Signal<EventArgs> WritingMemory;
		Signal<EventArgs> WrittenMemory;

		Signal<EventArgs> ReadingIO;
		Signal<EventArgs> ReadIO;

		Signal<EventArgs> WritingIO;
		Signal<EventArgs> WrittenIO;

		int execute() final;
		int step() final;

		[[nodiscard]] register16_t& AF() final;
		[[nodiscard]] register16_t& BC() final;
		[[nodiscard]] register16_t& DE() final;
		[[nodiscard]] register16_t& HL() final;

		[[nodiscard]] auto& IX() { return m_ix; }
		[[nodiscard]] auto& IXH() { return IX().high; }
		[[nodiscard]] auto& IXL() { return IX().low; }

		[[nodiscard]] auto& IY() { return m_iy; }
		[[nodiscard]] auto& IYH() { return IY().high; }
		[[nodiscard]] auto& IYL() { return IY().low; }

		// ** From the Z80 CPU User Manual
		// Memory Refresh(R) Register.The Z80 CPU contains a memory refresh counter,
		// enabling dynamic memories to be used with the same ease as static memories.Seven bits
		// of this 8-bit register are automatically incremented after each instruction fetch.The eighth
		// bit remains as programmed, resulting from an LD R, A instruction. The data in the refresh
		// counter is sent out on the lower portion of the address bus along with a refresh control
		// signal while the CPU is decoding and executing the fetched instruction. This mode of refresh
		// is transparent to the programmer and does not slow the CPU operation.The programmer
		// can load the R register for testing purposes, but this register is normally not used by the
		// programmer. During refresh, the contents of the I Register are placed on the upper eight
		// bits of the address bus.
		[[nodiscard]] auto& REFRESH() { return m_refresh; }

		[[nodiscard]] auto& IV() { return iv; }
		[[nodiscard]] auto& IM() { return m_interruptMode; }
		[[nodiscard]] auto& IFF1() { return m_iff1; }
		[[nodiscard]] auto& IFF2() { return m_iff2; }

		void exx() { m_registerSet ^= 1; }
		void exxAF() { m_accumulatorFlagsSet ^= 1; }

		[[nodiscard]] auto requestingIO() const { return lowered(IORQ()); }
		[[nodiscard]] auto requestingMemory() const { return lowered(MREQ()); }

		[[nodiscard]] auto requestingRead() const { return lowered(RD()); }
		[[nodiscard]] auto requestingWrite() const { return lowered(WR()); }

		// ** From the Z80 CPU User Manual
		// RFSH.Refresh(output, active Low). RFSH, together with MREQ, indicates that the lower
		// seven bits of the system’s address bus can be used as a refresh address to the system’s
		// dynamic memories.
		DECLARE_PIN_OUTPUT(RFSH)

		DECLARE_PIN_INPUT(NMI)
		DECLARE_PIN_OUTPUT(M1)
		DECLARE_PIN_OUTPUT(MREQ)
		DECLARE_PIN_OUTPUT(IORQ)
		DECLARE_PIN_OUTPUT(RD)
		DECLARE_PIN_OUTPUT(WR)

	protected:
		void handleRESET() final;
		void handleINT() final;

		void pushWord(register16_t destination) final;

		void memoryWrite() final;
		uint8_t memoryRead() final;

		void busWrite() final;
		uint8_t busRead() final;

		void jr(int8_t offset) final;
		int jrConditional(int condition) final;

	private:

		DEFINE_PIN_ACTIVATOR_LOW(RFSH)
		DEFINE_PIN_ACTIVATOR_LOW(M1)
		DEFINE_PIN_ACTIVATOR_LOW(MREQ)
		DEFINE_PIN_ACTIVATOR_LOW(IORQ)
		DEFINE_PIN_ACTIVATOR_LOW(RD)
		DEFINE_PIN_ACTIVATOR_LOW(WR)

		enum { BC_IDX, DE_IDX, HL_IDX };

		std::array<std::array<register16_t, 3>, 2> m_registers;
		int m_registerSet = 0;

		std::array<register16_t, 2> m_accumulatorFlags;
		int m_accumulatorFlagsSet = 0;

		register16_t m_ix = 0xffff;
		register16_t m_iy = 0xffff;

		refresh_t m_refresh = 0x7f;

		uint8_t iv = 0xff;
		int m_interruptMode = 0;
		bool m_iff1 = false;
		bool m_iff2 = false;

		bool m_prefixCB = false;
		bool m_prefixDD = false;
		bool m_prefixED = false;
		bool m_prefixFD = false;

		int8_t m_displacement = 0;

		void handleNMI();

		void resetPrefixes();

		[[nodiscard]] auto displaced() const { return m_prefixDD || m_prefixFD; }
		[[nodiscard]] uint16_t displacedAddress();
		void fetchDisplacement();
		[[nodiscard]] uint8_t fetchOpCode();

		uint8_t readBusDataM1();

		typedef std::function<register16_t(void)> addresser_t;
		void loadAccumulatorIndirect(addresser_t addresser);
		void storeAccumulatorIndirect(addresser_t addresser);

		typedef std::function<uint8_t(void)> reader_t;
		void readInternalRegister(reader_t reader);

		[[nodiscard]] register16_t& HL2();
		[[nodiscard]] register16_t& RP(int rp);
		[[nodiscard]] register16_t& RP2(int rp);

		[[nodiscard]] uint8_t R(int r);
		void R(int r, uint8_t value);
		void R2(int r, uint8_t value);

		[[nodiscard]] static uint8_t adjustHalfCarryAdd(uint8_t f, uint8_t before, uint8_t value, int calculation);
		[[nodiscard]] static uint8_t adjustHalfCarrySub(uint8_t f, uint8_t before, uint8_t value, int calculation);
		[[nodiscard]] static uint8_t adjustOverflowAdd(uint8_t f, uint8_t before, uint8_t value, uint8_t calculation);
		[[nodiscard]] static uint8_t adjustOverflowAdd(uint8_t f, int beforeNegative, int valueNegative, int afterNegative);
		[[nodiscard]] static uint8_t adjustOverflowSub(uint8_t f, uint8_t before, uint8_t value, uint8_t calculation);
		[[nodiscard]] static uint8_t adjustOverflowSub(uint8_t f, int beforeNegative, int valueNegative, int afterNegative);

		[[nodiscard]] static bool convertCondition(uint8_t f, int flag);

		static uint8_t subtract(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);

		void executeCB(int x, int y, int z);
		void executeED(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		[[nodiscard]] static uint8_t increment(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t decrement(uint8_t& f, uint8_t operand);

		void di();
		void ei();

		void retn();
		void reti();

		void returnConditionalFlag(uint8_t f, int flag);
		void jrConditionalFlag(uint8_t f, int flag);
		void callConditionalFlag(uint8_t f, int flag);
		void jumpConditionalFlag(uint8_t f, int flag);

		[[nodiscard]] register16_t sbc(uint8_t& f, register16_t operand, register16_t value);
		[[nodiscard]] register16_t adc(uint8_t& f, register16_t operand, register16_t value);
		[[nodiscard]] register16_t add(uint8_t& f, register16_t operand, register16_t value, int carry = 0);

		[[nodiscard]] static uint8_t add(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);
		[[nodiscard]] static uint8_t adc(uint8_t& f, uint8_t operand, uint8_t value);
		[[nodiscard]] static uint8_t sub(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0);
		[[nodiscard]] static uint8_t sbc(uint8_t& f, uint8_t operand, uint8_t value);
		[[nodiscard]] static uint8_t andr(uint8_t& f, uint8_t operand, uint8_t value);
		[[nodiscard]] static uint8_t xorr(uint8_t& f, uint8_t operand, uint8_t value);
		[[nodiscard]] static uint8_t orr(uint8_t& f, uint8_t operand, uint8_t value);
		static void compare(uint8_t& f, uint8_t operand, uint8_t value);

		[[nodiscard]] static uint8_t rlc(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t rrc(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t rl(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t rr(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t sla(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t sra(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t sll(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t srl(uint8_t& f, uint8_t operand);

		static void bit(uint8_t& f, int n, uint8_t operand);
		[[nodiscard]] static uint8_t res(int n, uint8_t operand);
		[[nodiscard]] static uint8_t set(int n, uint8_t operand);

		[[nodiscard]] static uint8_t daa(uint8_t& f, uint8_t operand);

		static void scf(uint8_t& f, uint8_t operand);
		static void ccf(uint8_t& f, uint8_t operand);
		[[nodiscard]] static uint8_t cpl(uint8_t& f, uint8_t operand);

		void xhtl(register16_t& exchange);

		void blockCompare(uint8_t& f, uint8_t value, register16_t source, register16_t& counter);

		void cpi(uint8_t& f, uint8_t value);
		[[nodiscard]] bool cpir(uint8_t& f, uint8_t value);

		void cpd(uint8_t& f, uint8_t value);
		[[nodiscard]] bool cpdr(uint8_t& f, uint8_t value);

		void blockLoad(uint8_t& f, uint8_t a, register16_t source, register16_t destination, register16_t& counter);

		void ldi(uint8_t& f, uint8_t a);
		[[nodiscard]] bool ldir(uint8_t& f, uint8_t a);

		void ldd(uint8_t& f, uint8_t a);
		[[nodiscard]] bool lddr(uint8_t& f, uint8_t a);

		void blockIn(register16_t& source, register16_t destination);

		void ini();
		[[nodiscard]] bool inir();

		void ind();
		[[nodiscard]] bool indr();

		void blockOut(register16_t source, register16_t& destination);

		void outi();
		[[nodiscard]] bool otir();

		void outd();
		[[nodiscard]] bool otdr();

		[[nodiscard]] uint8_t neg(uint8_t& f, uint8_t operand);

		void rrd(uint8_t& f, register16_t address, uint8_t& update);
		void rld(uint8_t& f, register16_t address, uint8_t& update);

		void portWrite(uint8_t port);
		void portWrite();

		uint8_t portRead(uint8_t port);
		uint8_t portRead();
	};
}