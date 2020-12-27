#include "stdafx.h"
#include "Z80.h"

// based on http://www.z80.info/decoding.htm

EightBit::Z80::Z80(Bus& bus)
: IntelProcessor(bus) {
	RaisedPOWER.connect([this](EventArgs) {

		raiseM1();
		raiseRFSH();
		raiseIORQ();
		raiseMREQ();
		raiseRD();
		raiseWR();

		di();
		IM() = 0;

		REFRESH() = 0;
		IV() = Mask8;

		exxAF();
		exx();

		AF() = IX() = IY() = BC() = DE() = HL() = Mask16;

		m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
	});

	RaisedM1.connect([this](EventArgs) {
		++REFRESH();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(NMI, Z80);
DEFINE_PIN_LEVEL_CHANGERS(M1, Z80);
DEFINE_PIN_LEVEL_CHANGERS(RFSH, Z80);
DEFINE_PIN_LEVEL_CHANGERS(MREQ, Z80);
DEFINE_PIN_LEVEL_CHANGERS(IORQ, Z80);
DEFINE_PIN_LEVEL_CHANGERS(RD, Z80);
DEFINE_PIN_LEVEL_CHANGERS(WR, Z80);

void EightBit::Z80::memoryWrite() {

	class _Writer final {
		Z80& m_parent;
	public:
		_Writer(Z80& parent) : m_parent(parent) {
			m_parent.WritingMemory.fire(EventArgs::empty());
			m_parent.tick(2);
			m_parent.lowerMREQ();
		}

		~_Writer() {
			m_parent.raiseMREQ();
			m_parent.WrittenMemory.fire(EventArgs::empty());
		}
	};

	_Writer writer(*this);
	IntelProcessor::memoryWrite();
}

uint8_t EightBit::Z80::memoryRead() {

	class _Reader final {
		Z80& m_parent;
	public:
		_Reader(Z80& parent) : m_parent(parent) {
			m_parent.ReadingMemory.fire(EventArgs::empty());
			m_parent.tick(2);
			m_parent.lowerMREQ();
		}

		~_Reader() {
			m_parent.raiseMREQ();
			m_parent.ReadMemory.fire(EventArgs::empty());
		}
	};

	_Reader reader(*this);
	return IntelProcessor::memoryRead();
}

void EightBit::Z80::busWrite() {
	tick();
	_ActivateWR writer(*this);
	IntelProcessor::busWrite();
}

uint8_t EightBit::Z80::busRead() {
	tick();
	_ActivateRD reader(*this);
	return IntelProcessor::busRead();
}

void EightBit::Z80::handleRESET() {
	IntelProcessor::handleRESET();
	di();
	IV() = REFRESH() = 0;
	SP().word = AF().word = Mask16;
	tick(3);
}

void EightBit::Z80::handleNMI() {
	raiseNMI();
	raiseHALT();
	IFF2() = IFF1();
	IFF1() = false;
	readBusDataM1();
	restart(0x66);
}

void EightBit::Z80::handleINT() {
	IntelProcessor::handleINT();
	tick(2);	// 2 extra clock cycles introduced to allow the bus to settle
	uint8_t data;
	{
		_ActivateIORQ iorq(*this);
		data = readBusDataM1();
	}
	di();
	switch (IM()) {
	case 0:		// i8080 equivalent
		IntelProcessor::execute(data);
		break;
	case 1:
		restart(7 << 3);
		break;
	case 2:
		tick(7);	// How long to allow fetching data from the device...
		call(MEMPTR() = { data, IV() });
		break;
	default:
		UNREACHABLE;
	}
}

void EightBit::Z80::di() {
	IFF1() = IFF2() = false;
}

void EightBit::Z80::ei() {
	IFF1() = IFF2() = true;
}

uint8_t EightBit::Z80::increment(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF);
	const uint8_t result = operand + 1;
	f = adjustSZXY<Z80>(f, result);
	f = setBit(f, VF, result == Bit7);
	f = clearBit(f, HC, lowNibble(result));
	return result;
}

uint8_t EightBit::Z80::decrement(uint8_t& f, const uint8_t operand) {
	f = setBit(f, NF);
	f = clearBit(f, HC, lowNibble(operand));
	const uint8_t result = operand - 1;
	f = adjustSZXY<Z80>(f, result);
	f = setBit(f, VF, result == Mask7);
	return result;
}

bool EightBit::Z80::convertCondition(const uint8_t f, int flag) {
	ASSUME(flag >= 0);
	ASSUME(flag <= 7);
	switch (flag) {
	case 0:
		return !(f & ZF);
	case 1:
		return f & ZF;
	case 2:
		return !(f & CF);
	case 3:
		return f & CF;
	case 4:
		return !(f & PF);
	case 5:
		return f & PF;
	case 6:
		return !(f & SF);
	case 7:
		return f & SF;
	default:
		UNREACHABLE;
	}
}

void EightBit::Z80::returnConditionalFlag(const uint8_t f, const int flag) {
	tick();
	if (convertCondition(f, flag))
		ret();
}

void EightBit::Z80::jrConditionalFlag(const uint8_t f, const int flag) {
	jrConditional(convertCondition(f, flag));
}

void EightBit::Z80::jumpConditionalFlag(const uint8_t f, const int flag) {
	jumpConditional(convertCondition(f, flag));
}

void EightBit::Z80::callConditionalFlag(const uint8_t f, const int flag) {
	callConditional(convertCondition(f, flag));
}

void EightBit::Z80::retn() {
	ret();
	IFF1() = IFF2();
}

void EightBit::Z80::reti() {
	retn();
}

EightBit::register16_t EightBit::Z80::sbc(uint8_t& f, const register16_t operand, const register16_t value) {

	const auto subtraction = operand.word - value.word - (f & CF);
	const register16_t result = subtraction;

	f = setBit(f, NF);
	f = clearBit(f, ZF, result.word);
	f = setBit(f, CF, subtraction & Bit16);
	f = adjustHalfCarrySub(f, operand.high, value.high, result.high);
	f = adjustXY<Z80>(f, result.high);

	const auto beforeNegative = operand.high & SF;
	const auto valueNegative = value.high & SF;
	const auto afterNegative = result.high & SF;

	f = setBit(f, SF, afterNegative);
	f = adjustOverflowSub(f, beforeNegative, valueNegative, afterNegative);

	MEMPTR() = operand + 1;

	tick(7);
	return result;
}

EightBit::register16_t EightBit::Z80::adc(uint8_t& f, const register16_t operand, const register16_t value) {

	const auto result = add(f, operand, value, f & CF);
	f = clearBit(f, ZF, result.word);

	const auto beforeNegative = operand.high & SF;
	const auto valueNegative = value.high & SF;
	const auto afterNegative = result.high & SF;

	f = setBit(f, SF, afterNegative);
	f = adjustOverflowAdd(f, beforeNegative, valueNegative, afterNegative);

	return result;
}

EightBit::register16_t EightBit::Z80::add(uint8_t& f, const register16_t operand, const register16_t value, int carry) {

	const int addition = operand.word + value.word + carry;
	const register16_t result = addition;

	f = clearBit(f, NF);
	f = setBit(f, CF, addition & Bit16);
	f = adjustHalfCarryAdd(f, operand.high, value.high, result.high);
	f = adjustXY<Z80>(f, result.high);

	MEMPTR() = operand + 1;

	tick(7);
	return result;
}

uint8_t EightBit::Z80::add(uint8_t& f, const uint8_t operand, const uint8_t value, const int carry) {

	const register16_t addition = operand + value + carry;
	const auto result = addition.low;

	f = adjustHalfCarryAdd(f, operand, value, result);
	f = adjustOverflowAdd(f, operand, value, result);

	f = clearBit(f, NF);
	f = setBit(f, CF, addition.high & CF);
	f = adjustSZXY<Z80>(f, result);

	return result;
}

uint8_t EightBit::Z80::adc(uint8_t& f, const uint8_t operand, const uint8_t value) {
	return add(f, operand, value, f & CF);
}

uint8_t EightBit::Z80::subtract(uint8_t& f, const uint8_t operand, const uint8_t value, const int carry) {

	const register16_t subtraction = operand - value - carry;
	const auto result = subtraction.low;

	f = adjustHalfCarrySub(f, operand, value, result);
	f = adjustOverflowSub(f, operand, value, result);

	f = setBit(f, NF);
	f = setBit(f, CF, subtraction.high & CF);
	f = adjustSZ<Z80>(f, result);

	return result;
}

uint8_t EightBit::Z80::sub(uint8_t& f, const uint8_t operand, const uint8_t value, const int carry) {
	const auto subtraction = subtract(f, operand, value, carry);
	f = adjustXY<Z80>(f, subtraction);
	return subtraction;
}

uint8_t EightBit::Z80::sbc(uint8_t& f, const uint8_t operand, const uint8_t value) {
	return sub(f, operand, value, f & CF);
}

uint8_t EightBit::Z80::andr(uint8_t& f, const uint8_t operand, const uint8_t value) {
	f = setBit(f, HC);
	f = clearBit(f, CF | NF);
	const uint8_t result = operand & value;
	f = adjustSZPXY<Z80>(f, result);
	return result;
}

uint8_t EightBit::Z80::xorr(uint8_t& f, const uint8_t operand, const uint8_t value) {
	f = clearBit(f, HC | CF | NF);
	const uint8_t result = operand ^ value;
	f = adjustSZPXY<Z80>(f, result);
	return result;
}

uint8_t EightBit::Z80::orr(uint8_t& f, const uint8_t operand, const uint8_t value) {
	f = clearBit(f, HC | CF | NF);
	const uint8_t result = operand | value;
	f = adjustSZPXY<Z80>(f, result);
	return result;
}

void EightBit::Z80::compare(uint8_t& f, uint8_t operand, const uint8_t value) {
	subtract(f, operand, value);
	f = adjustXY<Z80>(f, value);
}

uint8_t EightBit::Z80::rlc(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF | HC);
	const auto carry = operand & Bit7;
	f = setBit(f, CF, carry);
	const uint8_t result = (operand << 1) | (carry >> 7);
	f = adjustXY<Z80>(f, result);
	return result;
}

uint8_t EightBit::Z80::rrc(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF | HC);
	const auto carry = operand & Bit0;
	f = setBit(f, CF, carry);
	const uint8_t result = (operand >> 1) | (carry << 7);
	f = adjustXY<Z80>(f, result);
	return result;
}

uint8_t EightBit::Z80::rl(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF | HC);
	const auto carry = f & CF;
	f = setBit(f, CF, operand & Bit7);
	const uint8_t result = (operand << 1) | carry;
	f = adjustXY<Z80>(f, result);
	return result;
}

uint8_t EightBit::Z80::rr(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF | HC);
	const auto carry = f & CF;
	f = setBit(f, CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | (carry << 7);
	f = adjustXY<Z80>(f, result);
	return result;
}

//

uint8_t EightBit::Z80::sla(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF | HC);
	f = setBit(f, CF, operand & Bit7);
	const uint8_t result = operand << 1;
	f = adjustXY<Z80>(f, result);
	return result;
}

uint8_t EightBit::Z80::sra(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF | HC);
	f = setBit(f, CF, operand & Bit0);
	const uint8_t result = (operand >> 1) | (operand & Bit7);
	f = adjustXY<Z80>(f, result);
	return result;
}

uint8_t EightBit::Z80::sll(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF | HC);
	f = setBit(f, CF, operand & Bit7);
	const uint8_t result = (operand << 1) | Bit0;
	f = adjustXY<Z80>(f, result);
	return result;
}

uint8_t EightBit::Z80::srl(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF | HC);
	f = setBit(f, CF, operand & Bit0);
	const uint8_t result = (operand >> 1) & ~Bit7;
	f = adjustXY<Z80>(f, result);
	f = setBit(f, ZF, result);
	return result;
}

void EightBit::Z80::bit(uint8_t& f, const int n, const uint8_t operand) {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	f = setBit(f, HC);
	f = clearBit(f, NF);
	const auto discarded = operand & Chip::bit(n);
	f = adjustSZ<Z80>(f, discarded);
	f = clearBit(f, PF, discarded);
}

uint8_t EightBit::Z80::res(const int n, const uint8_t operand) {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	return clearBit(operand, Chip::bit(n));
}

uint8_t EightBit::Z80::set(const int n, const uint8_t operand) {
	ASSUME(n >= 0);
	ASSUME(n <= 7);
	return setBit(operand, Chip::bit(n));
}

uint8_t EightBit::Z80::neg(uint8_t& f, uint8_t operand) {

	f = setBit(f, PF, operand == Bit7);
	f = setBit(f, CF, operand);
	f = setBit(f, NF);

	const uint8_t result = (~operand + 1);	// two's complement

	f = adjustHalfCarrySub(f, 0U, operand, result);
	f = adjustOverflowSub(f, 0U, operand, result);

	f = adjustSZXY<Z80>(f, result);

	return result;
}

uint8_t EightBit::Z80::daa(uint8_t& f, uint8_t operand) {

	const auto lowAdjust = (f & HC) || (lowNibble(operand) > 9);
	const auto highAdjust = (f & CF) || (operand > 0x99);

	auto updated = operand;
	if (f & NF) {
		if (lowAdjust)
			updated -= 6;
		if (highAdjust)
			updated -= 0x60;
	} else {
		if (lowAdjust)
			updated += 6;
		if (highAdjust)
			updated += 0x60;
	}

	f = (f & (CF | NF)) | (operand > 0x99 ? CF : 0) | ((operand ^ updated) & HC);

	f = adjustSZPXY<Z80>(f, updated);

	return updated;
}

uint8_t EightBit::Z80::cpl(uint8_t& f, const uint8_t operand) {
	f = setBit(f, HC | NF);
	const uint8_t result = ~operand;
	f = adjustXY<Z80>(f, result);
	return result;
}

void EightBit::Z80::scf(uint8_t& f, const uint8_t operand) {
	f = setBit(f, CF);
	f = clearBit(f, HC | NF);
	f = adjustXY<Z80>(f, operand);
}

void EightBit::Z80::ccf(uint8_t& f, const uint8_t operand) {
	f = clearBit(f, NF);
	const auto carry = f & CF;
	f = setBit(f, HC, carry);
	f = clearBit(f, CF, carry);
	f = adjustXY<Z80>(f, operand);
}

void EightBit::Z80::xhtl(register16_t& exchange) {
	MEMPTR().low = IntelProcessor::memoryRead(SP());
	++BUS().ADDRESS();
	MEMPTR().high = memoryRead();
	tick();
	IntelProcessor::memoryWrite(exchange.high);
	exchange.high = MEMPTR().high;
	--BUS().ADDRESS();
	IntelProcessor::memoryWrite(exchange.low);
	exchange.low = MEMPTR().low;
	tick(2);
}

void EightBit::Z80::blockCompare(uint8_t& f, const uint8_t value, const register16_t source, register16_t& counter) {

	const auto contents = IntelProcessor::memoryRead(source);
	uint8_t result = value - contents;

	f = setBit(f, PF, --counter.word);

	f = adjustSZ<Z80>(f, result);
	f = adjustHalfCarrySub(f, value, contents, result);
	f = setBit(f, NF);

	result -= ((f & HC) >> 4);

	f = setBit(f, YF, result & Bit1);
	f = setBit(f, XF, result & Bit3);

	tick(5);
}

void EightBit::Z80::cpi(uint8_t& f, uint8_t value) {
	blockCompare(f, value, HL()++, BC());
	++MEMPTR();
}

void EightBit::Z80::cpd(uint8_t& f, uint8_t value) {
	blockCompare(f, value, HL()--, BC());
	--MEMPTR();
}

bool EightBit::Z80::cpir(uint8_t& f, uint8_t value) {
	cpi(f, value);
	return (f & PF) && !(f & ZF);	// See CPI
}

bool EightBit::Z80::cpdr(uint8_t& f, uint8_t value) {
	cpd(f, value);
	return (f & PF) && !(f & ZF);	// See CPD
}

void EightBit::Z80::blockLoad(uint8_t& f, const uint8_t a, const register16_t source, const register16_t destination, register16_t& counter) {
	const auto value = IntelProcessor::memoryRead(source);
	IntelProcessor::memoryWrite(destination, value);
	const auto xy = a + value;
	f = setBit(f, XF, xy & Bit3);
	f = setBit(f, YF, xy & Bit1);
	f = clearBit(f, NF | HC);
	f = setBit(f, PF, --counter.word);
	tick(2);
}

void EightBit::Z80::ldd(uint8_t& f, const uint8_t a) {
	blockLoad(f, a, HL()--, DE()--, BC());
}

void EightBit::Z80::ldi(uint8_t& f, const uint8_t a) {
	blockLoad(f, a, HL()++, DE()++, BC());
}

bool EightBit::Z80::ldir(uint8_t& f, const uint8_t a) {
	ldi(f, a);
	return !!(f & PF);		// See LDI
}

bool EightBit::Z80::lddr(uint8_t& f, const uint8_t a) {
	ldd(f, a);
	return !!(f & PF);		// See LDD
}

void EightBit::Z80::blockIn(register16_t& source, const register16_t destination) {
	MEMPTR() = BUS().ADDRESS() = source;
	tick();
	const auto value = portRead();
	IntelProcessor::memoryWrite(destination, value);
	source.high = decrement(F(), source.high);
	F() = setBit(F(), NF);
}

void EightBit::Z80::ini() {
	blockIn(BC(), HL()++);
	++MEMPTR();
}

void EightBit::Z80::ind() {
	blockIn(BC(), HL()--);
	--MEMPTR();
}

bool EightBit::Z80::inir() {
	ini();
	return !(F() & ZF);	// See INI
}

bool EightBit::Z80::indr() {
	ind();
	return !(F() & ZF);	// See IND
}

void EightBit::Z80::blockOut(const register16_t source, register16_t& destination) {
	tick();
	const auto value = IntelProcessor::memoryRead(source);
	destination.high = decrement(F(), destination.high);
	BUS().ADDRESS() = destination;
	portWrite();
	MEMPTR() = destination;
	F() = setBit(F(), NF, value & Bit7);
	F() = setBit(F(), HC | CF, (L() + value) > 0xff);
	F() = adjustParity<Z80>(F(), ((value + L()) & Mask3) ^ B());
}

void EightBit::Z80::outi() {
	blockOut(HL()++, BC());
	++MEMPTR();
}

void EightBit::Z80::outd() {
	blockOut(HL()--, BC());
	--MEMPTR();
}

bool EightBit::Z80::otir() {
	outi();
	return !(F() & ZF);	// See OUTI
}

bool EightBit::Z80::otdr() {
	outd();
	return !(F() & ZF);	// See OUTD
}

void EightBit::Z80::rrd(uint8_t& f, register16_t address, uint8_t& update) {
	(MEMPTR() = BUS().ADDRESS() = address)++;
	const auto memory = memoryRead();
	tick(4);
	IntelProcessor::memoryWrite(promoteNibble(update) | highNibble(memory));
	update = higherNibble(update) | lowerNibble(memory);
	f = adjustSZPXY<Z80>(f, update);
	f = clearBit(f, NF | HC);
}

void EightBit::Z80::rld(uint8_t& f, register16_t address, uint8_t& update) {
	(MEMPTR() = BUS().ADDRESS() = address)++;
	const auto memory = memoryRead();
	tick(4);
	IntelProcessor::memoryWrite(promoteNibble(memory) | lowNibble(update));
	update = higherNibble(update) | highNibble(memory);
	f = adjustSZPXY<Z80>(f, update);
	f = clearBit(f, NF | HC);
}

void EightBit::Z80::portWrite(const uint8_t port) {
	MEMPTR() = BUS().ADDRESS() = { port, A() };
	BUS().DATA() = A();
	portWrite();
	++MEMPTR().low;
}

void EightBit::Z80::portWrite() {

	class _Writer final {
		Z80& m_parent;
	public:
		_Writer(Z80& parent) : m_parent(parent) {
			m_parent.WritingIO.fire(EventArgs::empty());
		}

		~_Writer() {
			m_parent.WrittenIO.fire(EventArgs::empty());
			m_parent.tick(3);
		}
	};

	_Writer writer(*this);
	_ActivateIORQ iorq(*this);
	busWrite();
}

uint8_t EightBit::Z80::portRead(const uint8_t port) {
	MEMPTR() = BUS().ADDRESS() = { port, A() };
	++MEMPTR().low;
	return portRead();
}

uint8_t EightBit::Z80::portRead() {

	class _Reader final {
		Z80& m_parent;
	public:
		_Reader(Z80& parent) : m_parent(parent) {
			m_parent.ReadingIO.fire(EventArgs::empty());
		}

		~_Reader() {
			m_parent.ReadIO.fire(EventArgs::empty());
			m_parent.tick(3);
		}
	};

	_Reader reader(*this);
	_ActivateIORQ iorq(*this);
	return busRead();
}

//

uint16_t EightBit::Z80::displacedAddress() {
	assert(m_displaced);
	return MEMPTR().word = (m_prefixDD ? IX() : IY()).word + m_displacement;
}

void EightBit::Z80::fetchDisplacement() {
	m_displacement = fetchByte();
}

// ** From the Z80 CPU User Manual

// Figure 5 depicts the timing during an M1 (op code fetch) cycle. The Program Counter is
// placed on the address bus at the beginning of the M1 cycle. One half clock cycle later, the
// MREQ signal goes active. At this time, the address to memory has had time to stabilize so
// that the falling edge of MREQ can be used directly as a chip enable clock to dynamic
// memories. The RD line also goes active to indicate that the memory read data should be
// enabled onto the CPU data bus. The CPU samples the data from the memory space on the
// data bus with the rising edge of the clock of state T3, and this same edge is used by the
// CPU to turn off the RD and MREQ signals. As a result, the data is sampled by the CPU
// before the RD signal becomes inactive. Clock states T3 and T4 of a fetch cycle are used to
// refresh dynamic memories. The CPU uses this time to decode and execute the fetched
// instruction so that no other concurrent operation can be performed.

// When a software HALT instruction is executed, the CPU executes NOPs until an interrupt
// is received(either a nonmaskable or a maskable interrupt while the interrupt flip-flop is
// enabled). The two interrupt lines are sampled with the rising clock edge during each T4
// state as depicted in Figure 11.If a nonmaskable interrupt is received or a maskable interrupt
// is received and the interrupt enable flip-flop is set, then the HALT state is exited on
// the next rising clock edge.The following cycle is an interrupt acknowledge cycle corresponding
// to the type of interrupt that was received.If both are received at this time, then
// the nonmaskable interrupt is acknowledged because it is the highest priority.The purpose
// of executing NOP instructions while in the HALT state is to keep the memory refresh signals
// active.Each cycle in the HALT state is a normal M1(fetch) cycle except that the data
// received from the memory is ignored and an NOP instruction is forced internally to the
// CPU.The HALT acknowledge signal is active during this time indicating that the processor
// is in the HALT state
uint8_t EightBit::Z80::fetchOpCode() {
	tick();
	uint8_t returned;
	{
		_ActivateM1 m1(*this);
		const auto halted = lowered(HALT());
		returned = IntelProcessor::memoryRead(PC());
		if (UNLIKELY(halted))
			returned = 0;	// NOP
		else
			PC()++;
	}
	BUS().ADDRESS() = { REFRESH(), IV() };
	{
		_ActivateRFSH rfsh(*this);
		_ActivateMREQ mreq(*this);
	}
	return returned;
}

int EightBit::Z80::step() {
	resetCycles();
	ExecutingInstruction.fire(*this);
	if (LIKELY(powered())) {
		m_displaced = m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;
		bool handled = false;
		if (lowered(RESET())) {
			handleRESET();
			handled = true;
		} else if (lowered(NMI())) {
			handleNMI();
			handled = true;
		} else if (lowered(INT())) {
			raiseINT();
			raiseHALT();
			if (IFF1()) {
				handleINT();
				handled = true;
			}
		}
		if (!handled)
			IntelProcessor::execute(fetchOpCode());
	}
	ExecutedInstruction.fire(*this);
	return cycles();
}

int EightBit::Z80::execute() {

	const auto& decoded = getDecodedOpcode(opcode());

	const auto x = decoded.x;
	const auto y = decoded.y;
	const auto z = decoded.z;

	const auto p = decoded.p;
	const auto q = decoded.q;

	if (m_prefixCB)
		executeCB(x, y, z);
	else if (m_prefixED)
		executeED(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	ASSUME(cycles() > 0);
	return cycles();
}

void EightBit::Z80::executeCB(const int x, const int y, const int z) {

	const bool memoryZ = z == 6;
	const bool indirect = (!m_displaced && memoryZ) || m_displaced;
	const bool direct = !indirect;

	uint8_t operand;
	if (m_displaced) {
		tick(2);
		operand = IntelProcessor::memoryRead(displacedAddress());
	} else {
		operand = R(z);
	}

	const bool update = x != 1; // BIT does not update
	switch (x) {
	case 0:	{ // rot[y] r[z]
		switch (y) {
		case 0:
			operand = rlc(F(), operand);
			break;
		case 1:
			operand = rrc(F(), operand);
			break;
		case 2:
			operand = rl(F(), operand);
			break;
		case 3:
			operand = rr(F(), operand);
			break;
		case 4:
			operand = sla(F(), operand);
			break;
		case 5:
			operand = sra(F(), operand);
			break;
		case 6:
			operand = sll(F(), operand);
			break;
		case 7:
			operand = srl(F(), operand);
			break;
		default:
			UNREACHABLE;
		}
		F() = adjustSZP<Z80>(F(), operand);
		break;
	} case 1:	// BIT y, r[z]
		bit(F(), y, operand);
		F() = adjustXY<Z80>(F(), direct ? operand : MEMPTR().high);
		if (memoryZ)
			tick();
		break;
	case 2:	// RES y, r[z]
		operand = res(y, operand);
		break;
	case 3:	// SET y, r[z]
		operand = set(y, operand);
		break;
	default:
		UNREACHABLE;
	}
	if (update) {
		tick();
		if (m_displaced) {
			IntelProcessor::memoryWrite(operand);
			if (!memoryZ)
				R2(z, operand);
		} else {
			R(z, operand);
		}
	}
}

void EightBit::Z80::executeED(const int x, const int y, const int z, const int p, const int q) {

	switch (x) {
	case 0:
	case 3:	// Invalid instruction, equivalent to NONI followed by NOP
		break;
	case 1:
		switch (z) {
		case 0: // Input from port with 16-bit address
			(MEMPTR() = BUS().ADDRESS() = BC())++;
			portRead();
			if (y != 6)	// IN r[y],(C)
				R(y, BUS().DATA());
			F() = adjustSZPXY<Z80>(F(), BUS().DATA());
			F() = clearBit(F(), NF | HC);
			break;
		case 1:	// Output to port with 16-bit address
			(MEMPTR() = BUS().ADDRESS() = BC())++;
			BUS().DATA() = y == 6 ? 0 : R(y);
			portWrite();
			break;
		case 2:	// 16-bit add/subtract with carry
			switch (q) {
			case 0:	// SBC HL, rp[p]
				HL2() = sbc(F(), HL2(), RP(p));
				break;
			case 1:	// ADC HL, rp[p]
				HL2() = adc(F(), HL2(), RP(p));
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 3:	// Retrieve/store register pair from/to immediate address
			BUS().ADDRESS() = fetchWord();
			switch (q) {
			case 0: // LD (nn), rp[p]
				setWord(RP(p));
				break;
			case 1:	// LD rp[p], (nn)
				RP(p) = getWord();
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 4:	// Negate accumulator
			A() = neg(F(), A());
			break;
		case 5:	// Return from interrupt
			switch (y) {
			case 1:
				reti();	// RETI
				break;
			default:
				retn();	// RETN
				break;
			}
			break;
		case 6:	// Set interrupt mode
			switch (y) {
			case 0:
			case 1:
			case 4:
			case 5:
				IM() = 0;
				break;
			case 2:
			case 6:
				IM() = 1;
				break;
			case 3:
			case 7:
				IM() = 2;
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 7:	// Assorted ops
			switch (y) {
			case 0:	// LD I,A
				IV() = A();
				tick();
				break;
			case 1:	// LD R,A
				REFRESH() = A();
				tick();
				break;
			case 2:	// LD A,I
				readInternalRegister([this]() { return IV(); });
				break;
			case 3:	// LD A,R
				readInternalRegister([this]() { return REFRESH(); });
				tick();
				break;
			case 4:	// RRD
				rrd(F(), HL(), A());
				break;
			case 5:	// RLD
				rld(F(), HL(), A());
				break;
			case 6:	// NOP
			case 7:	// NOP
				break;
			default:
				UNREACHABLE;
			}
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 2:
		switch (z) {
		case 0:	// LD
			switch (y) {
			case 4:	// LDI
				ldi(F(), A());
				break;
			case 5:	// LDD
				ldd(F(), A());
				break;
			case 6:	// LDIR
				if (ldir(F(), A())) {
					MEMPTR() = --PC();
					--PC();
					tick(5);
				}
				break;
			case 7:	// LDDR
				if (lddr(F(), A())) {
					MEMPTR() = --PC();
					--PC();
					tick(5);
				}
				break;
			}
			break;
		case 1:	// CP
			switch (y) {
			case 4:	// CPI
				cpi(F(), A());
				break;
			case 5:	// CPD
				cpd(F(), A());
				break;
			case 6:	// CPIR
				if (cpir(F(), A())) {
					MEMPTR() = --PC();
					--PC();
					tick(5);
				}
				break;
			case 7:	// CPDR
				if (cpdr(F(), A())) {
					MEMPTR() = --PC();
					--PC();
					tick(5);
				} else {
					MEMPTR() = PC() - 2;
					tick(2);
				}
				break;
			}
			break;
		case 2:	// IN
			switch (y) {
			case 4:	// INI
				ini();
				break;
			case 5:	// IND
				ind();
				break;
			case 6:	// INIR
				if (inir()) {
					PC() -= 2;
					tick(5);
				}
				break;
			case 7:	// INDR
				if (indr()) {
					PC() -= 2;
					tick(5);
				}
				break;
			}
			break;
		case 3:	// OUT
			switch (y) {
			case 4:	// OUTI
				outi();
				break;
			case 5:	// OUTD
				outd();
				break;
			case 6:	// OTIR
				if (otir()) {
					PC() -= 2;
					tick(5);
				}
				break;
			case 7:	// OTDR
				if (otdr()) {
					PC() -= 2;
					tick(5);
				}
				break;
			}
			break;
		}
		break;
	}
}

void EightBit::Z80::executeOther(const int x, const int y, const int z, const int p, const int q) {
	const bool memoryY = y == 6;
	const bool memoryZ = z == 6;
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				break;
			case 1:	// EX AF AF'
				exxAF();
				break;
			case 2:	// DJNZ d
				tick();
				jrConditional(--B());
				break;
			case 3:	// JR d
				jr(fetchByte());
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				jrConditionalFlag(F(), y - 4);
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				RP(p) = fetchWord();
				break;
			case 1:	// ADD HL,rp
				HL2() = add(F(), HL2(), RP(p));
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 2:	// Indirect loading
			switch (q) {
			case 0:
				switch (p) {
				case 0:	// LD (BC),A
					storeAccumulatorIndirect([this]() { return BC(); });
					break;
				case 1:	// LD (DE),A
					storeAccumulatorIndirect([this]() { return DE(); });
					break;
				case 2:	// LD (nn),HL
					BUS().ADDRESS() = fetchWord();
					setWord(HL2());
					break;
				case 3: // LD (nn),A
					storeAccumulatorIndirect([this]() { return fetchWord(); });
					break;
				default:
					UNREACHABLE;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					loadAccumulatorIndirect([this]() { return BC(); });
					break;
				case 1:	// LD A,(DE)
					loadAccumulatorIndirect([this]() { return DE(); });
					break;
				case 2:	// LD HL,(nn)
					BUS().ADDRESS() = fetchWord();
					HL2() = getWord();
					break;
				case 3:	// LD A,(nn)
					loadAccumulatorIndirect([this]() { return fetchWord(); });
					break;
				default:
					UNREACHABLE;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 3:	// 16-bit INC/DEC
			switch (q) {
			case 0:	// INC rp
				++RP(p);
				break;
			case 1:	// DEC rp
				--RP(p);
				break;
			default:
				UNREACHABLE;
			}
			tick(2);
			break;
		case 4: { // 8-bit INC
			if (memoryY && m_displaced) {
				fetchDisplacement();
				tick(5);
			}
			const auto original = R(y);
			if (memoryY)
				tick();
			R(y, increment(F(), original));
			break;
		}
		case 5: { // 8-bit DEC
			if (memoryY && m_displaced) {
				fetchDisplacement();
				tick(5);
			}
			const auto original = R(y);
			if (memoryY)
				tick();
			R(y, decrement(F(), original));
			break;
		}
		case 6: { // 8-bit load immediate
			if (memoryY && m_displaced)
				fetchDisplacement();
			const auto value = fetchByte();
			if (m_displaced)
				tick(2);
			R(y, value);	// LD r,n
			break;
		}
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				A() = rlc(F(), A());
				break;
			case 1:
				A() = rrc(F(), A());
				break;
			case 2:
				A() = rl(F(), A());
				break;
			case 3:
				A() = rr(F(), A());
				break;
			case 4:
				A() = daa(F(), A());
				break;
			case 5:
				A() = cpl(F(), A());
				break;
			case 6:
				scf(F(), A());
				break;
			case 7:
				ccf(F(), A());
				break;
			default:
				UNREACHABLE;
			}
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 1:	// 8-bit loading
		if ((memoryZ && memoryY)) { 	// Exception (replaces LD (HL), (HL))
			lowerHALT();
		} else {
			bool normal = true;
			if (m_displaced) {
				if (memoryZ || memoryY)
					fetchDisplacement();
				if (memoryZ) {
					switch (y) {
					case 4:
						if (m_displaced)
							tick(5);
						H() = R(z);
						normal = false;
						break;
					case 5:
						if (m_displaced)
							tick(5);
						L() = R(z);
						normal = false;
						break;
					}
				}
				if (memoryY) {
					switch (z) {
					case 4:
						if (m_displaced)
							tick(5);
						R(y, H());
						normal = false;
						break;
					case 5:
						if (m_displaced)
							tick(5);
						R(y, L());
						normal = false;
						break;
					}
				}
			}
			if (normal) {
				if (m_displaced)
					tick(5);
				R(y, R(z));
			}
		}
		break;
	case 2: { // Operate on accumulator and register/memory location
		if (memoryZ && m_displaced) {
			fetchDisplacement();
			tick(5);
		}
		const auto value = R(z);
		switch (y) {
		case 0:	// ADD A,r
			A() = add(F(), A(), value);
			break;
		case 1:	// ADC A,r
			A() = adc(F(), A(), value);
			break;
		case 2:	// SUB r
			A() = sub(F(), A(), value);
			break;
		case 3:	// SBC A,r
			A() = sbc(F(), A(), value);
			break;
		case 4:	// AND r
			A() = andr(F(), A(), value);
			break;
		case 5:	// XOR r
			A() = xorr(F(), A(), value);
			break;
		case 6:	// OR r
			A() = orr(F(), A(), value);
			break;
		case 7:	// CP r
			compare(F(), A(), value);
			break;
		default:
			UNREACHABLE;
		}
		break;
	}
	case 3:
		switch (z) {
		case 0:	// Conditional return
			returnConditionalFlag(F(), y);
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				RP2(p) = popWord();
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					break;
				case 1:	// EXX
					exx();
					break;
				case 2:	// JP (HL)
					jump(HL2());
					break;
				case 3:	// LD SP,HL
					SP() = HL2();
					tick(2);
					break;
				default:
					UNREACHABLE;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 2:	// Conditional jump
			jumpConditionalFlag(F(), y);
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				jump(MEMPTR() = fetchWord());
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				if (m_displaced) {
					fetchDisplacement();
					IntelProcessor::execute(fetchByte());
				} else {
					IntelProcessor::execute(fetchOpCode());
				}
				break;
			case 2:	// OUT (n),A
				portWrite(fetchByte());
				break;
			case 3:	// IN A,(n)
				A() = portRead(fetchByte());
				break;
			case 4:	// EX (SP),HL
				xhtl(HL2());
				break;
			case 5:	// EX DE,HL
				std::swap(DE(), HL());
				break;
			case 6:	// DI
				di();
				break;
			case 7:	// EI
				ei();
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			callConditionalFlag(F(), y);
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					call(MEMPTR() = fetchWord());
					break;
				case 1:	// DD prefix
					m_displaced = m_prefixDD = true;
					IntelProcessor::execute(fetchOpCode());
					break;
				case 2:	// ED prefix
					m_prefixED = true;
					IntelProcessor::execute(fetchOpCode());
					break;
				case 3:	// FD prefix
					m_displaced = m_prefixFD = true;
					IntelProcessor::execute(fetchOpCode());
					break;
				default:
					UNREACHABLE;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 6: { // Operate on accumulator and immediate operand: alu[y] n
			const auto operand = fetchByte();
			switch (y) {
			case 0:	// ADD A,n
				A() = add(F(), A(), operand);
				break;
			case 1:	// ADC A,n
				A() = adc(F(), A(), operand);
				break;
			case 2:	// SUB n
				A() = sub(F(), A(), operand);
				break;
			case 3:	// SBC A,n
				A() = sbc(F(), A(), operand);
				break;
			case 4:	// AND n
				A() = andr(F(), A(), operand);
				break;
			case 5:	// XOR n
				A() = xorr(F(), A(), operand);
				break;
			case 6:	// OR n
				A() = orr(F(), A(), operand);
				break;
			case 7:	// CP n
				compare(F(), A(), operand);
				break;
			default:
				UNREACHABLE;
			}
			break;
		}
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			break;
		default:
			UNREACHABLE;
		}
		break;
	}
}
