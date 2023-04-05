#include <stdio.h>
#include "defines.h"
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS

unsigned int PC, IR;
union instructionRegister inst;

extern int MEM(unsigned int A, int V, int nRW, int S);
extern unsigned int REG(unsigned int A, unsigned int V, unsigned int nRW);
extern void resetRegister();
extern void resetMem(void);
extern unsigned int invertEndian(unsigned int data);
extern void instructionDecode(void);

unsigned char progMEM[M_SIZE];
unsigned char dataMEM[M_SIZE];
unsigned char stakMEM[M_SIZE];

unsigned int data;
unsigned int addr;

unsigned int iCount;	// # of instructions
unsigned int dCount;	// # of data

void showRegister(void) {
    printf("PC:\t0x%.8x\n", PC);
    printf("HI:\t0x%.8x\nLO:\t0x%.8x\n", HI, LO);
    for (int i = 0; i < REG_SIZE; i++) {
        printf("R[%d]:\t0x%.8x\n", i, REG(i, 0, 0));
    }
}

void setPC(unsigned int val) {
    PC = val;
    return;
}

void step() {
    inst.I = MEM(PC, 0, 0, 2);
    PC += 4;
    instructionDecode();
}

void main()
{
    printf("\t<Command List>\n");
    printf("=================================\n");
    printf("| sr:\tset Register\t\t|\n");
    printf("| sm:\tset Memory\t\t|\n");
    printf("|  l:\tload to Memory\t\t|\n");
    printf("|  j:\tjump to Address\t\t|\n");
    printf("|  g:\trun to the end\t\t|\n");
    printf("|  s:\trun one step\t\t|\n");
    printf("|  m:\tshow Memory\t\t|\n");
    printf("|  r:\tshow Register\t\t|\n");
    printf("|  x:\texit\t\t\t|\n");
    printf("=================================\n");
    while (1) {
        // Get command line
        char command[10];
        printf("command: ");
        scanf("%s", command);
        getchar();

        if (strcmp(command, "sr") == 0) {
            unsigned int registerNumber, value;
            printf("Register Number: ");
            scanf("%d", &registerNumber);
            printf("Value: ");
            scanf("%d", &value);
            printf("R[%d] = %d\n", registerNumber, value);
            REG(registerNumber, value, 1);
        }
        else if (strcmp(command, "sm") == 0) {
            unsigned int location, value;
            printf("Location: 0x");
            scanf("%x", &location);
            printf("Value: ");
            scanf("%d", &value);
            printf("MEM[0x%.8x] = %d\n", location, value);
            MEM(location, value, 1, 2);
        }
        else if (strcmp(command, "l") == 0) {
            resetMem();
            resetRegister();
            char fileName[25];
            int fileNumber;
            FILE* fp;
            printf("\t<File List>\n");
            printf("=================================\n");
            printf("| 1:\tas_ex01_arith.bin\t|\n");
            printf("| 2:\tas_ex02_logic.bin\t|\n");
            printf("| 3:\tas_ex03_ifelse.bin\t|\n");
            printf("| 4:\tas_ex04_fct.bin\t\t|\n");
            printf("=================================\n");
            printf("Enter File Number: ");
            scanf("%d", &fileNumber);
            getchar();
            if(fileNumber == 1) strcpy(fileName , ".\\as_ex01_arith.bin");
            else if (fileNumber == 2) strcpy(fileName, ".\\as_ex02_logic.bin");
            else if (fileNumber == 3) strcpy(fileName, ".\\as_ex03_ifelse.bin");
            else if (fileNumber == 4) strcpy(fileName, ".\\as_ex04_fct.bin");
            else {
                printf("wrong File Number\n");
                continue;
            }
            
            errno_t err;
            err = fopen_s(&fp, fileName, "rb");
            if (err != 0) {
                printf("Cannot open file: %s with errno %d\n", fileName, err);
                return;
            }

            // read instruction and data numbers
            fread(&data, sizeof(data), 1, fp);
            iCount = invertEndian(data);
            fread(&data, sizeof(data), 1, fp);
            dCount = invertEndian(data);
            printf("Number of Instructions: %d, Number of Data: %d\n", iCount, dCount);
            REG(31, PC + 4 * (iCount - 1), 1);

            // Load to memory
            addr = 0;
            for (int i = 0; i < (int)iCount; i++) {
                fread(&data, sizeof(unsigned int), 1, fp);
                data = invertEndian(data);
                MEM(PROG_START + addr, data, 1, 2);
                addr += 4;
            }

            addr = 0;
            for (int i = 0; i < (int)dCount; i++) {
                fread(&data, sizeof(unsigned int), 1, fp);
                data = invertEndian(data);
                MEM(DATA_START + addr, data, 1, 2);
                addr += 4;
            }
            setPC(PROG_START);

            fclose(fp);
        }
        else if (strcmp(command, "j") == 0) {
            unsigned int jaddr;
            printf("Enter Jump Address: 0x");
            scanf("%x", &jaddr);
            setPC(jaddr);
        }
        else if (strcmp(command, "g") == 0) {
            while (IR != SYSCALL) step();
            IR = NULL;
        }
        else if (strcmp(command, "s") == 0) {
            step();
            IR = NULL;
        }
        else if (strcmp(command, "m") == 0) {
            unsigned int start, end;
            printf("Start: 0x");
            scanf("%x", &start);
            printf("End: 0x");
            scanf("%x", &end);

            for (unsigned int addr = start; addr < end; addr += 4) {
                printf("MEM[0x%x]: 0x%.8x\n", addr, MEM(addr, 0, 0, 2));
            }
        }
        else if (strcmp(command, "r") == 0) showRegister();
        else if (strcmp(command, "x") == 0) break;
        else printf("Wrong Command\n");
    }
    return;
}