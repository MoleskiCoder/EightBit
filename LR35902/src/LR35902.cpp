#include "stdafx.h"
#include "LR35902.h"
#include "GameBoyBus.h"
#include "Display.h"

// based on http://www.z80.info/decoding.htm

EightBit::GameBoy::LR35902::LR35902(Bus& memory)
: IntelProcessor(memory),
  m_bus(memory),
  m_enabledLCD(false),
  m_ime(false),
  m_stopped(false),
  m_prefixCB(false) {
}

void EightBit::GameBoy::LR35902::reset() {
	IntelProcessor::reset();
	di();
	SP().word = Mask16 - 1;
	m_prefixCB = false;
}

void EightBit::GameBoy::LR35902::di() {
	IME() = false;
}

void EightBit::GameBoy::LR35902::ei() {
	IME() = true;
}

int EightBit::GameBoy::LR35902::interrupt(uint8_t value) {
	di();
	restart(value);
	return 4;
}

void EightBit::GameBoy::LR35902::increment(uint8_t& f, uint8_t& operand) {
	clearFlag(f, NF);
	adjustZero<LR35902>(f, ++operand);
	clearFlag(f, HC, lowNibble(operand));
}

void EightBit::GameBoy::LR35902::decrement(uint8_t& f, uint8_t& operand) {
	setFlag(f, NF);
	clearFlag(f, HC, lowNibble(operand));
	adjustZero<LR35902>(f, --operand);
}

bool EightBit::GameBoy::LR35902::jrConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return jrConditional(!(f & ZF));
	case 1:	// Z
		return jrConditional(f & ZF);
	case 2:	// NC
		return jrConditional(!(f & CF));
	case 3:	// C
		return jrConditional(f & CF);
	default:
		UNREACHABLE;
	}
	throw std::logic_error("Unhandled JR conditional");
}

bool EightBit::GameBoy::LR35902::jumpConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return jumpConditional(!(f & ZF));
	case 1:	// Z
		return jumpConditional(f & ZF);
	case 2:	// NC
		return jumpConditional(!(f & CF));
	case 3:	// C
		return jumpConditional(f & CF);
	default:
		UNREACHABLE;
	}
	throw std::logic_error("Unhandled JP conditional");
}

void EightBit::GameBoy::LR35902::reti() {
	ret();
	ei();
}

bool EightBit::GameBoy::LR35902::returnConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return returnConditional(!(f & ZF));
	case 1:	// Z
		return returnConditional(f & ZF);
	case 2:	// NC
		return returnConditional(!(f & CF));
	case 3:	// C
		return returnConditional(f & CF);
	default:
		UNREACHABLE;
	}
	throw std::logic_error("Unhandled RET conditional");
}

bool EightBit::GameBoy::LR35902::callConditionalFlag(uint8_t& f, int flag) {
	switch (flag) {
	case 0:	// NZ
		return callConditional(!(f & ZF));
	case 1:	// Z
		return callConditional(f & ZF);
	case 2:	// NC
		return callConditional(!(f & CF));
	case 3:	// C
		return callConditional(f & CF);
	default:
		UNREACHABLE;
	}
	throw std::logic_error("Unhandled CALL conditional");
}

void EightBit::GameBoy::LR35902::add(uint8_t& f, register16_t& operand, register16_t value) {

	MEMPTR() = operand;

	const auto result = MEMPTR().word + value.word;

	operand.word = result;

	clearFlag(f, NF);
	setFlag(f, CF, result & Bit16);
	adjustHalfCarryAdd(f, MEMPTR().high, value.high, operand.high);
}

void EightBit::GameBoy::LR35902::add(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand + value + carry;

	adjustHalfCarryAdd(f, operand, value, result.low);

	operand = result.low;

	clearFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustZero<LR35902>(f, operand);
}

void EightBit::GameBoy::LR35902::adc(uint8_t& f, uint8_t& operand, uint8_t value) {
	add(f, operand, value, (f & CF) >> 4);
}

void EightBit::GameBoy::LR35902::subtract(uint8_t& f, uint8_t& operand, uint8_t value, int carry) {

	register16_t result;
	result.word = operand - value - carry;

	adjustHalfCarrySub(f, operand, value, result.low);

	operand = result.low;

	setFlag(f, NF);
	setFlag(f, CF, result.word & Bit8);
	adjustZero<LR35902>(f, operand);
}

void EightBit::GameBoy::LR35902::sbc(uint8_t& f, uint8_t& operand, uint8_t value) {
	subtract(f, operand, value, (f & CF) >> 4);
}

void EightBit::GameBoy::LR35902::andr(uint8_t& f, uint8_t& operand, uint8_t value) {
	setFlag(f, HC);
	clearFlag(f, CF | NF);
	adjustZero<LR35902>(f, operand &= value);
}

void EightBit::GameBoy::LR35902::xorr(uint8_t& f, uint8_t& operand, uint8_t value) {
	clearFlag(f, HC | CF | NF);
	adjustZero<LR35902>(f, operand ^= value);
}

void EightBit::GameBoy::LR35902::orr(uint8_t& f, uint8_t& operand, uint8_t value) {
	clearFlag(f, HC | CF | NF);
	adjustZero<LR35902>(f, operand |= value);
}

void EightBit::GameBoy::LR35902::compare(uint8_t& f, uint8_t check, uint8_t value) {
	subtract(f, check, value);
}

uint8_t EightBit::GameBoy::LR35902::rlc(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC | ZF);
	auto carry = operand & Bit7;
	setFlag(f, CF, carry);
	return (operand << 1) | (carry >> 7);
}

uint8_t EightBit::GameBoy::LR35902::rrc(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC | ZF);
	auto carry = operand & Bit0;
	setFlag(f, CF, carry);
	return (operand >> 1) | (carry << 7);
}

uint8_t EightBit::GameBoy::LR35902::rl(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC | ZF);
	const auto carry = f & CF;
	setFlag(f, CF, operand & Bit7);
	return (operand << 1) | (carry >> 4);	// CF at Bit4
}

uint8_t EightBit::GameBoy::LR35902::rr(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC | ZF);
	const auto carry = f & CF;
	setFlag(f, CF, operand & Bit0);
	return (operand >> 1) | (carry << 3);	// CF at Bit4
}

uint8_t EightBit::GameBoy::LR35902::sla(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC | ZF);
	setFlag(f, CF, operand & Bit7);
	return operand << 1;
}

uint8_t EightBit::GameBoy::LR35902::sra(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC | ZF);
	setFlag(f, CF, operand & Bit0);
	return (operand >> 1) | (operand & Bit7);
}

uint8_t EightBit::GameBoy::LR35902::swap(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC | CF);
	return promoteNibble(operand) | demoteNibble(operand);
}

uint8_t EightBit::GameBoy::LR35902::srl(uint8_t& f, uint8_t operand) {
	clearFlag(f, NF | HC | ZF);
	setFlag(f, CF, operand & Bit0);
	return (operand >> 1) & ~Bit7;
}

uint8_t EightBit::GameBoy::LR35902::bit(uint8_t& f, int n, uint8_t operand) {
	auto carry = f & CF;
	uint8_t discarded = operand;
	andr(f, discarded, 1 << n);
	setFlag(f, CF, carry);
	return operand;
}

uint8_t EightBit::GameBoy::LR35902::res(int n, uint8_t operand) {
	return operand & ~(1 << n);
}

uint8_t EightBit::GameBoy::LR35902::set(int n, uint8_t operand) {
	return operand | (1 << n);
}

void EightBit::GameBoy::LR35902::daa(uint8_t& a, uint8_t& f) {

	int updated = a;

	if (f & NF) {
		if (f & HC)
			updated = (updated - 6) & Mask8;
		if (f & CF)
			updated -= 0x60;
	} else {
		if ((f & HC) || lowNibble(updated) > 9)
			updated += 6;
		if ((f & CF) || updated > 0x9F)
			updated += 0x60;
	}

	clearFlag(f, HC | ZF);
	setFlag(f, CF, (f & CF) || (updated & Bit8));
	a = updated & Mask8;

	adjustZero<LR35902>(f, a);
}

void EightBit::GameBoy::LR35902::cpl(uint8_t& a, uint8_t& f) {
	setFlag(f, HC | NF);
	a = ~a;
}

void EightBit::GameBoy::LR35902::scf(uint8_t& a, uint8_t& f) {
	setFlag(f, CF);
	clearFlag(f, HC | NF);
}

void EightBit::GameBoy::LR35902::ccf(uint8_t& a, uint8_t& f) {
	clearFlag(f, NF | HC);
	clearFlag(f, CF, f & CF);
}

int EightBit::GameBoy::LR35902::runRasterLines() {
	m_enabledLCD = !!(m_bus.IO().peek(IoRegisters::LCDC) & IoRegisters::LcdEnable);
	m_bus.IO().resetLY();
	return runRasterLines(Display::RasterHeight * Bus::CyclesPerLine, Display::RasterHeight);
}

int EightBit::GameBoy::LR35902::runRasterLines(int limit, int lines) {
	int count = 0;
	int allowed = Bus::CyclesPerLine;
	for (int line = 0; line < lines; ++line) {
		auto executed = runRasterLine(allowed);
		count += executed;
		allowed = Bus::CyclesPerLine - (executed - Bus::CyclesPerLine);
	}
	return count;
}

int EightBit::GameBoy::LR35902::runRasterLine(int limit) {

	/*
	A scanline normally takes 456 clocks (912 clocks in double speed
	mode) to complete. A scanline starts in mode 2, then goes to
	mode 3 and, when the LCD controller has finished drawing the
	line (the timings depend on lots of things) it goes to mode 0.
	During lines 144-153 the LCD controller is in mode 1.
	Line 153 takes only a few clocks to complete (the exact
	timings are below). The rest of the clocks of line 153 are
	spent in line 0 in mode 1! 

	During mode 0 and mode 1 the CPU can access both VRAM and OAM.
	During mode 2 the CPU can only access VRAM, not OAM.
	During mode 3 OAM and VRAM can't be accessed.
	In GBC mode the CPU can't access Palette RAM(FF69h and FF6Bh)
	during mode 3.
	A scanline normally takes 456 clocks(912 clocks in double speed mode) to complete.
	A scanline starts in mode 2, then goes to mode 3 and , when the LCD controller has
	finished drawing the line(the timings depend on lots of things) it goes to mode 0.
	During lines 144 - 153 the LCD controller is in mode 1.
	Line 153 takes only a few clocks to complete(the exact timings are below).
	The rest of the clocks of line 153 are spent in line 0 in mode 1!
	*/

	int count = 0;
	if (m_enabledLCD) {

		if ((m_bus.IO().peek(IoRegisters::STAT) & Bit6) && (m_bus.IO().peek(IoRegisters::LYC) == m_bus.IO().peek(IoRegisters::LY)))
			m_bus.IO().triggerInterrupt(IoRegisters::Interrupts::DisplayControlStatus);

		// Mode 2, OAM unavailable
		m_bus.IO().updateLcdStatusMode(IoRegisters::LcdStatusMode::SearchingOamRam);
		if (m_bus.IO().peek(IoRegisters::STAT) & Bit5)
			m_bus.IO().triggerInterrupt(IoRegisters::Interrupts::DisplayControlStatus);
		count += run(80);	// ~19us

		// Mode 3, OAM/VRAM unavailable
		m_bus.IO().updateLcdStatusMode(IoRegisters::LcdStatusMode::TransferringDataToLcd);
		count += run(170);	// ~41us

		// Mode 0
		m_bus.IO().updateLcdStatusMode(IoRegisters::LcdStatusMode::HBlank);
		if (m_bus.IO().peek(IoRegisters::STAT) & Bit3)
			m_bus.IO().triggerInterrupt(IoRegisters::Interrupts::DisplayControlStatus);
		count += run(limit - count);	// ~48.6us

		m_bus.IO().incrementLY();

	} else {
		count += run(Bus::CyclesPerLine);
	}

	return count;
}

int EightBit::GameBoy::LR35902::runVerticalBlankLines() {
	auto lines = Bus::TotalLineCount - Display::RasterHeight;
	return runVerticalBlankLines(lines * Bus::CyclesPerLine, lines);
}

int EightBit::GameBoy::LR35902::runVerticalBlankLines(int limit, int lines) {

	/*
	Vertical Blank interrupt is triggered when the LCD
	controller enters the VBL screen mode (mode 1, LY=144).
	This happens once per frame, so this interrupt is
	triggered 59.7 times per second. During this period the
	VRAM and OAM can be accessed freely, so it's the best
	time to update graphics (for example, use the OAM DMA to
	update sprites for next frame, or update tiles to make
	animations).
	This period lasts 4560 clocks in normal speed mode and
	9120 clocks in double speed mode. That's exactly the
	time needed to draw 10 scanlines.
	The VBL interrupt isn't triggered when the LCD is
	powered off or on, even when it was on VBL mode.
	It's only triggered when the VBL period starts. 
	*/

	if (m_enabledLCD) {
		m_bus.IO().updateLcdStatusMode(IoRegisters::LcdStatusMode::VBlank);
		if (m_bus.IO().peek(IoRegisters::STAT) & Bit4)
			m_bus.IO().triggerInterrupt(IoRegisters::Interrupts::DisplayControlStatus);
		m_bus.IO().triggerInterrupt(IoRegisters::Interrupts::VerticalBlank);
	}
	return runRasterLines(limit, lines);
}

int EightBit::GameBoy::LR35902::singleStep() {

	int current = 0;

	auto interruptEnable = m_bus.peek(IoRegisters::BASE + IoRegisters::IE);
	auto interruptFlags = m_bus.IO().peek(IoRegisters::IF);
	auto ime = IME();

	auto masked = interruptEnable & interruptFlags;
	if (masked) {
		if (ime) {
			m_bus.IO().poke(IoRegisters::IF, 0);
		} else {
			if (isHalted())
				proceed();
		}
	}

	if (ime && (masked & IoRegisters::Interrupts::VerticalBlank)) {
		current += interrupt(0x40);
	} else if (ime && (masked & IoRegisters::Interrupts::DisplayControlStatus)) {
		current += interrupt(0x48);
	} else if (ime && (masked & IoRegisters::Interrupts::TimerOverflow)) {
		current += interrupt(0x50);
	} else if (ime && (masked & IoRegisters::Interrupts::SerialTransfer)) {
		current += interrupt(0x58);
	} else if (ime && (masked & IoRegisters::Interrupts::KeypadPressed)) {
		current += interrupt(0x60);
	} else {
		current += isHalted() ? 1 : step();
	}

	m_bus.IO().checkTimers(current);
	m_bus.IO().transferDma();

	return current;
}

int EightBit::GameBoy::LR35902::step() {
	ExecutingInstruction.fire(*this);
	m_prefixCB = false;
	cycles = 0;
	auto ran = fetchExecute();
	ExecutedInstruction.fire(*this);
	return ran;
}

int EightBit::GameBoy::LR35902::execute(uint8_t opcode) {

	const auto& decoded = getDecodedOpcode(opcode);

	auto x = decoded.x;
	auto y = decoded.y;
	auto z = decoded.z;

	auto p = decoded.p;
	auto q = decoded.q;

	if (m_prefixCB)
		executeCB(x, y, z, p, q);
	else
		executeOther(x, y, z, p, q);

	if (cycles == 0)
		throw std::logic_error("Unhandled opcode");

	return clockCycles();
}

void EightBit::GameBoy::LR35902::executeCB(int x, int y, int z, int p, int q) {
	auto& a = A();
	auto& f = F();
	switch (x) {
	case 0:	{ // rot[y] r[z]
		auto operand = R(z, a);
		switch (y) {
		case 0:
			operand = rlc(f, operand);
			break;
		case 1:
			operand = rrc(f, operand);
			break;
		case 2:
			operand = rl(f, operand);
			break;
		case 3:
			operand = rr(f, operand);
			break;
		case 4:
			operand = sla(f, operand);
			break;
		case 5:
			operand = sra(f, operand);
			break;
		case 6:	// GB: SWAP r
			operand = swap(f, operand);
			break;
		case 7:
			operand = srl(f, operand);
			break;
		default:
			UNREACHABLE;
		}
		cycles += 2;
		R(z, a, operand);
		adjustZero<LR35902>(f, operand);
		if (z == 6)
			cycles += 2;
		break;
	} case 1:	// BIT y, r[z]
		bit(f, y, R(z, a));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	case 2:	// RES y, r[z]
		R(z, a, res(y, R(z, a)));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	case 3:	// SET y, r[z]
		R(z, a, set(y, R(z, a)));
		cycles += 2;
		if (z == 6)
			cycles += 2;
		break;
	default:
		UNREACHABLE;
	}
}

void EightBit::GameBoy::LR35902::executeOther(int x, int y, int z, int p, int q) {
	auto& a = A();
	auto& f = F();
	switch (x) {
	case 0:
		switch (z) {
		case 0:	// Relative jumps and assorted ops
			switch (y) {
			case 0:	// NOP
				cycles++;
				break;
			case 1:	// GB: LD (nn),SP
				fetchWord();
				setWordViaMemptr(SP());
				cycles += 5;
				break;
			case 2:	// GB: STOP
				stop();
				cycles++;
				break;
			case 3:	// JR d
				jr(fetchByte());
				cycles += 4;
				break;
			case 4: // JR cc,d
			case 5:
			case 6:
			case 7:
				if (jrConditionalFlag(f, y - 4))
					cycles++;
				cycles += 2;
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// 16-bit load immediate/add
			switch (q) {
			case 0: // LD rp,nn
				fetchWord(RP(p));
				cycles += 3;
				break;
			case 1:	// ADD HL,rp
				add(f, HL(), RP(p));
				cycles += 2;
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
					setByte(BC(), a);
					cycles += 2;
					break;
				case 1:	// LD (DE),A
					setByte(DE(), a);
					cycles += 2;
					break;
				case 2:	// GB: LDI (HL),A
					setByte(HL().word++, a);
					cycles += 2;
					break;
				case 3: // GB: LDD (HL),A
					setByte(HL().word--, a);
					cycles += 2;
					break;
				default:
					UNREACHABLE;
				}
				break;
			case 1:
				switch (p) {
				case 0:	// LD A,(BC)
					a = getByte(BC());
					cycles += 2;
					break;
				case 1:	// LD A,(DE)
					a = getByte(DE());
					cycles += 2;
					break;
				case 2:	// GB: LDI A,(HL)
					a = getByte(HL().word++);
					cycles += 2;
					break;
				case 3:	// GB: LDD A,(HL)
					a = getByte(HL().word--);
					cycles += 2;
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
				++RP(p).word;
				break;
			case 1:	// DEC rp
				--RP(p).word;
				break;
			default:
				UNREACHABLE;
			}
			cycles += 2;
			break;
		case 4: { // 8-bit INC
			auto operand = R(y, a);
			increment(f, operand);
			R(y, a, operand);
			cycles++;
			if (y == 6)
				cycles += 2;
			break;
		} case 5: {	// 8-bit DEC
			auto operand = R(y, a);
			decrement(f, operand);
			R(y, a, operand);
			cycles++;
			if (y == 6)
				cycles += 2;
			break;
		} case 6:	// 8-bit load immediate
			R(y, a, fetchByte());	// LD r,n
			cycles += 2;
			break;
		case 7:	// Assorted operations on accumulator/flags
			switch (y) {
			case 0:
				a = rlc(f, a);
				break;
			case 1:
				a = rrc(f, a);
				break;
			case 2:
				a = rl(f, a);
				break;
			case 3:
				a = rr(f, a);
				break;
			case 4:
				daa(a, f);
				break;
			case 5:
				cpl(a, f);
				break;
			case 6:
				scf(a, f);
				break;
			case 7:
				ccf(a, f);
				break;
			default:
				UNREACHABLE;
			}
			cycles++;
			break;
		default:
			UNREACHABLE;
		}
		break;
	case 1:	// 8-bit loading
		if (z == 6 && y == 6) { 	// Exception (replaces LD (HL), (HL))
			halt();
		} else {
			R(y, a, R(z, a));
			if ((y == 6) || (z == 6)) // M operations
				cycles++;
		}
		cycles++;
		break;
	case 2:	// Operate on accumulator and register/memory location
		switch (y) {
		case 0:	// ADD A,r
			add(f, a, R(z, a));
			break;
		case 1:	// ADC A,r
			adc(f, a, R(z, a));
			break;
		case 2:	// SUB r
			subtract(f, a, R(z, a));
			break;
		case 3:	// SBC A,r
			sbc(f, a, R(z, a));
			break;
		case 4:	// AND r
			andr(f, a, R(z, a));
			break;
		case 5:	// XOR r
			xorr(f, a, R(z, a));
			break;
		case 6:	// OR r
			orr(f, a, R(z, a));
			break;
		case 7:	// CP r
			compare(f, a, R(z, a));
			break;
		default:
			UNREACHABLE;
		}
		cycles++;
		if (z == 6)
			cycles++;
		break;
	case 3:
		switch (z) {
		case 0:	// Conditional return
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				if (returnConditionalFlag(f, y))
					cycles += 3;
				cycles += 2;
				break;
			case 4:	// GB: LD (FF00 + n),A
				m_bus.write(IoRegisters::BASE + fetchByte(), a);
				cycles += 3;
				break;
			case 5: { // GB: ADD SP,dd
					auto before = SP().word;
					int8_t value = fetchByte();
					auto result = before + value;
					SP().word = result;
					auto carried = before ^ value ^ (result & Mask16);
					clearFlag(f, ZF | NF);
					setFlag(f, CF, carried & Bit8);
					setFlag(f, HC, carried & Bit4);
				}
				cycles += 4;
				break;
			case 6:	// GB: LD A,(FF00 + n)
				a = m_bus.read(IoRegisters::BASE + fetchByte());
				cycles += 3;
				break;
			case 7: { // GB: LD HL,SP + dd
					auto before = SP().word;
					int8_t value = fetchByte();
					auto result = before + value;
					HL().word = result;
					auto carried = before ^ value ^ (result & Mask16);
					clearFlag(f, ZF | NF);
					setFlag(f, CF, carried & Bit8);
					setFlag(f, HC, carried & Bit4);
				}
				cycles += 3;
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 1:	// POP & various ops
			switch (q) {
			case 0:	// POP rp2[p]
				popWord(RP2(p));
				cycles += 3;
				break;
			case 1:
				switch (p) {
				case 0:	// RET
					ret();
					cycles += 4;
					break;
				case 1:	// GB: RETI
					reti();
					cycles += 4;
					break;
				case 2:	// JP HL
					PC() = HL();
					cycles++;
					break;
				case 3:	// LD SP,HL
					SP() = HL();
					cycles += 2;
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
			switch (y) {
			case 0:
			case 1:
			case 2:
			case 3:
				jumpConditionalFlag(f, y);
				cycles += 3;
				break;
			case 4:	// GB: LD (FF00 + C),A
				m_bus.write(IoRegisters::BASE + C(), a);
				cycles += 2;
				break;
			case 5:	// GB: LD (nn),A
				fetchWord();
				setByte(MEMPTR(), a);
				cycles += 4;
				break;
			case 6:	// GB: LD A,(FF00 + C)
				a = m_bus.read(IoRegisters::BASE + C());
				cycles += 2;
				break;
			case 7:	// GB: LD A,(nn)
				fetchWord();
				a = getByte(MEMPTR());
				cycles += 4;
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 3:	// Assorted operations
			switch (y) {
			case 0:	// JP nn
				fetchWord();
				jump();
				cycles += 4;
				break;
			case 1:	// CB prefix
				m_prefixCB = true;
				fetchExecute();
				break;
			case 6:	// DI
				di();
				cycles++;
				break;
			case 7:	// EI
				ei();
				cycles++;
				break;
			}
			break;
		case 4:	// Conditional call: CALL cc[y], nn
			if (callConditionalFlag(f, y))
				cycles += 3;
			cycles += 3;
			break;
		case 5:	// PUSH & various ops
			switch (q) {
			case 0:	// PUSH rp2[p]
				pushWord(RP2(p));
				cycles += 4;
				break;
			case 1:
				switch (p) {
				case 0:	// CALL nn
					fetchWord();
					call();
					cycles += 6;
					break;
				}
				break;
			default:
				UNREACHABLE;
			}
			break;
		case 6:	// Operate on accumulator and immediate operand: alu[y] n
			switch (y) {
			case 0:	// ADD A,n
				add(f, a, fetchByte());
				break;
			case 1:	// ADC A,n
				adc(f, a, fetchByte());
				break;
			case 2:	// SUB n
				subtract(f, a, fetchByte());
				break;
			case 3:	// SBC A,n
				sbc(f, a, fetchByte());
				break;
			case 4:	// AND n
				andr(f, a, fetchByte());
				break;
			case 5:	// XOR n
				xorr(f, a, fetchByte());
				break;
			case 6:	// OR n
				orr(f, a, fetchByte());
				break;
			case 7:	// CP n
				compare(f, a, fetchByte());
				break;
			default:
				UNREACHABLE;
			}
			cycles += 2;
			break;
		case 7:	// Restart: RST y * 8
			restart(y << 3);
			cycles += 4;
			break;
		default:
			UNREACHABLE;
		}
		break;
	}
}
