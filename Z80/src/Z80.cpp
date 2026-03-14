#include "stdafx.h"
#include "../inc/Z80.h"

// based on http://www.z80.info/decoding.htm

EightBit::Z80::Z80(Bus& bus, InputOutput& ports)
: IntelProcessor(bus),
  m_ports(ports) {
	RaisedPOWER.connect([this](EventArgs) {

		raiseM1();
		raiseRFSH();
		raiseIORQ();
		raiseMREQ();
		raiseRD();
		raiseWR();

		disableInterrupts();
		IM() = 0;

		REFRESH() = 0;
		IV() = Mask8;

		exxAF();
		exx();

		IX() = IY() = Mask16;
		resetWorkingRegisters();
	});

	RaisedM1.connect([this](EventArgs) {
		++REFRESH();
	});

	LoweredRESET.connect([this](EventArgs) {
		m_resetPending = true;
	});

	LoweredNMI.connect([this](EventArgs) {
		m_nonMaskableInterruptPending = true;
	});

	LoweredINT.connect([this](EventArgs) {
		m_interruptPending = true;
	});
}

EightBit::Z80::Z80(const Z80& rhs)
: IntelProcessor(rhs),
  m_ports(rhs.m_ports) {

	m_registers = rhs.m_registers;
	m_registerSet = rhs.m_registerSet;

	m_accumulatorFlags = rhs.m_accumulatorFlags;
	m_accumulatorFlagsSet = rhs.m_accumulatorFlagsSet;

	m_ix = rhs.m_ix;
	m_iy = rhs.m_iy;

	iv = rhs.iv;
	m_interruptMode = rhs.m_interruptMode;
	m_iff1 = rhs.m_iff1;
	m_iff2 = rhs.m_iff2;

	RFSH() = rhs.RFSH();
	NMI() = rhs.NMI();
	M1() = rhs.M1();
	MREQ() = rhs.MREQ();
	IORQ() = rhs.IORQ();
	RD() = rhs.RD();
	WR() = rhs.WR();
}

bool EightBit::Z80::operator==(const EightBit::Z80& rhs) const {

	const auto base = IntelProcessor::operator==(rhs);

	const auto pins =
		RFSH() == rhs.RFSH()
		&& NMI() == rhs.NMI()
		&& M1() == rhs.M1()
		&& MREQ() == rhs.MREQ()
		&& IORQ() == rhs.IORQ()
		&& RD() == rhs.RD()
		&& WR() == rhs.WR();

	auto* z80 = const_cast<Z80*>(this);
	auto& z80_rhs = const_cast<Z80&>(rhs);

	z80->exxAF();
	z80_rhs.exxAF();
	const auto af = AF() == rhs.AF();
	z80->exxAF();
	z80_rhs.exxAF();

	z80->exx();
	z80_rhs.exx();
	const auto pairs =
		BC() == rhs.BC()
		&& DE() == rhs.DE()
		&& HL() == rhs.HL();
	z80->exx();
	z80_rhs.exx();

	const auto indices =
		IX() == rhs.IX()
		&& IY() == rhs.IY();

	const auto miscellaneous =
		REFRESH() == rhs.REFRESH()
		&& IV() == rhs.IV()
		&& IM() == rhs.IM()
		&& IFF1() == rhs.IFF1()
		&& IFF2() == rhs.IFF2();

	return base && pins && af && pairs && indices && miscellaneous;
}

DEFINE_PIN_LEVEL_CHANGERS(NMI, Z80);
DEFINE_PIN_LEVEL_CHANGERS(M1, Z80);
DEFINE_PIN_LEVEL_CHANGERS(RFSH, Z80);
DEFINE_PIN_LEVEL_CHANGERS(MREQ, Z80);
DEFINE_PIN_LEVEL_CHANGERS(IORQ, Z80);
DEFINE_PIN_LEVEL_CHANGERS(RD, Z80);
DEFINE_PIN_LEVEL_CHANGERS(WR, Z80);

const EightBit::register16_t& EightBit::Z80::AF() const noexcept {
	return m_accumulatorFlags[m_accumulatorFlagsSet];
}

const EightBit::register16_t& EightBit::Z80::BC() const noexcept {
	return m_registers[m_registerSet][BC_IDX];
}

const EightBit::register16_t& EightBit::Z80::DE() const noexcept {
	return m_registers[m_registerSet][DE_IDX];
}

const EightBit::register16_t& EightBit::Z80::HL() const noexcept {
	return m_registers[m_registerSet][HL_IDX];
}

void EightBit::Z80::memoryUpdate(int ticks) noexcept {
	assert(ticks > 0 && "Ticks must be greater than zero");
	WritingMemory.fire();
	tick(ticks);
	lowerMREQ();
		lowerWR();
			tick();
			IntelProcessor::memoryWrite();
		raiseWR();
	raiseMREQ();
	tick();
	WrittenMemory.fire();
}

void EightBit::Z80::memoryWrite() noexcept {
	memoryUpdate(1);
}

void EightBit::Z80::refreshMemory() noexcept {
	assert(lowered(M1()) && "M1 must be lowered to refresh memory");
	BUS().ADDRESS() = { REFRESH(), IV() };
	lowerRFSH();
		tick();
		lowerMREQ();
		raiseMREQ();
	raiseRFSH();
}

uint8_t EightBit::Z80::memoryRead() noexcept {
	ReadingMemory.fire();
	tick();
	lowerMREQ();
		lowerRD();
			tick();
			const auto data = IntelProcessor::memoryRead();
		raiseRD();
	raiseMREQ();
	if (lowered(M1()))
		refreshMemory();
	tick();
	ReadMemory.fire();
	return data;
}

void EightBit::Z80::handleRESET() noexcept {
	IntelProcessor::handleRESET();
	disableInterrupts();
	IM() = 0;
	IV() = 0;
	REFRESH() = 0;
	SP().word = AF().word = Mask16;
}

void EightBit::Z80::handleNMI() noexcept {
	raiseNMI();
	raiseHALT();
	IFF2() = IFF1();
	IFF1() = false;
	lowerM1();
	raiseM1();
	restart(0x66);
}

uint8_t EightBit::Z80::readDataUnderInterrupt() {
	lowerM1();
		tick(3);
		lowerIORQ();
			tick();
			const auto data = BUS().DATA();
		raiseIORQ();
		assert(cycles() == 4);
		refreshMemory();
		assert(cycles() == 5);
	raiseM1();
	return data;
}

void EightBit::Z80::handleINT() noexcept {

	IntelProcessor::handleINT();

	const auto data = readDataUnderInterrupt();
	tick();
	assert(cycles() == 6);

	switch (IM()) {
	case 0:		// i8080 equivalent
		IntelProcessor::execute(data);
		break;
	case 1:
		restart(7 << 3);
		assert(cycles() == 13);
		break;
	case 2:
		tick();
		MEMPTR() = Processor::getWordPaged(IV(), data);
		call(MEMPTR());
		assert(cycles() == 13);
		break;
	default:
		UNREACHABLE;
	}
}

void EightBit::Z80::disableInterrupts() {
	IFF1() = IFF2() = false;
}

void EightBit::Z80::enableInterrupts() {
	IFF1() = IFF2() = true;
}

void EightBit::Z80::returnConditionalFlag(const int flag) noexcept {
	const auto condition = convertCondition(flag);
	tick();
	if (condition)
		ret();
}

void EightBit::Z80::retn() noexcept {
	ret();
	IFF1() = IFF2();
}

void EightBit::Z80::reti() noexcept {
	retn();
}


void EightBit::Z80::call(register16_t destination) {
	tick();
	Processor::call(destination);
}

void EightBit::Z80::jumpRelative(int8_t offset) noexcept {
	IntelProcessor::jumpRelative(offset);
	tick(5);
}

void EightBit::Z80::sbc(const register16_t value) noexcept {

	const auto operand = HL2();

	const auto subtraction = operand.word - value.word - carry();
	intermediate() = subtraction;

	setBit(NF);
	setBit(ZF, intermediate().zero());
	setBit(CF, subtraction & Bit16);
	adjustHalfCarrySub(operand.high, value.high, intermediate().high);
	adjustXY(intermediate().high);

	const auto beforeNegative = signTest(operand.high);
	const auto valueNegative = signTest(value.high);
	const auto afterNegative = signTest(intermediate().high);

	setBit(SF, afterNegative);
	adjustOverflowSub(beforeNegative, valueNegative, afterNegative);

	MEMPTR() = operand + 1;

	HL2() = intermediate();
}

void EightBit::Z80::adc(const register16_t value) noexcept {

	const auto beforeNegative = signTest(HL2().high);
	const auto valueNegative = signTest(value.high);

	add(value, carry());
	setBit(ZF, intermediate().zero());

	const auto afterNegative = signTest(intermediate().high);

	setBit(SF, afterNegative);
	adjustOverflowAdd(beforeNegative, valueNegative, afterNegative);
}

void EightBit::Z80::add(const register16_t value, int carry) noexcept {

	const auto operand = HL2();

	const auto addition = operand.word + value.word + carry;
	intermediate().word = addition;

	clearBit(NF);
	setBit(CF, addition & Bit16);
	adjustHalfCarryAdd(operand.high, value.high, intermediate().high);
	adjustXY(intermediate().high);

	MEMPTR() = operand + 1;

	HL2() = intermediate();
}

void EightBit::Z80::xhtl(register16_t& exchange) noexcept {
	MEMPTR().low = IntelProcessor::memoryRead(SP());
	++BUS().ADDRESS();
	MEMPTR().high = memoryRead();
	BUS().DATA() = exchange.high;
	exchange.high = MEMPTR().high;
	memoryUpdate(2);
	--BUS().ADDRESS();
	BUS().DATA() = exchange.low;
	exchange.low = MEMPTR().low;
	memoryUpdate(1);
	tick(2);
}

void EightBit::Z80::blockCompare() noexcept {

	const auto data = IntelProcessor::memoryRead(HL());
	uint8_t result = A() - data;

	setBit(PF, --BC().word);

	adjustSZ(result);
	adjustHalfCarrySub(A(), data, result);
	setBit(NF);

	result -= (halfCarry() >> 4);

	setBit(YF, result & Bit1);
	setBit(XF, result & Bit3);

	tick(5);
}

void EightBit::Z80::cpi() noexcept {
	blockCompare();
	++HL();
	++MEMPTR();
}

void EightBit::Z80::cpd() noexcept {
	blockCompare();
	--HL();
	--MEMPTR();
}

void EightBit::Z80::cpir() noexcept {
	cpi();
	if (parity() != 0 && zero() == 0) {
		repeatBlockInstruction();
		tick(5);
	}
}

void EightBit::Z80::cpdr() noexcept {
	cpd();
	if (parity() != 0 && zero() == 0) {
		repeatBlockInstruction();
		tick(5);
	} else {
		MEMPTR().word = PC().word - 2;
		tick(2);
	}
}

void EightBit::Z80::blockLoad() noexcept {
	const auto data = IntelProcessor::memoryRead(HL());
	IntelProcessor::memoryWrite(DE());
	const auto xy = A() + data;
	setBit(XF, xy & Bit3);
	setBit(YF, xy & Bit1);
	clearBit(NF | HC);
	setBit(PF, --BC().word);
	tick(2);
}

void EightBit::Z80::ldd() noexcept {
	blockLoad();
	--HL();
	--DE();
}

void EightBit::Z80::ldi() noexcept {
	blockLoad();
	++HL();
	++DE();
}

void EightBit::Z80::ldir() noexcept {
	ldi();
	if (parity() != 0)
		repeatBlockInstruction();
	tick(5);
}

void EightBit::Z80::lddr() noexcept {
	ldd();
	if (parity() != 0)
		repeatBlockInstruction();
	tick(5);
}

void EightBit::Z80::repeatBlockInstruction() noexcept {
	MEMPTR() = --PC();
	--PC();
	adjustXY(PC().high);
}

void EightBit::Z80::adjustBlockRepeatFlagsIO() {
	if (carry() != 0) {
		const auto negative = signTest(BUS().DATA()) != 0;
		const uint8_t direction = B() + (negative ? -1 : +1);
		const auto calculatedParity = (parity() >> 2) ^ (evenParity(direction & Mask::Mask3) ? 1 : 0) ^ 1;
		setBit(PF, calculatedParity != 0);
		setBit(HC, (B() & Mask::Mask4) == (negative ? 0 : Mask::Mask4));
	} else {
		adjustParity((B() & Mask::Mask3));
	}
}

void EightBit::Z80::adjustBlockInputOutputFlags(int basis) noexcept {
	setBit(HC | CF, basis > 0xff);
	adjustParity(((basis & Mask::Mask3) ^ B()));
}

void EightBit::Z80::adjustBlockInFlagsIncrement() noexcept {
	adjustBlockInputOutputFlags((C() + 1) + BUS().DATA());
}

void EightBit::Z80::adjustBlockInFlagsDecrement() noexcept {
	adjustBlockInputOutputFlags((C() - 1) + BUS().DATA());
}

void EightBit::Z80::adjustBlockOutFlags() noexcept {
	// HL needs to have been incremented or decremented prior to this call
	setBit(NF, signTest(BUS().DATA()));
	adjustBlockInputOutputFlags(L() + BUS().DATA());
}

void EightBit::Z80::blockIn() noexcept {
	tick();
	readPort(BC());
	BUS().ADDRESS() = HL();
	memoryUpdate(1);
	adjustSZXY(--B());
	setBit(NF, BUS().DATA() & SF);
}

void EightBit::Z80::ini() noexcept {
	blockIn();
	adjustBlockInFlagsIncrement();
	++HL();
	++MEMPTR();
}

void EightBit::Z80::ind() noexcept {
	blockIn();
	adjustBlockInFlagsDecrement();
	--HL();
	--MEMPTR();
}

void EightBit::Z80::inir() noexcept {
	ini();
	if (zero() == 0) {
		repeatBlockInstruction();
		adjustBlockRepeatFlagsIO();
		tick(5);
	}
}

void EightBit::Z80::indr() noexcept {
	ind();
	if (zero() == 0) {
		repeatBlockInstruction();
		adjustBlockRepeatFlagsIO();
		tick(5);
	}
}

void EightBit::Z80::blockOut() noexcept {
	tick();
	Processor::memoryRead(HL());
	B() = decrement(B());
	writePort(BC());
}

void EightBit::Z80::outi() noexcept {
	blockOut();
	++HL();
	adjustBlockOutFlags();
	++MEMPTR();
}

void EightBit::Z80::outd() noexcept {
	blockOut();
	--HL();
	adjustBlockOutFlags();
	--MEMPTR();
}

void EightBit::Z80::otir() noexcept {
	outi();
	if (zero() == 0) {
		repeatBlockInstruction();
		adjustBlockRepeatFlagsIO();
		tick(5);
	}
}

void EightBit::Z80::otdr() noexcept {
	outd();
	if (zero() == 0) {
		repeatBlockInstruction();
		adjustBlockRepeatFlagsIO();
		tick(5);
	}
}

void EightBit::Z80::rrd(register16_t address, uint8_t& update) noexcept {
	(MEMPTR() = BUS().ADDRESS() = address)++;
	const auto memory = memoryRead();
	tick(4);
	IntelProcessor::memoryWrite(promoteNibble(update) | highNibble(memory));
	update = higherNibble(update) | lowerNibble(memory);
	adjustSZPXY(update);
	clearBit(NF | HC);
}

void EightBit::Z80::rld(register16_t address, uint8_t& update) noexcept {
	(MEMPTR() = BUS().ADDRESS() = address)++;
	const auto memory = memoryRead();
	tick(4);
	IntelProcessor::memoryWrite(promoteNibble(memory) | lowNibble(update));
	update = higherNibble(update) | highNibble(memory);
	adjustSZPXY(update);
	clearBit(NF | HC);
}

void EightBit::Z80::writePort(const register16_t port) noexcept {
	BUS().ADDRESS() = port;
	writePort();
}

void EightBit::Z80::writePort(const uint8_t port) noexcept {
	BUS().ADDRESS() = { port, BUS().DATA() = A() };
	writePort();
	++MEMPTR().low;
}

void EightBit::Z80::writePort() noexcept {
	MEMPTR() = BUS().ADDRESS();
	tick(2);
	lowerIORQ();
		lowerWR();
			tick();
			m_ports.write(BUS().ADDRESS(), BUS().DATA());
		raiseWR();
	raiseIORQ();
	tick();
}

void EightBit::Z80::readPort(register16_t port) noexcept {
	BUS().ADDRESS() = port;
	readPort();
}

void EightBit::Z80::readPort(const uint8_t port) noexcept {
	BUS().ADDRESS() = { port, A() };
	readPort();
	++MEMPTR();
}

void EightBit::Z80::readPort() noexcept {
	MEMPTR() = BUS().ADDRESS();
	tick(2);
	lowerIORQ();
		lowerRD();
			BUS().DATA() = m_ports.read(BUS().ADDRESS());
			tick();
		raiseRD();
	raiseIORQ();
	tick();
}

//

void EightBit::Z80::fetchDisplacement() noexcept {
	m_displacement = fetchByte();
}

//

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
uint8_t EightBit::Z80::fetchInstruction() noexcept {
	lowerM1();
		const auto data = IntelProcessor::fetchInstruction();
	raiseM1();
	return data;
}

void EightBit::Z80::loadAccumulatorIndirect(addresser_t addresser) noexcept {
	(MEMPTR() = BUS().ADDRESS() = addresser())++;
	A() = memoryRead();
}

void EightBit::Z80::storeAccumulatorIndirect(addresser_t addresser) noexcept {
	(MEMPTR() = BUS().ADDRESS() = addresser())++;
	MEMPTR().high = BUS().DATA() = A();
	memoryWrite();
}

void EightBit::Z80::readInternalRegister(reader_t reader) noexcept {
	adjustSZXY(A() = reader());
	clearBit(NF | HC);
	setBit(PF, IFF2());
}

EightBit::register16_t& EightBit::Z80::HL2() noexcept {
	if (UNLIKELY(m_prefixDD))
		return IX();
	if (UNLIKELY(m_prefixFD))
		return IY();
	return HL();
}

EightBit::register16_t& EightBit::Z80::RP(const int rp) noexcept {
	switch (rp) {
	case 0b00:
		return BC();
	case 0b01:
		return DE();
	case 0b10:
		return HL2();
	case 0b11:
		return SP();
	default:
		UNREACHABLE;
	}
}

EightBit::register16_t& EightBit::Z80::RP2(const int rp) noexcept {
	switch (rp) {
	case 0b00:
		return BC();
	case 0b01:
		return DE();
	case 0b10:
		return HL2();
	case 0b11:
		return AF();
	default:
		UNREACHABLE;
	}
}

uint8_t& EightBit::Z80::R(const int r, MemoryMapping::AccessLevel access) noexcept {
	switch (r) {
	case 0:
		return B();
	case 1:
		return C();
	case 2:
		return D();
	case 3:
		return E();
	case 4:
		return HL2().high;
	case 5:
		return HL2().low;
	case 6:
		if (displaced())
			displaceAddress();
		else
			BUS().ADDRESS() = HL();
		switch (access) {
			case MemoryMapping::AccessLevel::ReadOnly:
				memoryRead();
				break;
			case MemoryMapping::AccessLevel::WriteOnly:
				break;
			default:
				UNREACHABLE;
				break;
		}
		return BUS().DATA();
	case 7:
		return A();
	default:
		UNREACHABLE;
	}
}

void EightBit::Z80::R(const int r, const uint8_t value, const int ticks) noexcept {

	R(r, MemoryMapping::AccessLevel::WriteOnly) = value;
	if (r == 6)
		memoryUpdate(ticks);
}

void EightBit::Z80::R2(const int r, const uint8_t value) noexcept {
	switch (r) {
	case 0:
		B() = value;
		break;
	case 1:
		C() = value;
		break;
	case 2:
		D() = value;
		break;
	case 3:
		E() = value;
		break;
	case 4:
		H() = value;
		break;
	case 5:
		L() = value;
		break;
	case 6:
		IntelProcessor::memoryWrite(HL(), value);
		break;
	case 7:
		A() = value;
		break;
	default:
		UNREACHABLE;
	}
}

void EightBit::Z80::poweredStep() noexcept {

	m_modifiedF = 0;
	m_prefixCB = m_prefixDD = m_prefixED = m_prefixFD = false;

	if (m_resetPending) {
		m_resetPending = false;
		handleRESET();
		return;
	} else if (m_nonMaskableInterruptPending) {
		m_nonMaskableInterruptPending = false;
		handleNMI();
		return;
	} else if (m_interruptPending) {
		m_interruptPending = false;
		if (IFF1()) {
			handleINT();
			return;
		}
	}

	IntelProcessor::execute(fetchInstruction());

	Q() = m_modifiedF;
}

void EightBit::Z80::execute() noexcept {

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
}

void EightBit::Z80::executeCB(const int x, const int y, const int z) noexcept {

	const bool memoryZ = z == 6;
	const bool indirect = (!displaced() && memoryZ) || displaced();
	const auto direct = !indirect;

	uint8_t operand;
	if (displaced()) {
		tick(2);
		displaceAddress();
		operand = memoryRead();
	} else {
		operand = R(z);
	}

	const bool update = x != 1; // BIT does not update
	switch (x) {
	case 0:	{ // rot[y] r[z]
		switch (y) {
		case 0:
			operand = rlc(operand);
			break;
		case 1:
			operand = rrc(operand);
			break;
		case 2:
			operand = rl(operand);
			break;
		case 3:
			operand = rr(operand);
			break;
		case 4:
			operand = sla(operand);
			break;
		case 5:
			operand = sra(operand);
			break;
		case 6:
			operand = sll(operand);
			break;
		case 7:
			operand = srl(operand);
			break;
		default:
			UNREACHABLE;
		}
		adjustSZP(operand);
		break;
	} case 1:	// BIT y, r[z]
		bit(y, operand);
		adjustXY(direct ? operand : MEMPTR().high);
		if (indirect)
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
		if (displaced()) {
			tick();
			IntelProcessor::memoryWrite(operand);
			if (!memoryZ)
				R2(z, operand);
		} else {
			R(z, operand, 2);
		}
	}
}

void EightBit::Z80::executeED(const int x, const int y, const int z, const int p, const int q) noexcept {

	switch (x) {
	case 0:
	case 3:	// Invalid instruction, equivalent to NONI followed by NOP
		break;
	case 1:
		switch (z) {
		case 0: // Input from port with 16-bit address
			readPort(BC());
			++MEMPTR();
			if (y != 6)	// IN r[y],(C)
				R(y, BUS().DATA());
			adjustSZPXY(BUS().DATA());
			clearBit(NF | HC);
			break;
		case 1:	// Output to port with 16-bit address
			BUS().DATA() = y == 6 ? 0 : R(y);
			writePort(BC());
			++MEMPTR();
			break;
		case 2:	// 16-bit add/subtract with carry
			switch (q) {
			case 0:	// SBC HL, rp[p]
				sbc(RP(p));
				break;
			case 1:	// ADC HL, rp[p]
				adc(RP(p));
				break;
			default:
				UNREACHABLE;
			}
			tick(7);
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
			neg();
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
				tick();
				break;
			case 3:	// LD A,R
				readInternalRegister([this]() { return REFRESH(); });
				tick();
				break;
			case 4:	// RRD
				rrd(HL(), A());
				break;
			case 5:	// RLD
				rld(HL(), A());
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
				ldi();
				break;
			case 5:	// LDD
				ldd();
				break;
			case 6:	// LDIR
				ldir();
				break;
			case 7:	// LDDR
				lddr();
				break;
			}
			break;
		case 1:	// CP
			switch (y) {
			case 4:	// CPI
				cpi();
				break;
			case 5:	// CPD
				cpd();
				break;
			case 6:	// CPIR
				cpir();
				break;
			case 7:	// CPDR
				cpdr();
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
				inir();
				break;
			case 7:	// INDR
				indr();
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
				otir();
				break;
			case 7:	// OTDR
				otdr();
				break;
			}
			break;
		}
		break;
	}
}

void EightBit::Z80::executeOther(const int x, const int y, const int z, const int p, const int q) noexcept {
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
				jumpRelativeConditional(--B());
				break;
			case 3:	// JR d
				jumpRelative(fetchByte());
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				jumpRelativeConditionalFlag(y - 4);
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
				add(RP(p));
				tick(7);
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
			if (memoryY && displaced()) {
				fetchDisplacement();
				tick(5);
			}
			const auto original = R(y);
			R(y, increment(original), 2);
			break;
		}
		case 5: { // 8-bit DEC
			if (memoryY && displaced()) {
				fetchDisplacement();
				tick(5);
			}
			const auto original = R(y);
			R(y, decrement(original), 2);
			break;
		}
		case 6: { // 8-bit load immediate
			if (memoryY && displaced())
				fetchDisplacement();
			const auto value = fetchByte();
			if (memoryY)
				tick(2);
			R(y, value);	// LD r,n
			break;
		}
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				A() = rlc(A());
				break;
			case 1:
				A() = rrc(A());
				break;
			case 2:
				A() = rl(A());
				break;
			case 3:
				A() = rr(A());
				break;
			case 4:
				daa();
				break;
			case 5:
				cpl();
				break;
			case 6:
				scf();
				break;
			case 7:
				ccf();
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
		if (!(memoryZ && memoryY)) {
			bool normal = true;
			if (displaced()) {
				if (memoryZ || memoryY) {
					fetchDisplacement();
					tick(5);
				}
				if (memoryZ) {
					switch (y) {
					case 4:
						H() = R(z);
						normal = false;
						break;
					case 5:
						L() = R(z);
						normal = false;
						break;
					}
				}
				if (memoryY) {
					switch (z) {
					case 4:
						R(y, H());
						normal = false;
						break;
					case 5:
						R(y, L());
						normal = false;
						break;
					}
				}
			}
			if (normal)
				R(y, R(z));
		} else {
			lowerHALT(); // Exception (replaces LD (HL), (HL))
		}
		break;
	case 2: { // Operate on accumulator and register/memory location
		if (memoryZ && displaced()) {
			fetchDisplacement();
			tick(5);
		}
		const auto value = R(z);
		switch (y) {
		case 0:	// ADD A,r
			add(value);
			break;
		case 1:	// ADC A,r
			adc(value);
			break;
		case 2:	// SUB r
			sub(value);
			break;
		case 3:	// SBC A,r
			sbc(value);
			break;
		case 4:	// AND r
			andr(value);
			break;
		case 5:	// XOR r
			xorr(value);
			break;
		case 6:	// OR r
			orr(value);
			break;
		case 7:	// CP r
			compare(value);
			break;
		default:
			UNREACHABLE;
		}
		break;
	}
	case 3:
		switch (z) {
		case 0:	// Conditional return
			returnConditionalFlag(y);
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
					Processor::jump(HL2());
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
			jumpConditionalFlag(y);
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				jumpIndirect();
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				if (displaced()) {
					fetchDisplacement();
					IntelProcessor::execute(fetchByte());
				} else {
					IntelProcessor::execute(fetchInstruction());
				}
				break;
			case 2:	// OUT (n),A
				writePort(fetchByte());
				break;
			case 3:	// IN A,(n)
				readPort(fetchByte());
				A() = BUS().DATA();
				break;
			case 4:	// EX (SP),HL
				xhtl(HL2());
				break;
			case 5:	// EX DE,HL
				std::swap(DE(), HL());
				break;
			case 6:	// DI
				disableInterrupts();
				break;
			case 7:	// EI
				enableInterrupts();
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			callConditionalFlag(y);
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				tick();
				pushWord(RP2(p));
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					callIndirect();
					break;
				case 1:	// DD prefix
					m_prefixDD = true;
					IntelProcessor::execute(fetchInstruction());
					break;
				case 2:	// ED prefix
					m_prefixED = true;
					IntelProcessor::execute(fetchInstruction());
					break;
				case 3:	// FD prefix
					m_prefixFD = true;
					IntelProcessor::execute(fetchInstruction());
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
				add(operand);
				break;
			case 1:	// ADC A,n
				adc(operand);
				break;
			case 2:	// SUB n
				sub(operand);
				break;
			case 3:	// SBC A,n
				sbc(operand);
				break;
			case 4:	// AND n
				andr(operand);
				break;
			case 5:	// XOR n
				xorr(operand);
				break;
			case 6:	// OR n
				orr(operand);
				break;
			case 7:	// CP n
				compare(operand);
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
