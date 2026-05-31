#include "stdafx.h"
#include "../inc/mos6502.h"

EightBit::MOS6502::MOS6502(Bus& bus) noexcept
: base(bus) {
	RaisedPOWER.connect([this](EventArgs) {
		X() = Bit7;
		Y() = 0;
		A() = 0;
		P() = RF;
		S() = Mask8;
		lowerSYNC();
		lowerRW();
	});
}

DEFINE_PIN_LEVEL_CHANGERS(NMI, MOS6502)
DEFINE_PIN_LEVEL_CHANGERS(SO, MOS6502)
DEFINE_PIN_LEVEL_CHANGERS(SYNC, MOS6502)
DEFINE_PIN_LEVEL_CHANGERS(RDY, MOS6502)
DEFINE_PIN_LEVEL_CHANGERS(RW, MOS6502)

void EightBit::MOS6502::poweredStep() noexcept {

	// A cycle is used, whether RDY is high or not
	tick();

	if (lowered(SO()))
		handleSO();

	if (raised(RDY())) {

		m_immediateInstruction = false;
		opcode() = fetchInstruction();

		// Priority: RESET > NMI > INT
		if (lowered(RESET()))
			handleRESET();
		else if (lowered(NMI()))
			handleNMI();
		else if (lowered(INT()) && !interruptMasked())
			handleINT();
		else
			execute();
	}
}

uint8_t EightBit::MOS6502::fetchInstruction() noexcept {

	// Instruction fetch beginning
	lowerSYNC();

	assert(cycles() == 1 && "An extra cycle has occurred");

	// Can't use fetchByte, since that would add an extra tick.
	immediateAddress();

	ReadingMemory.fire();
		readFromMemory();
	ReadMemory.fire();

	assert(cycles() == 1 && "BUS read has introduced stray cycles");

	// Instruction fetch has now completed
	raiseSYNC();

	return BUS().DATA();
}

// Interrupt (etc.) handlers

void EightBit::MOS6502::handleRESET() noexcept {
	raiseRESET();
	reset();
}

void EightBit::MOS6502::handleINT() noexcept {
	raiseINT();
	interruptMaskable();
}

void EightBit::MOS6502::handleNMI() noexcept {
	raiseNMI();
	interruptNonMaskable();
}

void EightBit::MOS6502::handleSO() noexcept {
	raiseSO();
	SEV();
}

void EightBit::MOS6502::adjustInterruptFlags() {
	SEI();   // Disable IRQ
}

void EightBit::MOS6502::reset() {
	dummyPush();
	dummyPush();
	dummyPush();
	getPagedInto(0xff, _vectorRST, PC());
	adjustInterruptFlags();
}

void EightBit::MOS6502::interruptMaskable() {
	pushShort(PC());
	push(P());
	getPagedInto(0xff, _vectorIRQ, PC());
	adjustInterruptFlags();
}

void EightBit::MOS6502::interruptNonMaskable() {
	pushShort(PC());
	push(P());
	getPagedInto(0xff, _vectorNMI, PC());
	adjustInterruptFlags();
}

void EightBit::MOS6502::BRK() noexcept {
	pushShort(PC());
	push(P() | BF);
	getPagedInto(0xff, _vectorIRQ, PC());
	adjustInterruptFlags();
}

void EightBit::MOS6502::swallowRead() noexcept {
	base::memoryRead(PC());
}

void EightBit::MOS6502::swallowPop() noexcept {
	base::memoryRead({ S(), 1 });
}

void EightBit::MOS6502::swallowFetch() noexcept {
	fetchByte();
}

void EightBit::MOS6502::memoryWrite() noexcept {
	WritingMemory.fire();
		tick();
		writeToMemory();
	WrittenMemory.fire();
}

void EightBit::MOS6502::memoryRead() noexcept {
	ReadingMemory.fire();
		tick();
		readFromMemory();
	ReadMemory.fire();
}

void EightBit::MOS6502::readFromMemory() noexcept {
	raiseRW();
	base::memoryRead();
}

void EightBit::MOS6502::writeToMemory() noexcept {
	lowerRW();
	base::memoryWrite();
}

void EightBit::MOS6502::modifyWrite(uint8_t data) noexcept {
	// The read will have already taken place...
	memoryWrite();					// Modify cycle
	Processor::memoryWrite(data);	// Write cycle
}

//

#pragma region Address page fixup

void EightBit::MOS6502::maybeFixup() noexcept {
	if (BUS().ADDRESS().high != fixedPage())
		fixup();
}

void EightBit::MOS6502::fixup() noexcept {
	memoryRead();
	BUS().ADDRESS().high = fixedPage();
}

void EightBit::MOS6502::maybeFixupRead() noexcept {
	maybeFixup();
	memoryRead();
}

void EightBit::MOS6502::fixupRead() noexcept {
	fixup();
	memoryRead();
}

#pragma endregion

#pragma region Address resolution

void EightBit::MOS6502::getAddressPaged() noexcept {
	getShortPaged();
	BUS().ADDRESS() = intermediate();
}

void EightBit::MOS6502::absoluteAddress() noexcept {
	fetchShortAddress();
}

void EightBit::MOS6502::zeroPageAddress() noexcept {
	fetchByte();
	BUS().ADDRESS() = BUS().DATA();
}

void EightBit::MOS6502::zeroPageIndirectAddress() noexcept {
	zeroPageAddress();
	getAddressPaged();
}

void EightBit::MOS6502::indirectAddress() noexcept {
	absoluteAddress();
	getAddressPaged();
}

void EightBit::MOS6502::zeroPageWithIndexAddress(uint8_t index) noexcept {
	zeroPage();
	BUS().ADDRESS().low += index;
}

void EightBit::MOS6502::zeroPageXAddress() noexcept {
	zeroPageWithIndexAddress(X());
}

void EightBit::MOS6502::zeroPageYAddress() noexcept {
	zeroPageWithIndexAddress(Y());
}

void EightBit::MOS6502::absoluteWithIndexAddress(uint8_t index) noexcept {
	absoluteAddress();
	noteFixedAddress(BUS().ADDRESS().joined + index);
}

void EightBit::MOS6502::absoluteXAddress() noexcept {
	absoluteWithIndexAddress(X());
}

void EightBit::MOS6502::absoluteYAddress() noexcept {
	absoluteWithIndexAddress(Y());
}

void EightBit::MOS6502::indexedIndirectXAddress() noexcept {
	zeroPageXAddress();
	getAddressPaged();
}

void EightBit::MOS6502::indirectIndexedYAddress() noexcept {
	zeroPageIndirectAddress();
	noteFixedAddress(BUS().ADDRESS().joined + Y());
}

#pragma endregion

#pragma region Address and read

void EightBit::MOS6502::immediate() noexcept {
	m_immediateInstruction = true;
	fetchByte();
}

void EightBit::MOS6502::absolute() noexcept {
	absoluteAddress();
	memoryRead();
}

void EightBit::MOS6502::zeroPage() noexcept {
	zeroPageAddress();
	memoryRead();
}

void EightBit::MOS6502::zeroPageX() noexcept {
	zeroPageXAddress();
	memoryRead();
}

void EightBit::MOS6502::zeroPageY() noexcept {
	zeroPageYAddress();
	memoryRead();
}

void EightBit::MOS6502::indexedIndirectX() noexcept {
	indexedIndirectXAddress();
	memoryRead();
}

void EightBit::MOS6502::absoluteX() noexcept {
	absoluteXAddress();
	maybeFixupRead();
}

void EightBit::MOS6502::absoluteY() noexcept {
	absoluteYAddress();
	maybeFixupRead();
}

void EightBit::MOS6502::indirectIndexedY() noexcept {
	indirectIndexedYAddress();
	maybeFixupRead();
}

#pragma endregion

//

void EightBit::MOS6502::execute() noexcept {

	switch (opcode()) {

	case 0x00:	swallowFetch(); BRK();							break;	// BRK (implied)
	case 0x01:	indexedIndirectX(); ORA();						break;	// ORA (indexed indirect X)
	case 0x02:	swallowRead(); JAM();							break;	// *JAM
	case 0x03:	indexedIndirectX(); SLO();						break;	// *SLO (indexed indirect X)
	case 0x04:	zeroPage();	NOP();								break;	// *NOP (zero page)
	case 0x05:	zeroPage(); ORA();								break;	// ORA (zero page)
	case 0x06:	zeroPage(); ASL();								break;	// ASL (zero page)
	case 0x07:	zeroPage(); SLO();								break;	// *SLO (zero page)
	case 0x08:	swallowRead(); PHP();							break;	// PHP (implied)
	case 0x09:	immediate(); ORA();								break;	// ORA (immediate)
	case 0x0a:	swallowRead(); ASLA();							break;	// ASL A (implied)
	case 0x0b:	immediate(); ANC();								break;	// *ANC (immediate)
	case 0x0c:	absolute(); NOP(); 								break;	// *NOP (absolute)
	case 0x0d:	absolute(); ORA();								break;	// ORA (absolute)
	case 0x0e:	absolute(); ASL();								break;	// ASL (absolute)
	case 0x0f:	absolute(); SLO();								break;	// *SLO (absolute)

	case 0x10:	immediate(); BPL();								break;	// BPL (relative)
	case 0x11:	indirectIndexedY(); ORA();						break;	// ORA (indirect indexed Y)
	case 0x12:	swallowRead(); JAM();							break;	// *JAM
	case 0x13:	indirectIndexedYAddress(); fixupRead(); SLO();	break;	// *SLO (indirect indexed Y)
	case 0x14:	zeroPageX(); NOP();								break;	// *NOP (zero page, X)
	case 0x15:	zeroPageX(); ORA();								break;	// ORA (zero page, X)
	case 0x16:	zeroPageX(); ASL();								break;	// ASL (zero page, X)
	case 0x17:	zeroPageX(); SLO();								break;	// *SLO (zero page, X)
	case 0x18:	swallowRead(); resetFlag(CF);					break;	// CLC (implied)
	case 0x19:	absoluteY(); ORA();								break;	// ORA (absolute, Y)
	case 0x1a:	swallowRead(); NOP();							break;	// *NOP (implied)
	case 0x1b:	absoluteYAddress(); fixupRead(); SLO();			break;	// *SLO (absolute, Y)
	case 0x1c:	absoluteXAddress(); maybeFixupRead();			break;	// *NOP (absolute, X)
	case 0x1d:	absoluteX(); ORA();								break;	// ORA (absolute, X)
	case 0x1e:	absoluteXAddress(); fixupRead(); ASL();			break;	// ASL (absolute, X)
	case 0x1f:	absoluteXAddress(); fixupRead(); SLO();			break;	// *SLO (absolute, X)

	case 0x20:	JSR();											break;	// JSR (absolute)
	case 0x21:	indexedIndirectX(); AND();						break;	// AND (indexed indirect X)
	case 0x22:	swallowRead(); JAM();							break;	// *JAM
	case 0x23:	indexedIndirectX(); RLA();;						break;	// *RLA (indexed indirect X)
	case 0x24:	zeroPage(); BIT();								break;	// BIT (zero page)
	case 0x25:	zeroPage(); AND();								break;	// AND (zero page)
	case 0x26:	zeroPage(); ROL();								break;	// ROL (zero page)
	case 0x27:	zeroPage(); RLA();;								break;	// *RLA (zero page)
	case 0x28:	swallowRead(); PLP();							break;	// PLP (implied)
	case 0x29:	immediate(); AND();								break;	// AND (immediate)
	case 0x2a:	swallowRead(); ROLA();							break;	// ROL A (implied)
	case 0x2b:	immediate(); ANC();								break;	// *ANC (immediate)
	case 0x2c:	absolute(); BIT();								break;	// BIT (absolute)
	case 0x2d:	absolute(); AND();								break;	// AND (absolute)
	case 0x2e:	absolute(); ROL();								break;	// ROL (absolute)
	case 0x2f:	absolute(); RLA();;								break;	// *RLA (absolute)

	case 0x30:	immediate(); BMI();								break;	// BMI (relative)
	case 0x31:	indirectIndexedY(); AND();						break;	// AND (indirect indexed Y)
	case 0x32:	swallowRead(); JAM();							break;	// *JAM
	case 0x33:	indirectIndexedYAddress(); fixupRead(); RLA();	break;	// *RLA (indirect indexed Y)
	case 0x34:	zeroPageX(); NOP();								break;	// *NOP (zero page, X)
	case 0x35:	zeroPageX(); AND();								break;	// AND (zero page, X)
	case 0x36:	zeroPageX(); ROL();								break;	// ROL (zero page, X)
	case 0x37:	zeroPageX(); RLA();;							break;	// *RLA (zero page, X)
	case 0x38:	swallowRead(); SEC();							break;	// SEC (implied)
	case 0x39:	absoluteY(); AND();								break;	// AND (absolute, Y)
	case 0x3a:	swallowRead(); NOP();							break;	// *NOP (implied)
	case 0x3b:	absoluteYAddress(); fixupRead(); RLA();			break;	// *RLA (absolute, Y)
	case 0x3c:	absoluteXAddress(); maybeFixupRead(); NOP();	break;	// *NOP (absolute, X)
	case 0x3d:	absoluteX(); AND();								break;	// AND (absolute, X)
	case 0x3e:	absoluteXAddress(); fixupRead(); ROL();			break;	// ROL (absolute, X)
	case 0x3f:	absoluteXAddress(); fixupRead(); RLA();         break;	// *RLA (absolute, X)

	case 0x40:	swallowRead(); RTI();							break;	// RTI (implied)
	case 0x41:	indexedIndirectX(); EOR();						break;	// EOR (indexed indirect X)
	case 0x42:	swallowRead(); JAM();							break;	// *JAM
	case 0x43:	indexedIndirectX(); SRE();						break;	// *SRE (indexed indirect X)
	case 0x44:	zeroPage();	NOP();								break;	// *NOP (zero page)
	case 0x45:	zeroPage(); EOR();								break;	// EOR (zero page)
	case 0x46:	zeroPage(); LSR();								break;	// LSR (zero page)
	case 0x47:	zeroPage(); SRE();								break;	// *SRE (zero page)
	case 0x48:	swallowRead(); PHA();							break;	// PHA (implied)
	case 0x49:	immediate(); EOR();								break;	// EOR (immediate)
	case 0x4a:	swallowRead(); LSRA();							break;	// LSR A (implied)
	case 0x4b:	immediate(); ASR();								break;	// *ASR (immediate)
	case 0x4c:	absoluteAddress(); JMP();						break;	// JMP (absolute)
	case 0x4d:	absolute(); EOR();								break;	// EOR (absolute)
	case 0x4e:	absolute(); LSR(); 								break;	// LSR (absolute)
	case 0x4f:	absolute(); SRE();								break;	// *SRE (absolute)

	case 0x50:	immediate(); BVC();								break;	// BVC (relative)
	case 0x51:	indirectIndexedY(); EOR();						break;	// EOR (indirect indexed Y)
	case 0x52:	swallowRead(); JAM();							break;	// *JAM
	case 0x53:	indirectIndexedYAddress(); fixupRead(); SRE();	break;	// *SRE (indirect indexed Y)
	case 0x54:	zeroPageX(); NOP();								break;	// *NOP (zero page, X)
	case 0x55:	zeroPageX(); EOR();								break;	// EOR (zero page, X)
	case 0x56:	zeroPageX(); LSR(); 							break;	// LSR (zero page, X)
	case 0x57:	zeroPageX(); SRE();								break;	// *SRE (zero page, X)
	case 0x58:	swallowRead(); CLI();							break;	// CLI (implied)
	case 0x59:	absoluteY(); EOR();								break;	// EOR (absolute, Y)
	case 0x5a:	swallowRead(); NOP();							break;	// *NOP (implied)
	case 0x5b:	absoluteYAddress(); fixupRead(); SRE();			break;	// *SRE (absolute, Y)
	case 0x5c:	absoluteXAddress(); maybeFixupRead(); NOP();	break;	// *NOP (absolute, X)
	case 0x5d:	absoluteX(); EOR();								break;	// EOR (absolute, X)
	case 0x5e:	absoluteXAddress(); fixupRead(); LSR();			break;	// LSR (absolute, X)
	case 0x5f:	absoluteXAddress(); fixupRead(); SRE();			break;	// *SRE (absolute, X)

	case 0x60:	swallowRead(); RTS();							break;	// RTS (implied)
	case 0x61:	indexedIndirectX(); ADC();						break;	// ADC (indexed indirect X)
	case 0x62:	swallowRead(); JAM();							break;	// *JAM
	case 0x63:	indexedIndirectX(); RRA();						break;	// *RRA (indexed indirect X)
	case 0x64:	zeroPage();	NOP();								break;	// *NOP (zero page)
	case 0x65:	zeroPage(); ADC();								break;	// ADC (zero page)
	case 0x66:	zeroPage(); ROR();								break;	// ROR (zero page)
	case 0x67:	zeroPage(); RRA();								break;	// *RRA (zero page)
	case 0x68:	swallowRead(); PLA();							break;	// PLA (implied)
	case 0x69:	immediate(); ADC();								break;	// ADC (immediate)
	case 0x6a:	swallowRead(); RORA();							break;	// ROR A (implied)
	case 0x6b:	immediate(); ARR();								break;	// *ARR (immediate)
	case 0x6c:	indirectAddress(); JMP();						break;	// JMP (indirect)
	case 0x6d:	absolute(); ADC();								break;	// ADC (absolute)
	case 0x6e:	absolute(); ROR(); 								break;	// ROR (absolute)
	case 0x6f:	absolute(); RRA();								break;	// *RRA (absolute)

	case 0x70:	immediate(); BVS();								break;	// BVS (relative)
	case 0x71:	indirectIndexedY(); ADC();						break;	// ADC (indirect indexed Y)
	case 0x72:	swallowRead(); JAM();							break;	// *JAM
	case 0x73:	indirectIndexedYAddress(); fixupRead(); RRA();	break;	// *RRA (indirect indexed Y)
	case 0x74:	zeroPageX(); NOP();								break;	// *NOP (zero page, X)
	case 0x75:	zeroPageX(); ADC();								break;	// ADC (zero page, X)
	case 0x76:	zeroPageX(); ROR(); 							break;	// ROR (zero page, X)
	case 0x77:	zeroPageX(); RRA();								break;	// *RRA (zero page, X)
	case 0x78:	swallowRead(); SEI();							break;	// SEI (implied)
	case 0x79:	absoluteY(); ADC();								break;	// ADC (absolute, Y)
	case 0x7a:	swallowRead(); NOP();							break;	// *NOP (implied)
	case 0x7b:	absoluteYAddress(); fixupRead(); RRA();			break;	// *RRA (absolute, Y)
	case 0x7c:	absoluteXAddress(); maybeFixupRead(); NOP();	break;	// *NOP (absolute, X)
	case 0x7d:	absoluteX(); ADC();								break;	// ADC (absolute, X)
	case 0x7e:	absoluteXAddress(); fixupRead(); ROR();			break;	// ROR (absolute, X)
	case 0x7f:	absoluteXAddress(); fixupRead(); RRA();			break;	// *RRA (absolute, X)

	case 0x80:	immediate(); NOP();								break;	// *NOP (immediate)
	case 0x81:	indexedIndirectXAddress(); STA();				break;	// STA (indexed indirect X)
	case 0x82:	immediate(); NOP();								break;	// *NOP (immediate)
	case 0x83:	indexedIndirectXAddress(); SAX();				break;	// *SAX (indexed indirect X)
	case 0x84:	zeroPageAddress(); STY();						break;	// STY (zero page)
	case 0x85:	zeroPageAddress(); STA();						break;	// STA (zero page)
	case 0x86:	zeroPageAddress(); STX();						break;	// STX (zero page)
	case 0x87:	zeroPageAddress(); SAX();						break;	// *SAX (zero page)
	case 0x88:	swallowRead(); DEY();							break;	// DEY (implied)
	case 0x89:	immediate(); NOP();								break;	// *NOP (immediate)
	case 0x8a:	swallowRead(); TXA();							break;	// TXA (implied)
	case 0x8b:	immediate(); ANE();								break;	// *ANE (immediate)
	case 0x8c:	absoluteAddress(); STY();						break;	// STY (absolute)
	case 0x8d:	absoluteAddress(); STA();						break;	// STA (absolute)
	case 0x8e:	absoluteAddress(); STX();						break;	// STX (absolute)
	case 0x8f:	absoluteAddress(); SAX();						break;	// *SAX (absolute)

	case 0x90:	immediate(); BCC();								break;	// BCC (relative)
	case 0x91:	indirectIndexedYAddress(); fixup(); STA();		break;	// STA (indirect indexed Y)
	case 0x92:	swallowRead(); JAM();							break;	// *JAM
	case 0x93:	indirectIndexedYAddress(); fixup(); SHA();		break;	// *SHA (indirect indexed, Y)
	case 0x94:	zeroPageXAddress(); STY();						break;	// STY (zero page, X)
	case 0x95:	zeroPageXAddress(); STA();						break;	// STA (zero page, X)
	case 0x96:	zeroPageYAddress(); STX();						break;	// STX (zero page, Y)
	case 0x97:	zeroPageYAddress(); SAX();						break;	// *SAX (zero page, Y)
	case 0x98:	swallowRead(); TYA();							break;	// TYA (implied)
	case 0x99:	absoluteYAddress(); fixup(); STA();				break;	// STA (absolute, Y)
	case 0x9a:	swallowRead(); TXS();							break;	// TXS (implied)
	case 0x9b:	absoluteYAddress(); fixup(); TAS();				break;	// *TAS (absolute, Y)
	case 0x9c:	absoluteXAddress(); fixup(); SYA();				break;	// *SYA (absolute, X)
	case 0x9d:	absoluteXAddress(); fixup(); STA();				break;	// STA (absolute, X)
	case 0x9e:	absoluteYAddress(); fixup(); SXA();				break;	// *SXA (absolute, Y)
	case 0x9f:	absoluteYAddress(); fixup(); SHA();				break;	// *SHA (absolute, Y)

	case 0xa0:	immediate(); LDY();								break;	// LDY (immediate)
	case 0xa1:	indexedIndirectX(); LDA();						break;	// LDA (indexed indirect X)
	case 0xa2:	immediate(); LDX();								break;	// LDX (immediate)
	case 0xa3:	indexedIndirectX(); LAX();						break;	// *LAX (indexed indirect X)
	case 0xa4:	zeroPage(); LDY();								break;	// LDY (zero page)
	case 0xa5:	zeroPage(); LDA();								break;	// LDA (zero page)
	case 0xa6:	zeroPage(); LDX();								break;	// LDX (zero page)
	case 0xa7:	zeroPage(); LAX();								break;	// *LAX (zero page)
	case 0xa8:	swallowRead(); TAY();							break;	// TAY (implied)
	case 0xa9:	immediate(); LDA();								break;	// LDA (immediate)
	case 0xaa:	swallowRead(); TAX();							break;	// TAX (implied)
	case 0xab:	immediate(); ATX();								break;	// *ATX (immediate)
	case 0xac:	absolute(); LDY();								break;	// LDY (absolute)
	case 0xad:	absolute(); LDA();								break;	// LDA (absolute)
	case 0xae:	absolute(); LDX();								break;	// LDX (absolute)
	case 0xaf:	absolute(); LAX();								break;	// *LAX (absolute)

	case 0xb0:	immediate(); BCS();								break;	// BCS (relative)
	case 0xb1:	indirectIndexedY(); LDA();						break;	// LDA (indirect indexed Y)
	case 0xb2:	swallowRead(); JAM();							break;	// *JAM
	case 0xb3:	indirectIndexedY(); LAX();						break;	// *LAX (indirect indexed Y)
	case 0xb4:	zeroPageX(); LDY();								break;	// LDY (zero page, X)
	case 0xb5:	zeroPageX(); LDA();								break;	// LDA (zero page, X)
	case 0xb6:	zeroPageY(); LDX();								break;	// LDX (zero page, Y)
	case 0xb7:	zeroPageY(); LAX();								break;	// *LAX (zero page, Y)
	case 0xb8:	swallowRead(); CLV();							break;	// CLV (implied)
	case 0xb9:	absoluteY(); LDA();								break;	// LDA (absolute, Y)
	case 0xba:	swallowRead(); TSX();							break;	// TSX (implied)
	case 0xbb:	absoluteYAddress(); maybeFixupRead(); LAS();	break;	// *LAS (absolute, Y)
	case 0xbc:	absoluteX(); LDY();								break;	// LDY (absolute, X)
	case 0xbd:	absoluteX(); LDA();								break;	// LDA (absolute, X)
	case 0xbe:	absoluteY(); LDX();								break;	// LDX (absolute, Y)
	case 0xbf:	absoluteY(); LAX();								break;	// *LAX (absolute, Y)

	case 0xc0:	immediate(); CPY();								break;	// CPY (immediate)
	case 0xc1:	indexedIndirectX(); CMP();						break;	// CMP (indexed indirect X)
	case 0xc2:	immediate(); NOP();								break;	// *NOP (immediate)
	case 0xc3:	indexedIndirectX(); DCP();						break;	// *DCP (indexed indirect X)
	case 0xc4:	zeroPage(); CPY();								break;	// CPY (zero page)
	case 0xc5:	zeroPage(); CMP();								break;	// CMP (zero page)
	case 0xc6:	zeroPage(); DEC();								break;	// DEC (zero page)
	case 0xc7:	zeroPage(); DCP();								break;	// *DCP (zero page)
	case 0xc8:	swallowRead(); INY();							break;	// INY (implied)
	case 0xc9:	immediate(); CMP();								break;	// CMP (immediate)
	case 0xca:	swallowRead(); DEX();							break;	// DEX (implied)
	case 0xcb:	immediate(); AXS();								break;	// *AXS (immediate)
	case 0xcc:	absolute(); CPY();								break;	// CPY (absolute)
	case 0xcd:	absolute(); CMP();								break;	// CMP (absolute)
	case 0xce:	absolute(); DEC();								break;	// DEC (absolute)
	case 0xcf:	absolute(); DCP();								break;	// *DCP (absolute)

	case 0xd0:	immediate(); BNE();								break;	// BNE (relative)
	case 0xd1:	indirectIndexedY(); CMP();						break;	// CMP (indirect indexed Y)
	case 0xd2:	swallowRead(); JAM();							break;	// *JAM
	case 0xd3:	indirectIndexedYAddress(); fixupRead(); DCP();	break;	// *DCP (indirect indexed Y)
	case 0xd4:	zeroPageX(); NOP();								break;	// *NOP (zero page, X)
	case 0xd5:	zeroPageX(); CMP();								break;	// CMP (zero page, X)
	case 0xd6:	zeroPageX(); DEC();								break;	// DEC (zero page, X)
	case 0xd7:	zeroPageX(); DCP();								break;	// *DCP (zero page, X)
	case 0xd8:	swallowRead(); CLD();							break;	// CLD (implied)
	case 0xd9:	absoluteY(); CMP();								break;	// CMP (absolute, Y)
	case 0xda:	swallowRead(); NOP();							break;	// *NOP (implied)
	case 0xdb:	absoluteYAddress(); fixupRead(); DCP();			break;	// *DCP (absolute, Y)
	case 0xdc:	absoluteXAddress(); maybeFixupRead(); NOP();	break;	// *NOP (absolute, X)
	case 0xdd:	absoluteX(); CMP();								break;	// CMP (absolute, X)
	case 0xde:	absoluteXAddress(); fixupRead(); DEC();			break;	// DEC (absolute, X)
	case 0xdf:	absoluteXAddress(); fixupRead(); DCP();			break;	// *DCP (absolute, X)

	case 0xe0:	immediate(); CPX();								break;	// CPX (immediate)
	case 0xe1:	indexedIndirectX(); SBC();						break;	// SBC (indexed indirect X)
	case 0xe2:	immediate(); NOP();								break;	// *NOP (immediate)
	case 0xe3:	indexedIndirectX(); ISB();						break;	// *ISB (indexed indirect X)
	case 0xe4:	zeroPage(); CPX();								break;	// CPX (zero page)
	case 0xe5:	zeroPage(); SBC();								break;	// SBC (zero page)
	case 0xe6:	zeroPage(); INC();								break;	// INC (zero page)
	case 0xe7:	zeroPage(); ISB();								break;	// *ISB (zero page)
	case 0xe8:	swallowRead(); INX();							break;	// INX (implied)
	case 0xe9:	immediate(); SBC();								break;	// SBC (immediate)
	case 0xea:	swallowRead(); NOP();							break;	// NOP (implied)
	case 0xeb:	immediate(); SBC();								break;	// *SBC (immediate)
	case 0xec:	absolute(); CPX();								break;	// CPX (absolute)
	case 0xed:	absolute(); SBC();								break;	// SBC (absolute)
	case 0xee:	absolute(); INC();								break;	// INC (absolute)
	case 0xef:	absolute(); ISB();								break;	// *ISB (absolute)

	case 0xf0:	immediate(); BEQ();								break;	// BEQ (relative)
	case 0xf1:	indirectIndexedY(); SBC();						break;	// SBC (indirect indexed Y)
	case 0xf2:	swallowRead(); JAM();							break;	// *JAM
	case 0xf3:	indirectIndexedYAddress(); fixupRead(); ISB();	break;	// *ISB (indirect indexed Y)
	case 0xf4:	zeroPageX(); NOP();								break;	// *NOP (zero page, X)
	case 0xf5:	zeroPageX(); SBC();								break;	// SBC (zero page, X)
	case 0xf6:	zeroPageX(); INC();								break;	// INC (zero page, X)
	case 0xf7:	zeroPageX(); ISB();								break;	// *ISB (zero page, X)
	case 0xf8:	swallowRead(); SED();							break;	// SED (implied)
	case 0xf9:	absoluteY(); SBC();								break;	// SBC (absolute, Y)
	case 0xfa:	swallowRead(); NOP();							break;	// *NOP (implied)
	case 0xfb:	absoluteYAddress(); fixupRead(); ISB();			break;	// *ISB (absolute, Y)
	case 0xfc:	absoluteXAddress(); maybeFixupRead(); NOP();	break;	// *NOP (absolute, X)
	case 0xfd:	absoluteX(); SBC();								break;	// SBC (absolute, X)
	case 0xfe:	absoluteXAddress(); fixupRead(); INC();			break;	// INC (absolute, X)
	case 0xff:	absoluteXAddress(); fixupRead(); ISB();			break;	// *ISB (absolute, X)
	}
}

void EightBit::MOS6502::push(uint8_t value) noexcept {
	lowerStack();
	base::memoryWrite(value);
}

void EightBit::MOS6502::pop() noexcept {
	raiseStack();
	memoryRead();
}

void EightBit::MOS6502::dummyPush() noexcept {
	lowerStack();
	tick();	// In place of the memory write
}

#pragma region Instruction implementations

#pragma region Miscellaneous

void EightBit::MOS6502::JMP() noexcept {
	jump(BUS().ADDRESS());
}

#pragma endregion

#pragma region Load and store

void EightBit::MOS6502::STA() noexcept {
	base::memoryWrite(A());
}

void EightBit::MOS6502::STX() noexcept {
	base::memoryWrite(X());
}

void EightBit::MOS6502::STY() noexcept {
	base::memoryWrite(Y());
}

#pragma endregion

#pragma region Branching

void EightBit::MOS6502::fixupBranch(int8_t relative) noexcept {
	noteFixedAddress(PC().joined + relative);
	maybeFixup();
}

void EightBit::MOS6502::branch(bool condition) noexcept {
	if (condition) {
		int8_t relative = BUS().DATA();
		swallowRead();
		fixupBranch(relative);
		JMP();
	}
}

void EightBit::MOS6502::branchNot(int condition) noexcept {
	branch(condition == 0);
}

void EightBit::MOS6502::branch(int condition) noexcept {
	branch(condition != 0);
}

void EightBit::MOS6502::BCS() noexcept {
	branch(carry());
}

void EightBit::MOS6502::BCC() noexcept {
	branchNot(carry());
}

void EightBit::MOS6502::BVC() noexcept {
	branchNot(overflow());
}

void EightBit::MOS6502::BMI() noexcept {
	branch(negative());
}

void EightBit::MOS6502::BPL() noexcept {
	branchNot(negative());
}

void EightBit::MOS6502::BVS() noexcept {
	branch(overflow());
}

void EightBit::MOS6502::BEQ() noexcept {
	branch(zero());
}
void EightBit::MOS6502::BNE() noexcept {
	branchNot(zero());
}

#pragma endregion

#pragma region Increment/decrement

void EightBit::MOS6502::DEC() {
	modifyWrite(DEC(BUS().DATA()));
}

void EightBit::MOS6502::INC() {
	modifyWrite(INC(BUS().DATA()));
}

#pragma endregion

#pragma region Stack operations

void EightBit::MOS6502::JSR() {
	fetchByte();
	auto low = BUS().DATA();
	swallowPop();
	pushShort(PC());
	fetchByte();
	auto high = BUS().DATA();
	PC() = { low, high };
}

void EightBit::MOS6502::PHA() {
	push(A());
}

void EightBit::MOS6502::PLA() {
	swallowPop();
	pop();
	LDA();
}

void EightBit::MOS6502::PHP() {
	push(setBit(P(), BF));
}

void EightBit::MOS6502::PLP() {
	swallowPop();
	pop();
	P() = clearBit(setBit(BUS().DATA(), RF), BF);
}

void EightBit::MOS6502::RTI() {
	PLP();
	base::ret();
}

void EightBit::MOS6502::RTS() {
	swallowPop();
	base::ret();
	swallowFetch();
}

#pragma endregion

#pragma region Shift/rotate operations

#pragma region Shift

void EightBit::MOS6502::ASL() {
	modifyWrite(ASL(BUS().DATA()));
}

void EightBit::MOS6502::LSR() {
	modifyWrite(LSR(BUS().DATA()));
}

#pragma endregion

#pragma region Rotate

void EightBit::MOS6502::ROL() {
	modifyWrite(ROL(BUS().DATA()));
}

void EightBit::MOS6502::ROR() {
	modifyWrite(ROR(BUS().DATA()));
}

#pragma endregion

#pragma endregion

#pragma region Undocumented instructions

#pragma region Undocumented instructions with BCD effects

void EightBit::MOS6502::ARR() {
	denary() != 0 ? decimalARR() : binaryARR();
}

uint8_t EightBit::MOS6502::coreARR() {
	A() &= BUS().DATA();
	const auto unshiftedA = A();
	RORA();
	setFlag(VF, overflowTest((A() ^ A() << 1)));
	return unshiftedA;
}

void EightBit::MOS6502::decimalARR() {
	// With thanks to https://github.com/TomHarte/CLK
	// What a very strange instruction ARR is...
	const auto unshiftedA = coreARR();
	if (lowerNibble(unshiftedA) + (unshiftedA & 0x1) > 5)
		A() = lowerNibble((A() + 6)) | higherNibble(A());
	setFlag(CF, higherNibble(unshiftedA) + (unshiftedA & 0x10) > 0x50);
	if (carry() != 0)
		A() += 0x60;
}

void EightBit::MOS6502::binaryARR() {
	coreARR();
	setFlag(CF, overflowTest(A()));
}

#pragma endregion

#pragma region Undocumented instructions with fixup effects

void EightBit::MOS6502::storeFixupEffect(uint8_t data) {
	const uint8_t mask = unfixedPage() + 1;   // base_hi + 1, always
	const uint8_t updated = data & mask;
	if (fixed())
		BUS().ADDRESS().high = updated;
	base::memoryWrite(updated);
}

void EightBit::MOS6502::SHA() {
	storeFixupEffect(A() & X());
}

void EightBit::MOS6502::SYA() {
	storeFixupEffect(Y());
}

void EightBit::MOS6502::SXA() {
	storeFixupEffect(X());
}

#pragma endregion

void EightBit::MOS6502::SAX() {
	base::memoryWrite(A() & X());
}

void EightBit::MOS6502::LAX() {
	LDA();
	LDX();
}

void EightBit::MOS6502::ANC() {
	AND();
	setFlag(CF, negativeTest(A()));
}

void EightBit::MOS6502::AXS() {
	X() = through(binarySUB(A() & X()));
	resetFlag(CF, intermediate().high);
}

void EightBit::MOS6502::JAM() {
	base::memoryRead(0xff, 0xff);
	BUS().ADDRESS().low = 0xfe;
	memoryRead();
	memoryRead();
	BUS().ADDRESS().low = 0xff;
	memoryRead();
	memoryRead();
	memoryRead();
	memoryRead();
	memoryRead();
	memoryRead();
}

void EightBit::MOS6502::TAS() {
	S() = A() & X();
	SHA();
}

void EightBit::MOS6502::LAS() {
	A() = X() = S() = through(BUS().DATA() & S());
}

void EightBit::MOS6502::ANE() {
	A() = through((A() | 0xee) & X() & BUS().DATA());
}

void EightBit::MOS6502::ATX() {
	A() = X() = through((A() | 0xee) & BUS().DATA());
}

void EightBit::MOS6502::ASR() {
	AND();
	LSRA();
}

void EightBit::MOS6502::ISB() {
	INC();
	SBC();
}

void EightBit::MOS6502::RLA() {
	ROL();
	AND();
}

void EightBit::MOS6502::RRA() {
	ROR();
	ADC();
}

void EightBit::MOS6502::SLO() {
	ASL();
	ORA();
}

void EightBit::MOS6502::SRE() {
	LSR();
	EOR();
}

void EightBit::MOS6502::DCP() {
	DEC();
	CMP();
}

#pragma endregion

#pragma endregion
