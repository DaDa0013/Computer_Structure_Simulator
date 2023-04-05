#include <stdio.h>
#include "defines.h"
// debugging complete
extern unsigned int PC, IR;

unsigned int REG(unsigned int A, unsigned int V, unsigned int nRW) {
	if (A < 0 || A > REG_SIZE) {
		printf("Register Index Error\n");
		return -1;
	}
	if (nRW > 1) {
		printf("Invalid Control Number\n");
		return -1;
	}

	if (nRW == 0) { // read
		return R[A];
	}
	else if (nRW == 1) { // write
		R[A] = V;
	}
	return 0;
}

void resetRegister() {
	PC = 0x40000000;
	IR = NULL;
	LO = 0;
	HI = 0;

	for (int i = 0; i < REG_SIZE; i++) {
		REG(i, 0, 1);
	}
	REG(29, 0x80000000, 1);
}
