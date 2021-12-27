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

	private:
		DEFINE_PIN_ACTIVATOR_LOW(RFSH)
		DEFINE_PIN_ACTIVATOR_LOW(M1)
		DEFINE_PIN_ACTIVATOR_LOW(MREQ)
		DEFINE_PIN_ACTIVATOR_LOW(IORQ)
		DEFINE_PIN_ACTIVATOR_LOW(RD)
		DEFINE_PIN_ACTIVATOR_LOW(WR)

	public:
		struct refresh_t {

			bool high : 1;
			uint8_t variable : 7;

			constexpr refresh_t(const uint8_t value) noexcept
			: high(!!(value & Bit7)),
			  variable(value & Mask7)
			{ }

			constexpr operator uint8_t() const noexcept {
				return (high << 7) | variable;
			}

			constexpr auto& operator++() noexcept {
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

		bool operator==(const Z80& rhs) const;

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

		[[nodiscard]] const register16_t& AF() const noexcept final;
		[[nodiscard]] auto& AF() noexcept { return IntelProcessor::AF(); }
		[[nodiscard]] const register16_t& BC() const noexcept final;
		[[nodiscard]] auto& BC() noexcept { return IntelProcessor::BC(); }
		[[nodiscard]] const register16_t& DE() const noexcept final;
		[[nodiscard]] auto& DE() noexcept { return IntelProcessor::DE(); }
		[[nodiscard]] const register16_t& HL() const noexcept final;
		[[nodiscard]] auto& HL() noexcept { return IntelProcessor::HL(); }

		[[nodiscard]] const auto& IX() const noexcept { return m_ix; }
		NON_CONST_REGISTOR_ACCESSOR(IX);
		[[nodiscard]] auto& IXH() noexcept { return IX().high; }
		[[nodiscard]] auto& IXL() noexcept { return IX().low; }

		[[nodiscard]] const auto& IY() const noexcept { return m_iy; }
		NON_CONST_REGISTOR_ACCESSOR(IY);
		[[nodiscard]] auto& IYH() noexcept { return IY().high; }
		[[nodiscard]] auto& IYL() noexcept { return IY().low; }

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
		[[nodiscard]] constexpr auto& REFRESH() noexcept { return m_refresh; }
		[[nodiscard]] constexpr auto REFRESH() const noexcept { return m_refresh; }

		[[nodiscard]] constexpr auto& IV() noexcept { return iv; }
		[[nodiscard]] constexpr auto IV() const noexcept { return iv; }
		[[nodiscard]] constexpr auto& IM() noexcept { return m_interruptMode; }
		[[nodiscard]] constexpr auto IM() const noexcept { return m_interruptMode; }
		[[nodiscard]] constexpr auto& IFF1() noexcept { return m_iff1; }
		[[nodiscard]] constexpr auto IFF1() const noexcept { return m_iff1; }
		[[nodiscard]] constexpr auto& IFF2() noexcept { return m_iff2; }
		[[nodiscard]] constexpr auto IFF2() const noexcept { return m_iff2; }

		constexpr void exx() noexcept { m_registerSet ^= 1; }
		constexpr void exxAF() noexcept { m_accumulatorFlagsSet ^= 1; }

		[[nodiscard]] constexpr auto requestingIO() const noexcept { return lowered(IORQ()); }
		[[nodiscard]] constexpr auto requestingMemory() const noexcept { return lowered(MREQ()); }

		[[nodiscard]] constexpr auto requestingRead() const noexcept { return lowered(RD()); }
		[[nodiscard]] constexpr auto requestingWrite() const noexcept { return lowered(WR()); }

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

		void resetPrefixes() noexcept;

		[[nodiscard]] constexpr auto displaced() const noexcept { return m_prefixDD || m_prefixFD; }
		[[nodiscard]] uint16_t displacedAddress() noexcept;
		void fetchDisplacement();
		[[nodiscard]] uint8_t fetchOpCode();

		uint8_t readBusDataM1();

		typedef std::function<register16_t(void)> addresser_t;
		void loadAccumulatorIndirect(addresser_t addresser);
		void storeAccumulatorIndirect(addresser_t addresser);

		typedef std::function<uint8_t(void)> reader_t;
		void readInternalRegister(reader_t reader);

		[[nodiscard]] register16_t& HL2() noexcept;
		[[nodiscard]] register16_t& RP(int rp) noexcept;
		[[nodiscard]] register16_t& RP2(int rp) noexcept;

		[[nodiscard]] uint8_t R(int r);
		void R(int r, uint8_t value);
		void R2(int r, uint8_t value);

		[[nodiscard]] static uint8_t adjustHalfCarryAdd(uint8_t f, uint8_t before, uint8_t value, int calculation) noexcept;
		[[nodiscard]] static uint8_t adjustHalfCarrySub(uint8_t f, uint8_t before, uint8_t value, int calculation) noexcept;
		[[nodiscard]] static uint8_t adjustOverflowAdd(uint8_t f, uint8_t before, uint8_t value, uint8_t calculation) noexcept;
		[[nodiscard]] static uint8_t adjustOverflowAdd(uint8_t f, int beforeNegative, int valueNegative, int afterNegative) noexcept;
		[[nodiscard]] static uint8_t adjustOverflowSub(uint8_t f, uint8_t before, uint8_t value, uint8_t calculation) noexcept;
		[[nodiscard]] static uint8_t adjustOverflowSub(uint8_t f, int beforeNegative, int valueNegative, int afterNegative) noexcept;

		[[nodiscard]] static bool convertCondition(uint8_t f, int flag) noexcept;

		static uint8_t subtract(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0) noexcept;

		void executeCB(int x, int y, int z);
		void executeED(int x, int y, int z, int p, int q);
		void executeOther(int x, int y, int z, int p, int q);

		[[nodiscard]] static uint8_t increment(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t decrement(uint8_t& f, uint8_t operand) noexcept;

		void di() noexcept;
		void ei() noexcept;

		void retn();
		void reti();

		void returnConditionalFlag(uint8_t f, int flag);
		void jrConditionalFlag(uint8_t f, int flag);
		void callConditionalFlag(uint8_t f, int flag);
		void jumpConditionalFlag(uint8_t f, int flag);

		[[nodiscard]] register16_t sbc(uint8_t& f, register16_t operand, register16_t value);
		[[nodiscard]] register16_t adc(uint8_t& f, register16_t operand, register16_t value);
		[[nodiscard]] register16_t add(uint8_t& f, register16_t operand, register16_t value, int carry = 0);

		[[nodiscard]] static uint8_t add(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0) noexcept;
		[[nodiscard]] static uint8_t adc(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
		[[nodiscard]] static uint8_t sub(uint8_t& f, uint8_t operand, uint8_t value, int carry = 0) noexcept;
		[[nodiscard]] static uint8_t sbc(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
		[[nodiscard]] static uint8_t andr(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
		[[nodiscard]] static uint8_t xorr(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
		[[nodiscard]] static uint8_t orr(uint8_t& f, uint8_t operand, uint8_t value) noexcept;
		static void compare(uint8_t& f, uint8_t operand, uint8_t value) noexcept;

		[[nodiscard]] static uint8_t rlc(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t rrc(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t rl(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t rr(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t sla(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t sra(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t sll(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t srl(uint8_t& f, uint8_t operand) noexcept;

		static void bit(uint8_t& f, int n, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t res(int n, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t set(int n, uint8_t operand) noexcept;

		[[nodiscard]] static uint8_t daa(uint8_t& f, uint8_t operand) noexcept;

		static void scf(uint8_t& f, uint8_t operand) noexcept;
		static void ccf(uint8_t& f, uint8_t operand) noexcept;
		[[nodiscard]] static uint8_t cpl(uint8_t& f, uint8_t operand) noexcept;

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

		[[nodiscard]] uint8_t neg(uint8_t& f, uint8_t operand) noexcept;

		void rrd(uint8_t& f, register16_t address, uint8_t& update);
		void rld(uint8_t& f, register16_t address, uint8_t& update);

		void portWrite(uint8_t port);
		void portWrite();

		uint8_t portRead(uint8_t port);
		uint8_t portRead();
	};
}