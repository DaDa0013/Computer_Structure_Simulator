#include <stdio.h>
#include "defines.h"
#define _CRT_SECURE_NO_WARNINGS


extern union instructionRegister inst;
extern int ALU(int X, int Y, int C, int* Z);
extern void setPC(unsigned int val);
extern int MEM(unsigned int A, int V, int nRW, int S);
extern unsigned int REG(unsigned int A, unsigned int V, unsigned int nRW);
extern unsigned int PC, IR;
int S;
int zero;
int* Z = &zero;


void executeShift(unsigned int op, unsigned int rd, unsigned int rt, unsigned int sht)
{
	printf("%s  $%d, $%d, %d\n", instName[op], rd, rt, sht);

	if (op == SLL) S = 1;
	else if (op == SRL) S = 2;
	else if (op == SRA) S = 3;
	int ret = ALU(sht, REG(rt, 0, 0), S, Z);
	REG(rd, ret, 1);
}

void executeJr(unsigned int op, unsigned int rs)
{
	printf("%s  $%d\n", instName[op], rs);
	setPC(REG(rs, 0, 0));
}

void executeSyscall(unsigned int op)
{
	printf("%s\n", instName[op]);
	IR = SYSCALL;
	PC -= 4;
}

void executeMfHiLo(unsigned int op, unsigned int rd)
{
	printf("%s $%d\n", instName[op], rd);
	if (op == MFHI) REG(rd, HI, 1);
	else if (op == MFLO) REG(rd, LO, 1);
}

void executeMul(unsigned int op, unsigned int rs, unsigned int rt)
{
	printf("%s  $%d, $%d\n", instName[op], rs, rt);
	long long result = REG(rs, 0, 0) * REG(rt, 0, 0);
	HI = result >> 32;
	LO = result & 0xFFFFFFFF;
}

void executeALU(unsigned int op, unsigned int rd, unsigned int rs, unsigned int rt)
{
	printf("%s  $%d, $%d, $%d\n", instName[op], rd, rs, rt);

	if (op == ADD) S = 8;
	else if (op == SUB) S = 9;
	else if (op == AND) S = 12;
	else if (op == OR) S = 13;
	else if (op == XOR) S = 14;
	else if (op == NOR) S = 15;
	else if (op == SLT) S = 5;

	int ret = ALU(REG(rs, 0, 0), REG(rt, 0, 0), S, Z);
	REG(rd, ret, 1);
}

void executeALUI(unsigned int op, unsigned int rt, unsigned int rs, short immediate)
{
	printf("%s  $%d, $%d, %d\n", instName[op], rt, rs, immediate);
	int ret;
	if (op == LUI) {
		ret = (immediate << 16);
	}
	else {
		if (op == ADDI) S = 8;
		else if (op == SLTI) S = 5;
		else if (op == ANDI) S = 12;
		else if (op == ORI) S = 13;
		else if (op == XORI) S = 14;

		ret = ALU(R[rs], immediate, S, Z);
	}
	REG(rt, ret, 1);
}

void executeBranch(unsigned int op, unsigned int rs, unsigned int rt, short offset)
{
	if (op == BLTZ) {
		printf("%s  $%d, %d\n", instName[op], rs, offset << 2); // print shifted offset
		int ret = ALU(REG(rs, 0, 0), 0, 9, Z);
		if (ret == 1) setPC(PC + (offset << 2));
	}
	else {
		printf("%s  $%d, $%d, %d\n", instName[op], rs, rt, offset << 2); // print shifted offset

		ALU(REG(rs, 0, 0), REG(rt, 0, 0), 9, Z);
		if (op == BEQ) {
			if (*Z == 1) setPC(PC + (offset << 2));
		}
		else if (op == BNE) {
			if (*Z == 0) setPC(PC + (offset << 2));
		}
	}
}

void executeJump(unsigned int op, unsigned int offset)
{
	printf("%s 0x%08X\n", instName[op], offset << 2);	// print shifted offset
	if (op == J) setPC((PC & 0xC0000000) | (offset << 2));
	else if (op == JAL) {
		REG(REG_SIZE - 1, PC, 1);
		setPC((PC & 0xC0000000) | (offset << 2));
	}

}

void executeLoadStore(unsigned int op, unsigned int rt, unsigned int rs, short offset)
{
	printf("%s  $%d, %d($%d)\n", instName[op], rt, offset, rs);
	if (op == LB) REG(rt, MEM(REG(rs, 0, 0) + offset, 0, 0, 0), 1);
	else if (op == LW) REG(rt, MEM(REG(rs, 0, 0) + offset, 0, 0, 2), 1);
	else if (op == LBU) REG(rt, MEM(REG(rs, 0, 0) + offset, 0, 0, 0), 1);
	else if (op == SB) MEM(R[rs] + offset, REG(rt, 0, 0), 1, 0);
	else if (op == SW) MEM(R[rs] + offset, REG(rt, 0, 0), 1, 2);
}

// R-type instructin decoding
void decodeRtype(unsigned int fct)
{
	unsigned int fcth, fctl;

	fctl = fct & 0x7;
	fcth = (fct & 0x38) >> 3;

	if (fcth == 0) {
		if (fctl == 0)      executeShift(SLL, inst.RI.rd, inst.RI.rt, inst.RI.sht);
		else if (fctl == 2) executeShift(SRL, inst.RI.rd, inst.RI.rt, inst.RI.sht);
		else if (fctl == 3) executeShift(SRA, inst.RI.rd, inst.RI.rt, inst.RI.sht);
		else {
			printf("Undefined instruction\n");
			return;
		}
	}
	else if (fcth == 1) {
		if (fctl == 0)      executeJr(JR, inst.RI.rs);
		else if (fctl == 4) executeSyscall(SYSCALL);
		else printf("Undefined instruction\n");
	}
	else if (fcth == 2) {
		if (fctl == 0)      executeMfHiLo(MFHI, inst.RI.rd);
		else if (fctl == 2) executeMfHiLo(MFLO, inst.RI.rd);
		else printf("Undefined instruction\n");
	}
	else if (fcth == 3) {
		if (fctl == 0)      executeMul(MUL, inst.RI.rs, inst.RI.rt);
		else printf("Undefined instruction\n");
	}
	else if (fcth == 4) {
		if (fctl == 0)      executeALU(ADD, inst.RI.rd, inst.RI.rs, inst.RI.rt);
		else if (fctl == 2) executeALU(SUB, inst.RI.rd, inst.RI.rs, inst.RI.rt);
		else if (fctl == 4) executeALU(AND, inst.RI.rd, inst.RI.rs, inst.RI.rt);
		else if (fctl == 5) executeALU(OR, inst.RI.rd, inst.RI.rs, inst.RI.rt);
		else if (fctl == 6) executeALU(XOR, inst.RI.rd, inst.RI.rs, inst.RI.rt);
		else if (fctl == 7) executeALU(NOR, inst.RI.rd, inst.RI.rs, inst.RI.rt);
		else printf("Undefined instruction\n");
	}
	else if (fcth == 5) {
		if (fctl == 2)      executeALU(SLT, inst.RI.rd, inst.RI.rs, inst.RI.rt);
		else printf("Undefined instruction\n");
	}
	else printf("Undefined instruction\n");
}

// instruction decoding
void instructionDecode(void)
{
	unsigned int opc, fct;

	unsigned int opch, opcl;

	opc = inst.RI.opc;
	fct = inst.RI.fct;

	printf("Opc: %2x, Fct: %2x, Inst: ", opc, fct);

	opcl = opc & 0x7;
	opch = (opc & 0x38) >> 3;

	if (opch == 0) {
		if (opcl == 0) {

			decodeRtype(fct);
		}
		else if (opcl == 1) executeBranch(BLTZ, inst.II.rs, inst.II.rt, inst.II.offset);
		else if (opcl == 2) executeJump(J, inst.JI.jval);
		else if (opcl == 3) executeJump(JAL, inst.JI.jval);
		else if (opcl == 4) executeBranch(BEQ, inst.II.rs, inst.II.rt, inst.II.offset);
		else if (opcl == 5) executeBranch(BNE, inst.II.rs, inst.II.rt, inst.II.offset);
		else printf("Undefined instruction\n");
	}
	else if (opch == 1) {
		if (opcl == 0)      executeALUI(ADDI, inst.II.rt, inst.II.rs, inst.II.offset);
		else if (opcl == 2) executeALUI(SLTI, inst.II.rt, inst.II.rs, inst.II.offset);
		else if (opcl == 4) executeALUI(ANDI, inst.II.rt, inst.II.rs, inst.II.offset);
		else if (opcl == 5) executeALUI(ORI, inst.II.rt, inst.II.rs, inst.II.offset);
		else if (opcl == 6) executeALUI(XORI, inst.II.rt, inst.II.rs, inst.II.offset);
		else if (opcl == 7) executeALUI(LUI, inst.II.rt, inst.II.rs, inst.II.offset);
		else printf("Undefined instruction\n");
	}
	else if (opch == 4) {
		if (opcl == 0)      executeLoadStore(LB, inst.II.rt, inst.II.rs, inst.II.offset);
		else if (opcl == 3) executeLoadStore(LW, inst.II.rt, inst.II.rs, inst.II.offset);
		else if (opcl == 4) executeLoadStore(LBU, inst.II.rt, inst.II.rs, inst.II.offset);
		else printf("Undefined instruction\n");
	}
	else if (opch == 5) {
		if (opcl == 0)      executeLoadStore(SB, inst.II.rt, inst.II.rs, inst.II.offset);
		else if (opcl == 3) executeLoadStore(SW, inst.II.rt, inst.II.rs, inst.II.offset);
		else printf("Undefined instruction\n");
	}
	else printf("Undefined instruction\n");
}
