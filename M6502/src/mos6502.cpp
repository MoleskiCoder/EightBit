#include "stdafx.h"
#include "mos6502.h"

EightBit::MOS6502::MOS6502(Bus& bus)
: Processor(bus) {

	m_operations = {
		/* 0 */
			/* 0 */ { 7, [this]()	{ BRK();								} },
			/* 1 */ { 6, [this]()	{ ORA(AM_IndexedIndirectX());			} },
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 8, [this]()	{ SLO(AM_IndexedIndirectX());			} },	// *SLO
			/* 4 */ { 3, [this]()	{ AM_ZeroPage();						} },	// *DOP
			/* 5 */ { 3, [this]()	{ ORA(AM_ZeroPage());					} },
			/* 6 */ { 5, [this]()	{ setByte(ASL(AM_ZeroPage()));			} },
			/* 7 */ { 5, [this]()	{ SLO(AM_ZeroPage());					} },	// *SLO
			/* 8 */ { 3, [this]()	{ PHP();								} },
			/* 9 */ { 2, [this]()	{ ORA(AM_Immediate());					} },
			/* a */ { 2, [this]()	{ A() = ASL(A());						} },
			/* b */ { 0, []()		{										} },
			/* c */ { 4, [this]()	{ AM_Absolute();						} },	// *TOP
			/* d */ { 4, [this]()	{ ORA(AM_Absolute());					} },
			/* e */ { 6, [this]()	{ setByte(ASL(AM_Absolute()));			} },
			/* f */ { 6, [this]()	{ SLO(AM_Absolute());					} },	// *SLO
		/* 1 */
			/* 0 */ { 2, [this]()	{ Branch(!(P() & NF));					} },
			/* 1 */ { 5, [this]()	{ ORA(AM_IndirectIndexedY());			} },
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 7, [this]()	{ SLO(AM_IndirectIndexedY());			} },	// *SLO
			/* 4 */ { 4, [this]()	{ AM_ZeroPageX();						} },	// *DOP
			/* 5 */ { 4, [this]()	{ ORA(AM_ZeroPageX());					} },
			/* 6 */ { 6, [this]()	{ setByte(ASL(AM_ZeroPageX()));			} },
			/* 7 */ { 6, [this]()	{ SLO(AM_ZeroPageX());					} },	// *SLO
			/* 8 */ { 2, [this]()	{ clearFlag(P(), CF);					} },
			/* 9 */ { 4, [this]()	{ ORA(AM_AbsoluteY());					} },
			/* a */ { 2, []()		{										} },	// *NOP
			/* b */ { 6, [this]()	{ SLO(AM_AbsoluteY());					} },	// *SLO
			/* c */ { 4, [this]()	{ AM_AbsoluteX();						} },	// *TOP
			/* d */ { 4, [this]()	{ ORA(AM_AbsoluteX());					} },
			/* e */ { 7, [this]()	{ setByte(ASL(AM_AbsoluteX()));			} },
			/* f */ { 6, [this]()	{ SLO(AM_AbsoluteX());					} },	// *SLO
		/* 2 */
			/* 0 */ { 6, [this]()	{ JSR_abs();							} },
			/* 1 */ { 6, [this]()	{ ANDA(AM_IndexedIndirectX());			} },
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 8, [this]()	{ RLA(AM_IndexedIndirectX());			} },	// *RLA
			/* 4 */ { 3, [this]()	{ BIT(AM_ZeroPage());					} },
			/* 5 */ { 3, [this]()	{ ANDA(AM_ZeroPage());					} },
			/* 6 */ { 5, [this]()	{ setByte(ROL(AM_ZeroPage()));			} },
			/* 7 */ { 5, [this]()	{ RLA(AM_ZeroPage());					} },	// *RLA
			/* 8 */ { 4, [this]()	{ PLP();								} },
			/* 9 */ { 2, [this]()	{ ANDA(AM_Immediate());					} },
			/* a */ { 2, [this]()	{ A() = ROL(A());						} },
			/* b */ { 0, []()		{										} },
			/* c */ { 4, [this]()	{ BIT(AM_Absolute());					} },
			/* d */ { 4, [this]()	{ ANDA(AM_Absolute());					} },
			/* e */ { 6, [this]()	{ setByte(ROL(AM_Absolute()));			} },
			/* f */ { 6, [this]()	{ RLA(AM_Absolute());					} },	// *RLA
		/* 3 */
			/* 0 */ { 2, [this]()	{ Branch(!!(P() & NF));					} },
			/* 1 */ { 5, [this]()	{ ANDA(AM_IndirectIndexedY());			} },
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 7, [this]()	{ RLA(AM_IndirectIndexedY());			} },	// *RLA
			/* 4 */ { 4, [this]()	{ AM_ZeroPageX();						} },	// *DOP
			/* 5 */ { 4, [this]()	{ ANDA(AM_ZeroPageX());					} },
			/* 6 */ { 6, [this]()	{ setByte(ROL(AM_ZeroPageX()));			} },
			/* 7 */ { 6, [this]()	{ RLA(AM_ZeroPageX());					} },	// *RLA
			/* 8 */ { 2, [this]()	{ setFlag(P(), CF);						} },
			/* 9 */ { 4, [this]()	{ ANDA(AM_AbsoluteY());					} },
			/* a */ { 2, []()		{										} },	// *NOP
			/* b */ { 6, [this]()	{ RLA(AM_AbsoluteY());					} },	// *RLA
			/* c */ { 4, [this]()	{ AM_AbsoluteX();						} },	// *TOP
			/* d */ { 4, [this]()	{ ANDA(AM_AbsoluteX());					} },
			/* e */ { 7, [this]()	{ setByte(ROL(AM_AbsoluteX()));			} },
			/* f */ { 6, [this]()	{ RLA(AM_AbsoluteX());					} },	// *RLA
		/* 4 */
			/* 0 */ { 6, [this]()	{ RTI();								} },
			/* 1 */ { 6, [this]()	{ EORA(AM_IndexedIndirectX());			} },
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 8, [this]()	{ SRE(AM_IndexedIndirectX());			} },	// *SRE
			/* 4 */ { 3, [this]()	{ AM_ZeroPage();						} },	// *DOP
			/* 5 */ { 3, [this]()	{ EORA(AM_ZeroPage());					} },
			/* 6 */ { 5, [this]()	{ setByte(LSR(AM_ZeroPage()));			} },
			/* 7 */ { 5, [this]()	{ SRE(AM_ZeroPage());					} },	// *SRE
			/* 8 */ { 3, [this]()	{ push(A());							} },
			/* 9 */ { 2, [this]()	{ EORA(AM_Immediate());					} },
			/* a */ { 2, [this]()	{ A() = LSR(A());						} },
			/* b */ { 0, []()		{										} },
			/* c */ { 3, [this]()	{ JMP_abs();							} },
			/* d */ { 4, [this]()	{ EORA(AM_Absolute());					} },
			/* e */ { 6, [this]()	{ setByte(LSR(AM_Absolute()));			} },
			/* f */ { 6, [this]()	{ SRE(AM_Absolute());					} },	// *SRE
		/* 5 */
			/* 0 */ { 2, [this]()	{ Branch(!(P() & VF));					} },	// *SRE
			/* 1 */ { 5, [this]()	{ EORA(AM_IndirectIndexedY());			} },
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 7, [this]()	{ SRE(AM_IndirectIndexedY());			} },	// *SRE
			/* 4 */ { 4, [this]()	{ AM_ZeroPage();						} },	// *DOP
			/* 5 */ { 4, [this]()	{ EORA(AM_ZeroPageX());					} },
			/* 6 */ { 6, [this]()	{ setByte(LSR(AM_ZeroPageX()));			} },
			/* 7 */ { 6, [this]()	{ SRE(AM_ZeroPageX());					} },	// *SRE
			/* 8 */ { 2, [this]()	{ clearFlag(P(), IF);					} },
			/* 9 */ { 4, [this]()	{ EORA(AM_AbsoluteY());					} },
			/* a */ { 2, []()		{										} },	// *NOP
			/* b */ { 6, [this]()	{ SRE(AM_AbsoluteY());					} },	// *SRE
			/* c */ { 4, [this]()	{ AM_AbsoluteX();						} },	// *TOP
			/* d */ { 4, [this]()	{ EORA(AM_AbsoluteX());					} },
			/* e */ { 7, [this]()	{ setByte(LSR(AM_AbsoluteX()));			} },
			/* f */ { 6, [this]()	{ SRE(AM_AbsoluteX());					} },	// *SRE
		/* 6 */
			/* 0 */ { 6, [this]()	{ RTS();								} },
			/* 1 */ { 6, [this]()	{ A() = ADC(A(), AM_IndexedIndirectX());} },
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 8, [this]()	{ RRA(AM_IndexedIndirectX());			} },	// *RRA
			/* 4 */ { 3, [this]()	{ AM_ZeroPage();						} },	// *DOP
			/* 5 */ { 3, [this]()	{ A() = ADC(A(), AM_ZeroPage());		} },
			/* 6 */ { 5, [this]()	{ setByte(ROR(AM_ZeroPage()));			} },
			/* 7 */ { 5, [this]()	{ RRA(AM_ZeroPage());					} },	// *RRA
			/* 8 */ { 4, [this]()	{ adjustNZ(A() = pop());				} },
			/* 9 */ { 2, [this]()	{ A() = ADC(A(), AM_Immediate());		} },	// ADC #
			/* a */ { 2, [this]()	{ A() = ROR(A());						} },	// ROR A
			/* b */ { 0, []()		{										} },
			/* c */ { 5, [this]()	{ JMP_ind();							} },	// JMP ind
			/* d */ { 4, [this]()	{ A() = ADC(A(), AM_Absolute());		} },	// ADC abs
			/* e */ { 6, [this]()	{ setByte(ROR(AM_Absolute()));			} },	// ROR abs
			/* f */ { 6, [this]()	{ RRA(AM_Absolute());					} },	// *RRA
		/* 7 */
			/* 0 */ { 2, [this]()	{ Branch(!!(P() & VF));					} },
			/* 1 */ { 5, [this]()	{ A() = ADC(A(), AM_IndirectIndexedY());} },	// ADC ind,Y
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 7, [this]()	{ RRA(AM_IndirectIndexedY());			} },	// *RRA
			/* 4 */ { 4, [this]()	{ AM_ZeroPageX();						} },	// *DOP
			/* 5 */ { 4, [this]()	{ A() = ADC(A(), AM_ZeroPageX());		} },
			/* 6 */ { 6, [this]()	{ setByte(ROR(AM_ZeroPageX()));			} },
			/* 7 */ { 6, [this]()	{ RRA(AM_ZeroPageX());					} },	// *RRA
			/* 8 */ { 2, [this]()	{ setFlag(P(), IF);						} },
			/* 9 */ { 4, [this]()	{ A() = ADC(A(), AM_AbsoluteY());		} },	// ADC abs,Y
			/* a */ { 2, [this]()	{										} },	// *NOP
			/* b */ { 6, [this]()	{ RRA(AM_AbsoluteY());					} },	// *RRA
			/* c */ { 4, [this]()	{ AM_AbsoluteX();						} },	// *TOP
			/* d */ { 4, [this]()	{ A() = ADC(A(), AM_AbsoluteX());		} },	// ADC abs,X
			/* e */ { 7, [this]()	{ setByte(ROR(AM_AbsoluteX()));			} },	// ROR abs,X
			/* f */ { 6, [this]()	{ RRA(AM_AbsoluteX());					} },	// *RRA
		/* 8 */
			/* 0 */ { 2, [this]()	{ AM_Immediate();						} },	// *DOP
			/* 1 */ { 6, [this]()	{ AM_IndexedIndirectX(A());				} },
			/* 2 */ { 2, [this]()	{ AM_Immediate();						} },	// *DOP
			/* 3 */ { 6, [this]()	{ AM_IndexedIndirectX(A() & X());		} },	// *SAX 
			/* 4 */ { 3, [this]()	{ AM_ZeroPage(Y());						} },	// STY zpg
			/* 5 */ { 3, [this]()	{ AM_ZeroPage(A());						} },
			/* 6 */ { 3, [this]()	{ AM_ZeroPage(X());						} },
			/* 7 */ { 3, [this]()	{ AM_ZeroPage(A() & X());				} },	// *SAX
			/* 8 */ { 2, [this]()	{ adjustNZ(--Y());						} },
			/* 9 */ { 2, [this]()	{ AM_Immediate();						} },	// *DOP
			/* a */ { 2, [this]()	{ adjustNZ(A() = X());					} },	// TXA impl
			/* b */ { 0, []()		{										} },
			/* c */ { 4, [this]()	{ AM_Absolute(Y());						} },	// STY abs
			/* d */ { 4, [this]()	{ AM_Absolute(A());						} },
			/* e */ { 4, [this]()	{ AM_Absolute(X());						} },	// STX abs
			/* f */ { 4, [this]()	{ AM_Absolute(A() & X());				} },	// *SAX
		/* 9 */
			/* 0 */ { 2, [this]()	{ Branch(!(P() & CF));					} },
			/* 1 */ { 6, [this]()	{ AM_IndirectIndexedY(A());				} },	// STA ind,Y
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 0, []()		{										} },
			/* 4 */ { 4, [this]()	{ AM_ZeroPageX(Y());					} },	// STY zpg,X
			/* 5 */ { 4, [this]()	{ AM_ZeroPageX(A());					} },
			/* 6 */ { 4, [this]()	{ AM_ZeroPageY(X());					} },
			/* 7 */ { 4, [this]()	{ AM_ZeroPageY(A() & X());				} },	// *SAX
			/* 8 */ { 2, [this]()	{ adjustNZ(A() = Y());					} },
			/* 9 */ { 5, [this]()	{ AM_AbsoluteY(A());					} },	// STA abs,Y
			/* a */ { 2, [this]()	{ S() = X();							} },
			/* b */ { 0, []()		{										} },
			/* c */ { 0, []()		{										} },
			/* d */ { 5, [this]()	{ AM_AbsoluteX(A());					} },	// STA abs,X
			/* e */ { 0, []()		{										} },
			/* f */ { 0, []()		{										} },
		/* A */
			/* 0 */ { 2, [this]()	{ adjustNZ(Y() = AM_Immediate());		} },
			/* 1 */ { 6, [this]()	{ adjustNZ(A() = AM_IndexedIndirectX());} },
			/* 2 */ { 2, [this]()	{ adjustNZ(X() = AM_Immediate());		} },
			/* 3 */ { 6, [this]()	{ LAX(AM_IndexedIndirectX());			} },	// *LAX
			/* 4 */ { 3, [this]()	{ adjustNZ(Y() = AM_ZeroPage());		} },	// LDY zpg
			/* 5 */ { 3, [this]()	{ adjustNZ(A() = AM_ZeroPage());		} },
			/* 6 */ { 3, [this]()	{ adjustNZ(X() = AM_ZeroPage());		} },
			/* 7 */ { 3, [this]()	{ LAX(AM_ZeroPage());					} },	// *LAX
			/* 8 */ { 2, [this]()	{ adjustNZ(Y() = A());					} },
			/* 9 */ { 2, [this]()	{ adjustNZ(A() = AM_Immediate());		} },
			/* a */ { 2, [this]()	{ adjustNZ(X() = A());					} },	// TAX imp
			/* b */ { 0, []()		{										} },
			/* c */ { 4, [this]()	{ adjustNZ(Y() = AM_Absolute());		} },	// LDY abs
			/* d */ { 4, [this]()	{ adjustNZ(A() = AM_Absolute());		} },	// LDA abs
			/* e */ { 4, [this]()	{ adjustNZ(X() = AM_Absolute());		} },	// LDX abs
			/* f */ { 4, [this]()	{ LAX(AM_Absolute());					} },	// *LAX
		/* B */
			/* 0 */ { 2, [this]()	{ Branch(!!(P() & CF));					} },
			/* 1 */ { 5, [this]()	{ adjustNZ(A() = AM_IndirectIndexedY());} },	// LDA ind,Y
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 5, [this]()	{ LAX(AM_IndirectIndexedY());			} },	// *LAX
			/* 4 */ { 4, [this]()	{ adjustNZ(Y() = AM_ZeroPageX());		} },	// LDY zpg,X
			/* 5 */ { 4, [this]()	{ adjustNZ(A() = AM_ZeroPageX());		} },
			/* 6 */ { 4, [this]()	{ adjustNZ(X() = AM_ZeroPageY());		} },
			/* 7 */ { 4, [this]()	{ LAX(AM_ZeroPageY());					} },	// *LAX
			/* 8 */ { 2, [this]()	{ clearFlag(P(), VF);					} },
			/* 9 */ { 4, [this]()	{ adjustNZ(A() = AM_AbsoluteY());		} },	// LDA abs,Y
			/* a */ { 2, [this]()	{ adjustNZ(X() = S());					} },	// TSX impl
			/* b */ { 0, []()		{										} },
			/* c */ { 4, [this]()	{ adjustNZ(Y() = AM_AbsoluteX());		} },	// LDY abs,X
			/* d */ { 4, [this]()	{ adjustNZ(A() = AM_AbsoluteX());		} },	// LDA abs,X
			/* e */ { 4, [this]()	{ adjustNZ(X() = AM_AbsoluteY());		} },	// LDX abs,Y
			/* f */ { 4, [this]()	{ LAX(AM_AbsoluteY());					} },	// *LAX
		/* C */
			/* 0 */ { 2, [this]()	{ CMP(Y(), AM_Immediate());				} },
			/* 1 */ { 6, [this]()	{ CMP(A(), AM_IndexedIndirectX());		} },
			/* 2 */ { 2, [this]()	{ AM_Immediate();						} },	// *DOP
			/* 3 */ { 8, [this]()	{ DCP(AM_IndexedIndirectX());			} },	// *DCP
			/* 4 */ { 3, [this]()	{ CMP(Y(), AM_ZeroPage());				} },	// CPY zpg
			/* 5 */ { 3, [this]()	{ CMP(A(), AM_ZeroPage());				} },
			/* 6 */ { 5, [this]()	{ setByte(DEC(AM_ZeroPage()));			} },
			/* 7 */ { 5, [this]()	{ DCP(AM_ZeroPage());					} },	// *DCP
			/* 8 */ { 2, [this]()	{ adjustNZ(++Y());						} },
			/* 9 */ { 2, [this]()	{ CMP(A(), AM_Immediate());				} },	// CMP #
			/* a */ { 2, [this]()	{ adjustNZ(--X());						} },
			/* b */ { 0, []()		{										} },
			/* c */ { 4, [this]()	{ CMP(Y(), AM_Absolute());				} },	// CPY abs
			/* d */ { 4, [this]()	{ CMP(A(), AM_Absolute());				} },	// CMP abs
			/* e */ { 6, [this]()	{ setByte(DEC(AM_Absolute()));			} },	// DEC abs
			/* f */ { 6, [this]()	{ DCP(AM_Absolute());					} },	// *DCP
		/* D */
			/* 0 */ { 2, [this]()	{ Branch(!(P() & ZF));					} },
			/* 1 */ { 5, [this]()	{ CMP(A(), AM_IndirectIndexedY());		} },	// CMP ind,Y
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 7, [this]()	{ DCP(AM_IndirectIndexedY());			} },	// *DCP
			/* 4 */ { 4, [this]()	{ AM_ZeroPageX();						} },	// *DOP
			/* 5 */ { 4, [this]()	{ CMP(A(), AM_ZeroPageX());				} },
			/* 6 */ { 6, [this]()	{ setByte(DEC(AM_ZeroPageX()));			} },
			/* 7 */ { 6, [this]()	{ DCP(AM_ZeroPageX());					} },	// *DCP
			/* 8 */ { 2, [this]()	{ clearFlag(P(), DF);					} },
			/* 9 */ { 4, [this]()	{ CMP(A(), AM_AbsoluteY());				} },	// CMP abs,Y
			/* a */ { 2, [this]()	{										} },	// *NOP
			/* b */ { 6, [this]()	{ DCP(AM_AbsoluteY());					} },	// *DCP
			/* c */ { 4, [this]()	{ AM_AbsoluteX();						} },	// *TOP
			/* d */ { 4, [this]()	{ CMP(A(), AM_AbsoluteX());				} },	// CMP abs,X
			/* e */ { 7, [this]()	{ setByte(DEC(AM_AbsoluteX()));			} },
			/* f */ { 6, [this]()	{ DCP(AM_AbsoluteX());					} },	// *DCP
		/* E */
			/* 0 */ { 2, [this]()	{ CMP(X(), AM_Immediate());				} },
			/* 1 */ { 6, [this]()	{ A() = SBC(A(), AM_IndexedIndirectX());} },
			/* 2 */ { 2, [this]()	{ AM_Immediate();						} },	// *DOP
			/* 3 */ { 8, [this]()	{ ISB(AM_IndexedIndirectX());			} },	// *ISB
			/* 4 */ { 3, [this]()	{ CMP(X(), AM_ZeroPage());				} },	// CPX zpg
			/* 5 */ { 3, [this]()	{ A() = SBC(A(), AM_ZeroPage());		} },
			/* 6 */ { 5, [this]()	{ setByte(INC(AM_ZeroPage()));			} },
			/* 7 */ { 5, [this]()	{ ISB(AM_ZeroPage());					} },	// *ISB
			/* 8 */ { 2, [this]()	{ adjustNZ(++X());						} },
			/* 9 */ { 2, [this]()	{ A() = SBC(A(), AM_Immediate());		} },	// SBC #
			/* a */ { 2, [this]()	{										} },	// NOP
			/* b */ { 2, [this]()	{ A() = SBC(A(), AM_Immediate());		} },	// *SBC
			/* c */ { 4, [this]()	{ CMP(X(), AM_Absolute());				} },	// CPX abs
			/* d */ { 4, [this]()	{ A() = SBC(A(), AM_Absolute());		} },	// SBC abs
			/* e */ { 6, [this]()	{ setByte(INC(AM_Absolute()));			} },	// INC abs
			/* f */ { 6, [this]()	{ ISB(AM_Absolute());					} },	// *ISB
		/* F */
			/* 0 */ { 2, [this]()	{ Branch(!!(P() & ZF));					} },
			/* 1 */ { 5, [this]()	{ A() = SBC(A(), AM_IndirectIndexedY());} },	// SBC ind,Y
			/* 2 */ { 0, []()		{										} },
			/* 3 */ { 7, [this]()	{ ISB(AM_IndirectIndexedY());			} },	// *ISB
			/* 4 */ { 4, [this]()	{ AM_ZeroPageX();						} },	// *DOP
			/* 5 */ { 4, [this]()	{ A() = SBC(A(), AM_ZeroPageX());		} },
			/* 6 */ { 6, [this]()	{ setByte(INC(AM_ZeroPageX()));			} },
			/* 7 */ { 6, [this]()	{ ISB(AM_ZeroPageX());					} },	// *ISB
			/* 8 */ { 2, [this]()	{ setFlag(P(), DF);						} },
			/* 9 */ { 4, [this]()	{ A() = SBC(A(), AM_AbsoluteY());		} },	// SBC abs,Y
			/* a */ { 2, [this]()	{										} },	// *NOP
			/* b */ { 6, [this]()	{ ISB(AM_AbsoluteY());					} },	// *ISB
			/* c */ { 4, [this]()	{ AM_AbsoluteX();						} },	// *TOP
			/* d */ { 4, [this]()	{ A() = SBC(A(), AM_AbsoluteX());		} },	// SBC abs,X
			/* e */ { 7, [this]()	{ setByte(INC(AM_AbsoluteX()));			} },	// INC abs,X
			/* f */ { 6, [this]()	{ ISB(AM_AbsoluteX());					} }		// *ISB
	} ;

	X() = Bit7;
	Y() = 0;
	A() = 0;
	P() = RF;
	S() = Mask8;

	raise(SO());
}

int EightBit::MOS6502::step() {
	resetCycles();
	auto returned = 0;
	if (LIKELY(powered())) {
		ExecutingInstruction.fire(*this);
		if (UNLIKELY(lowered(SO()))) {
			P() |= VF;
			raise(SO());
		}
		if (UNLIKELY(lowered(NMI()))) {
			raise(NMI());
			interrupt(NMIvector);
			returned = 4;	// ?? TBC
		} else if (UNLIKELY(lowered(INT()))) {
			raise(INT());
			interrupt(IRQvector);
			returned = 4;	// ?? TBC
		} else if (UNLIKELY(lowered(HALT()))) {
			execute(0xea);	// NOP
			returned = 2;	//
		} else {
			returned = execute(fetchByte());
		}
		ExecutedInstruction.fire(*this);
	}
	return returned;
}

void EightBit::MOS6502::reset() {
	Processor::reset();
	getWord(0xff, RSTvector, PC());
}

void EightBit::MOS6502::getWord(uint8_t page, uint8_t offset, register16_t& output) {
	BUS().ADDRESS().low = offset;
	BUS().ADDRESS().high = page;
	output.low = getByte();
	BUS().ADDRESS().low++;
	output.high = getByte();
}

void EightBit::MOS6502::interrupt(uint8_t vector) {
	raise(HALT());
	pushWord(PC());
	push(P());
	setFlag(P(), IF);
	getWord(0xff, vector, PC());
}

int EightBit::MOS6502::execute(uint8_t cell) {

	const operation_t& operation = m_operations[cell];

	addCycles(operation.timing);
	(operation.method)();

	if (UNLIKELY(cycles() == 0))
		throw std::logic_error("Unhandled opcode");

	return cycles();
}

////

void EightBit::MOS6502::push(uint8_t value) {
	setByte(PageOne + S()--, value);
}

uint8_t EightBit::MOS6502::pop() {
	return getByte(PageOne + ++S());
}

////

uint8_t EightBit::MOS6502::ROR(uint8_t value) {
	const auto carry = P() & CF;
	setFlag(P(), CF, value & CF);
	value = (value >> 1) | (carry << 7);
	adjustNZ(value);
	return value;
}

uint8_t EightBit::MOS6502::LSR(uint8_t value) {
	setFlag(P(), CF, value & CF);
	adjustNZ(value >>= 1);
	return value;
}

void EightBit::MOS6502::BIT(uint8_t data) {
	adjustZero(A() & data);
	adjustNegative(data);
	setFlag(P(), VF, data & VF);
}

uint8_t EightBit::MOS6502::ROL(uint8_t value) {
	const uint8_t result = (value << 1) | (P() & CF);
	setFlag(P(), CF, value & Bit7);
	adjustNZ(result);
	return result;
}

uint8_t EightBit::MOS6502::ASL(uint8_t value) {
	setFlag(P(), CF, (value & Bit7) >> 7);
	adjustNZ(value <<= 1);
	return value;
}

uint8_t EightBit::MOS6502::SBC(const uint8_t operand, const uint8_t data) {

	const auto returned = SUB(operand, data, ~P() & CF);

	const register16_t& difference = MEMPTR();
	adjustNZ(difference.low);
	setFlag(P(), VF, (operand ^ data) & (operand ^ difference.low) & NF);
	clearFlag(P(), CF, difference.high);

	return returned;
}

uint8_t EightBit::MOS6502::SUB(const uint8_t operand, const uint8_t data, const int borrow) {
	return P() & DF ? SUB_d(operand, data, borrow) : SUB_b(operand, data, borrow);
}

uint8_t EightBit::MOS6502::SUB_b(const uint8_t operand, const uint8_t data, const int borrow) {
	MEMPTR().word = operand - data - borrow;
	return MEMPTR().low;
}

uint8_t EightBit::MOS6502::SUB_d(const uint8_t operand, const uint8_t data, const int borrow) {
	MEMPTR().word = operand - data - borrow;

	uint8_t low = lowNibble(operand) - lowNibble(data) - borrow;
	const auto lowNegative = low & NF;
	if (lowNegative)
		low -= 6;

	uint8_t high = highNibble(operand) - highNibble(data) - (lowNegative >> 7);
	const auto highNegative = high & NF;
	if (highNegative)
		high -= 6;

	return promoteNibble(high) | lowNibble(low);
}

void EightBit::MOS6502::CMP(uint8_t first, uint8_t second) {
	register16_t result;
	result.word = first - second;
	adjustNZ(result.low);
	clearFlag(P(), CF, result.high);
}

uint8_t EightBit::MOS6502::ADC(const uint8_t operand, const uint8_t data) {
	const auto returned = ADD(operand, data, P() & CF);
	adjustNZ(MEMPTR().low);
	return returned;
}

uint8_t EightBit::MOS6502::ADD(uint8_t operand, uint8_t data, int carry) {
	return P() & DF ? ADD_d(operand, data, carry) : ADD_b(operand, data, carry);
}

uint8_t EightBit::MOS6502::ADD_b(uint8_t operand, uint8_t data, int carry) {
	MEMPTR().word = operand + data + carry;

	setFlag(P(), VF, ~(operand ^ data) & (operand ^ MEMPTR().low) & NF);
	setFlag(P(), CF, MEMPTR().high & CF);

	return MEMPTR().low;
}

uint8_t EightBit::MOS6502::ADD_d(uint8_t operand, uint8_t data, int carry) {

	MEMPTR().word = operand + data + carry;

	uint8_t low = lowNibble(operand) + lowNibble(data) + carry;
	if (low > 9)
		low += 6;

	uint8_t high = highNibble(operand) + highNibble(data) + (low > 0xf ? 1 : 0);
	setFlag(P(), VF, ~(operand ^ data) & (operand ^ promoteNibble(high)) & NF);

	if (high > 9)
		high += 6;

	setFlag(P(), CF, high > 0xf);

	return promoteNibble(high) | lowNibble(low);
}

////

void EightBit::MOS6502::Branch(int8_t displacement) {
	const auto page = PC().high;
	PC().word += displacement;
	if (UNLIKELY(PC().high != page))
		addCycle();
	addCycle();
}

void EightBit::MOS6502::Branch(bool flag) {
	const int8_t displacement = AM_Immediate();
	if (flag)
		Branch(displacement);
}

//

void EightBit::MOS6502::PHP() {
	push(P() | BF);
}

void EightBit::MOS6502::PLP() {
	P() = (pop() | RF) & ~BF;
}

//

void EightBit::MOS6502::JSR_abs() {
	Address_Absolute();
	PC().word--;
	call();
}

void EightBit::MOS6502::RTI() {
	PLP();
	ret();
}

void EightBit::MOS6502::RTS() {
	ret();
	PC().word++;
}

void EightBit::MOS6502::JMP_abs() {
	Address_Absolute();
	jump();
}

void EightBit::MOS6502::JMP_ind() {
	Address_Indirect();
	jump();
}

void EightBit::MOS6502::BRK() {
	PC().word++;
	pushWord(PC());
	PHP();
	setFlag(P(), IF);
	getWord(0xff, IRQvector, PC());
}
