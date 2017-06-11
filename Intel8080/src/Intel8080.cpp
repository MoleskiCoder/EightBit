#include "stdafx.h"
#include "Intel8080.h"

#include "Memory.h"
#include "Disassembler.h"

EightBit::Intel8080::Intel8080(Memory& memory, InputOutput& ports)
:	IntelProcessor(memory),
	a(0),
	f(0),
	m_interrupt(false),
	m_ports(ports) {
	bc.word = de.word = hl.word = 0;
	installInstructions();
}

EightBit::Intel8080::Instruction EightBit::Intel8080::INS(instruction_t method, AddressingMode mode, std::string disassembly, int cycles) {
	Intel8080::Instruction returnValue;
	returnValue.vector = method;
	returnValue.mode = mode;
	returnValue.disassembly = disassembly;
	returnValue.count = cycles;
	return returnValue;
}

EightBit::Intel8080::Instruction EightBit::Intel8080::UNKNOWN() {
	Intel8080::Instruction returnValue;
	returnValue.vector = std::bind(&Intel8080::___, this);
	returnValue.mode = Unknown;
	returnValue.disassembly = "";
	returnValue.count = 0;
	return returnValue;
}

#define BIND(method)	std::bind(&Intel8080:: method, this)

void EightBit::Intel8080::installInstructions() {
	instructions = {
		////	0											1											2											3											4											5												6											7											8											9											A											B											C											D											E											F
		/* 0 */	INS(BIND(nop), Implied, "NOP", 4),			INS(BIND(lxi_b), Absolute, "LXI B,", 10),	INS(BIND(stax_b), Implied, "STAX B", 7),	INS(BIND(inx_b), Implied, "INX B", 5),		INS(BIND(inr_b), Implied, "INR B", 5),		INS(BIND(dcr_b), Implied, "DCR B", 5),			INS(BIND(mvi_b), Immediate, "MVI B,", 7),	INS(BIND(rlc), Implied, "RLC", 4),			UNKNOWN(),									INS(BIND(dad_b), Implied, "DAD B", 10),		INS(BIND(ldax_b), Implied, "LDAX B", 7),	INS(BIND(dcx_b), Implied, "DCX B", 5),		INS(BIND(inr_c), Implied, "INR C", 5),		INS(BIND(dcr_c), Implied, "DCR C", 5),		INS(BIND(mvi_c), Immediate, "MVI C,", 7),	INS(BIND(rrc), Implied, "RRC", 4),			//	0
		/* 1 */	UNKNOWN(),									INS(BIND(lxi_d), Absolute, "LXI D,", 10),	INS(BIND(stax_d), Implied, "STAX D", 7),	INS(BIND(inx_d), Implied, "INX D", 5),		INS(BIND(inr_d), Implied, "INR D", 5),		INS(BIND(dcr_d), Implied, "DCR D", 5),			INS(BIND(mvi_d), Immediate, "MVI D,", 7),	INS(BIND(ral), Implied, "RAL", 4),			UNKNOWN(),									INS(BIND(dad_d), Implied, "DAD D", 10),		INS(BIND(ldax_d), Implied, "LDAX D", 7),	INS(BIND(dcx_d), Implied, "DCX D", 5),		INS(BIND(inr_e), Implied, "INR E", 5),		INS(BIND(dcr_e), Implied, "DCR E", 5),		INS(BIND(mvi_e), Immediate, "MVI E,", 7),	INS(BIND(rar), Implied, "RAR", 4),			//	1
		/* 2 */	UNKNOWN(),									INS(BIND(lxi_h), Absolute, "LXI H,", 10),	INS(BIND(shld), Absolute, "SHLD", 16),		INS(BIND(inx_h), Implied, "INX H", 5),		INS(BIND(inr_h), Implied, "INR H", 5),		INS(BIND(dcr_h), Implied, "DCR H", 5),			INS(BIND(mvi_h), Immediate, "MVI H,",7),	INS(BIND(daa), Implied, "DAA", 4),			UNKNOWN(),									INS(BIND(dad_h), Implied, "DAD H", 10),		INS(BIND(lhld), Absolute, "LHLD ", 16),		INS(BIND(dcx_h), Implied, "DCX H", 5),		INS(BIND(inr_l), Implied, "INR L", 5),		INS(BIND(dcr_l), Implied, "DCR L", 5),		INS(BIND(mvi_l), Immediate, "MVI L,", 7),	INS(BIND(cma), Implied, "CMA", 4),			//	2
		/* 3 */	UNKNOWN(),									INS(BIND(lxi_sp), Absolute, "LXI SP,", 10),	INS(BIND(sta), Absolute, "STA ", 13),		INS(BIND(inx_sp), Implied, "INX SP", 5),	INS(BIND(inr_m), Implied, "INR M", 10),		INS(BIND(dcr_m), Implied, "DCR M", 10),			INS(BIND(mvi_m), Immediate, "MVI M,", 10),	INS(BIND(stc), Implied, "STC", 4),			UNKNOWN(),									INS(BIND(dad_sp), Implied, "DAD SP", 10),	INS(BIND(lda), Absolute, "LDA ", 13),		INS(BIND(dcx_sp), Implied, "DCX SP", 5),	INS(BIND(inr_a), Implied, "INR A", 5),		INS(BIND(dcr_a), Implied, "DCR A", 5),		INS(BIND(mvi_a), Immediate, "MVI A,", 7),	INS(BIND(cmc), Implied, "CMC", 4),			//	3

		/* 4 */	INS(BIND(mov_b_b), Implied, "MOV B,B", 5),	INS(BIND(mov_b_c), Implied, "MOV B,C", 5),	INS(BIND(mov_b_d), Implied, "MOV B,D", 5),	INS(BIND(mov_b_e), Implied, "MOV B,E", 5),	INS(BIND(mov_b_h), Implied, "MOV B,H", 5),	INS(BIND(mov_b_l), Implied, "MOV B,L", 5),		INS(BIND(mov_b_m), Implied, "MOV B,M", 7),	INS(BIND(mov_b_a), Implied, "MOV B,A", 5),	INS(BIND(mov_c_b), Implied, "MOV C,B", 5),	INS(BIND(mov_c_c), Implied, "MOV C,C", 5),	INS(BIND(mov_c_d), Implied, "MOV C,D", 5),	INS(BIND(mov_c_e), Implied, "MOV C,E", 5),	INS(BIND(mov_c_h), Implied, "MOV C,H", 5),	INS(BIND(mov_c_l), Implied, "MOV C,L", 5),	INS(BIND(mov_c_m), Implied, "MOV C,M", 7),	INS(BIND(mov_c_a), Implied, "MOV C,A", 5),	//	4
		/* 5 */	INS(BIND(mov_d_b), Implied, "MOV D,B", 5),	INS(BIND(mov_d_c), Implied, "MOV D,C", 5),	INS(BIND(mov_d_d), Implied, "MOV D,D", 5),	INS(BIND(mov_d_e), Implied, "MOV D,E", 5),	INS(BIND(mov_d_h), Implied, "MOV D,H", 5),	INS(BIND(mov_d_l), Implied, "MOV D,L", 5),		INS(BIND(mov_d_m), Implied, "MOV D,M", 7),	INS(BIND(mov_d_a), Implied, "MOV D,A", 5),	INS(BIND(mov_e_b), Implied, "MOV E,B", 5),	INS(BIND(mov_e_c), Implied, "MOV E,C", 5),	INS(BIND(mov_e_d), Implied, "MOV E,D", 5),	INS(BIND(mov_e_e), Implied, "MOV E,E", 5),	INS(BIND(mov_e_h), Implied, "MOV E,H", 5),	INS(BIND(mov_e_l), Implied, "MOV E,L", 5),	INS(BIND(mov_e_m), Implied, "MOV E,M", 7),	INS(BIND(mov_e_a), Implied, "MOV E,A", 5),	//	5
		/* 6 */	INS(BIND(mov_h_b), Implied, "MOV H,B", 5),	INS(BIND(mov_h_c), Implied, "MOV H,C", 5),	INS(BIND(mov_h_d), Implied, "MOV H,D", 5),	INS(BIND(mov_h_e), Implied, "MOV H,E", 5),	INS(BIND(mov_h_h), Implied, "MOV H,H", 5),	INS(BIND(mov_h_l), Implied, "MOV H,L", 5),		INS(BIND(mov_h_m), Implied, "MOV H,M", 7),	INS(BIND(mov_h_a), Implied, "MOV H,A", 5),	INS(BIND(mov_l_b), Implied, "MOV L,B", 5),	INS(BIND(mov_l_c), Implied, "MOV L,C", 5),	INS(BIND(mov_l_d), Implied, "MOV L,D", 5),	INS(BIND(mov_l_e), Implied, "MOV L,E", 5),	INS(BIND(mov_l_h), Implied, "MOV L,H", 5),	INS(BIND(mov_l_l), Implied, "MOV L,L", 5),	INS(BIND(mov_l_m), Implied, "MOV L,M", 7),	INS(BIND(mov_l_a), Implied, "MOV L,A", 5),	//	6
		/* 7 */	INS(BIND(mov_m_b), Implied, "MOV M,B", 7),	INS(BIND(mov_m_c), Implied, "MOV M,C", 7),	INS(BIND(mov_m_d), Implied, "MOV M,D", 7),	INS(BIND(mov_m_e), Implied, "MOV M,E", 7),	INS(BIND(mov_m_h), Implied, "MOV M,H", 7),	INS(BIND(mov_m_l), Implied, "MOV M,L", 7),		INS(BIND(hlt), Implied, "HLT", 7),			INS(BIND(mov_m_a), Implied, "MOV M,A", 7),	INS(BIND(mov_a_b), Implied, "MOV A,B", 5),	INS(BIND(mov_a_c), Implied, "MOV A,C", 5),	INS(BIND(mov_a_d), Implied, "MOV A,D", 5),	INS(BIND(mov_a_e), Implied, "MOV A,E", 5),	INS(BIND(mov_a_h), Implied, "MOV A,H", 5),	INS(BIND(mov_a_l), Implied, "MOV A,L", 5),	INS(BIND(mov_a_m), Implied, "MOV A,M", 7),	INS(BIND(mov_a_a), Implied, "MOV A,A", 5),	//	7

		/* 8 */	INS(BIND(add_b), Implied, "ADD B", 4),		INS(BIND(add_c), Implied, "ADD C", 4),		INS(BIND(add_d), Implied, "ADD D", 4),		INS(BIND(add_e), Implied, "ADD E", 4),		INS(BIND(add_h), Implied, "ADD H", 4),		INS(BIND(add_l), Implied, "ADD L", 4),			INS(BIND(add_m), Implied, "ADD M", 7),		INS(BIND(add_a), Implied, "ADD A", 4),		INS(BIND(adc_b), Implied, "ADC B", 4),		INS(BIND(adc_c), Implied, "ADC C", 4),		INS(BIND(adc_d), Implied, "ADC D", 4),		INS(BIND(adc_e), Implied, "ADC E", 4),		INS(BIND(adc_h), Implied, "ADC H", 4),		INS(BIND(adc_l), Implied, "ADC L", 4),		INS(BIND(adc_m), Implied, "ADC M", 4),		INS(BIND(adc_a), Implied, "ADC A", 4),		//	8
		/* 9 */	INS(BIND(sub_b), Implied, "SUB B", 4),		INS(BIND(sub_c), Implied, "SUB C", 4),		INS(BIND(sub_d), Implied, "SUB D", 4),		INS(BIND(sub_e), Implied, "SUB E", 4),		INS(BIND(sub_h), Implied, "SUB H", 4),		INS(BIND(sub_l), Implied, "SUB L", 4),			INS(BIND(sub_m), Implied, "SUB M", 7),		INS(BIND(sub_a), Implied, "SUB A", 4),		INS(BIND(sbb_b), Implied, "SBB B", 4),		INS(BIND(sbb_c), Implied, "SBB C", 4),		INS(BIND(sbb_d), Implied, "SBB D", 4),		INS(BIND(sbb_e), Implied, "SBB E", 4),		INS(BIND(sbb_h), Implied, "SBB H", 4),		INS(BIND(sbb_l), Implied, "SBB L", 4),		INS(BIND(sbb_m), Implied, "SBB M", 4),		INS(BIND(sbb_a), Implied, "SBB A", 4),		//	9
		/* A */	INS(BIND(ana_b), Implied, "ANA B", 4),		INS(BIND(ana_c), Implied, "ANA C", 4),		INS(BIND(ana_d), Implied, "ANA D", 4),		INS(BIND(ana_e), Implied, "ANA E", 4),		INS(BIND(ana_h), Implied, "ANA H", 4),		INS(BIND(ana_l), Implied, "ANA L", 4),			INS(BIND(ana_m), Implied, "ANA M", 7),		INS(BIND(ana_a), Implied, "ANA A", 4),		INS(BIND(xra_b), Implied, "XRA B", 4),		INS(BIND(xra_c), Implied, "XRA C", 4),		INS(BIND(xra_d), Implied, "XRA D", 4),		INS(BIND(xra_e), Implied, "XRA E", 4),		INS(BIND(xra_h), Implied, "XRA H", 4),		INS(BIND(xra_l), Implied, "XRA L", 4),		INS(BIND(xra_m), Implied, "XRA M", 4),		INS(BIND(xra_a), Implied, "XRA A", 4),		//	A
		/* B */	INS(BIND(ora_b), Implied, "ORA B", 4),		INS(BIND(ora_c), Implied, "ORA C", 4),		INS(BIND(ora_d), Implied, "ORA D", 4),		INS(BIND(ora_e), Implied, "ORA E", 4),		INS(BIND(ora_h), Implied, "ORA H", 4),		INS(BIND(ora_l), Implied, "ORA L", 4),			INS(BIND(ora_m), Implied, "ORA M", 7),		INS(BIND(ora_a), Implied, "ORA A", 4),		INS(BIND(cmp_b), Implied, "CMP B", 4),		INS(BIND(cmp_c), Implied, "CMP C", 4),		INS(BIND(cmp_d), Implied, "CMP D", 4),		INS(BIND(cmp_e), Implied, "CMP E", 4),		INS(BIND(cmp_h), Implied, "CMP H", 4),		INS(BIND(cmp_l), Implied, "CMP L", 4),		INS(BIND(cmp_m), Implied, "CMP M", 4),		INS(BIND(cmp_a), Implied, "CMP A", 4),		//	B

		/* C */	INS(BIND(rnz), Implied, "RNZ", 5),			INS(BIND(pop_b), Implied, "POP B", 10),		INS(BIND(jnz), Absolute, "JNZ ", 10),		INS(BIND(jmp), Absolute, "JMP ", 10),		INS(BIND(cnz), Absolute, "CNZ ", 11),		INS(BIND(push_b), Implied, "PUSH B", 11),		INS(BIND(adi), Immediate, "ADI ", 7),		INS(BIND(rst_0), Implied, "RST 0", 11),		INS(BIND(rz), Implied, "RZ", 11),			INS(BIND(ret), Implied, "RET", 10),			INS(BIND(jz), Absolute, "JZ ", 10),			UNKNOWN(),									INS(BIND(cz), Absolute, "CZ ", 11),			INS(BIND(callDirect), Absolute, "CALL ", 17), INS(BIND(aci), Immediate, "ACI ", 7),		INS(BIND(rst_1), Implied, "RST 1", 11),		//	C
		/* D */	INS(BIND(rnc), Implied, "RNC", 5),			INS(BIND(pop_d), Implied, "POP D", 10),		INS(BIND(jnc), Absolute, "JNC ", 10),		INS(BIND(out), Immediate, "OUT ", 10),		INS(BIND(cnc), Absolute, "CNC ", 11),		INS(BIND(push_d), Implied, "PUSH D", 11),		INS(BIND(sui), Immediate, "SUI ", 7),		INS(BIND(rst_2), Implied, "RST 2", 11),		INS(BIND(rc), Implied, "RC", 11),			UNKNOWN(),									INS(BIND(jc), Absolute, "JC ", 10),			INS(BIND(in), Immediate, "IN ", 10),		INS(BIND(cc), Absolute, "CC ", 11),			UNKNOWN(),									INS(BIND(sbi), Immediate, "SBI ", 7),		INS(BIND(rst_3), Implied, "RST 3", 11),		//	D
		/* E */	INS(BIND(rpo), Implied, "RPO", 5),			INS(BIND(pop_h), Implied, "POP H", 10),		INS(BIND(jpo), Absolute, "JPO ", 10),		INS(BIND(xhtl),	Implied, "XHTL", 18),		INS(BIND(cpo), Absolute, "CPO ", 11),		INS(BIND(push_h), Implied, "PUSH H", 11),		INS(BIND(ani), Immediate, "ANI ", 7),		INS(BIND(rst_4), Implied, "RST 4", 11),		INS(BIND(rpe), Implied, "RPE", 11),			INS(BIND(pchl), Implied, "PCHL", 5),		INS(BIND(jpe), Absolute, "JPE ", 10),		INS(BIND(xchg), Implied, "XCHG", 4),		INS(BIND(cpe), Absolute, "CPE ", 11),		UNKNOWN(),									INS(BIND(xri), Immediate, "XRI ", 7),		INS(BIND(rst_5), Implied, "RST 5", 11),		//	E
		/* F */	INS(BIND(rp), Implied, "RP", 5),			INS(BIND(pop_psw), Implied, "POP PSW", 10),	INS(BIND(jp), Absolute, "JP ", 10),			INS(BIND(di), Implied, "DI ", 4),			INS(BIND(cp), Absolute, "CP ", 11),			INS(BIND(push_psw), Implied, "PUSH PSW", 11),	INS(BIND(ori), Immediate, "ORI ", 7),		INS(BIND(rst_6), Implied, "RST 6", 11),		INS(BIND(rm), Implied, "RM", 11),			INS(BIND(sphl), Implied, "SPHL", 5),		INS(BIND(jm), Absolute, "JM ", 10),			INS(BIND(ei), Implied, "EI", 4),			INS(BIND(cm), Absolute, "CM ", 11),			UNKNOWN(),									INS(BIND(cpi), Immediate, "CPI ", 7),		INS(BIND(rst_7), Implied, "RST 7", 11),		//	F
	};
}

void EightBit::Intel8080::initialise() {
	Processor::initialise();
	bc.word = de.word = hl.word = 0;
	a = f = 0;
}

int EightBit::Intel8080::step() {
	ExecutingInstruction.fire(*this);
	return execute(fetchByte());
}

int EightBit::Intel8080::execute(uint8_t opcode) {
	const auto& instruction = instructions[opcode];
	return execute(instruction);
}

//

void EightBit::Intel8080::___() {
	auto opcode = m_memory.get(pc.word - 1);
	auto message = Disassembler::invalid(opcode);
	throw std::domain_error(message);
}
