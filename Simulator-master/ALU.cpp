#include <stdio.h>
#include "defines.h"

int R[REG_SIZE] = { 0 };
const char* instName[] = { "nop", "sll", "srl", "sra", "slt", "mfhi", "mflo", "mul",
					"add", "sub", "jr", "syscall", "and", "or", "xor", "nor",
					"lb", "sb", "lw", "sw", "slti", "bltz", "j", "jal",
					"addi", "lbu", "beq", "bne", "andi", "ori", "xori", "lui" };
int LO = 0;
int HI = 0;

// Arithmetic unit
int addSubtract(int X, int Y, int C) {
	if (C < 0 || C > 1) {
		printf("error in addSubtract()...\n");
		return 0;
	}
	if (C == 0)
		return X + Y;
	else
		return X - Y;
}

// Logic unit
int logicOperation(int X, int Y, int C)
{
	if (C < 0 || C > 3) {
		printf("error in logicOperation()...\n");
		return 0;
	}

	if (C == 0)
		return X & Y;	// bitwise-AND
	else if (C == 1)
		return X | Y;   // bitwise-OR
	else if (C == 2)
		return X ^ Y;   // bitwise-XOR
	else
		return ~(X | Y); // bitwise-NOR
}

// Shift unit
// V: 5 bit shift amount
int shiftOperation(int V, int Y, int C)
{
	int ret;

	if (C < 0 || C > 3 || V > 31) {
		printf("error in shiftOperation()...\n");
		return 0;
	}

	if (C == 0) {	// No shift
		ret = Y;
	}
	else if (C == 1) {	// Logical left
		ret = Y << V;
	}
	else if (C == 2) {	// Logical right
		ret = (unsigned int)Y >> V;
	}
	else {	// Arithmetic right
		ret = Y >> V;
	}

	return ret;
}

int checkSetLess(int X, int Y)
{
	if (X < Y) return 1;
	else return 0;
}

int checkZero(int S)
{
	if (S == 0) return 1;
	else return 0;
}

// ALU

int ALU(int X, int Y, int C, int* Z) {
	int c32, c10;
	int ret;

	c32 = (C >> 2) & 3;	// 3, 2 bit of Control input
	c10 = C & 3;		// 1, 0 bit of Control input

	if (c32 == 0) { // shift

		ret = shiftOperation(X, Y, c10);
	}
	else if (c32 == 1) {  // set less
		ret = checkSetLess(X, Y);
	}
	else if (c32 == 2) {  // addsubtract
		ret = addSubtract(X, Y, c10 & 1);
		*Z = checkZero(ret);
	}
	else {  // logic
		ret = logicOperation(X, Y, c10);
	}

	return ret;
}

// debugging complete
