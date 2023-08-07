#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "function.h"
rename_table_entry *rename_table;
int32_t *regs;

void init_memory() {
    // 初始化 Cache INSTR
    for (int i = 0; i < 512; i++) {
        INSTR[i].INST = 0;
        INSTR[i].MeM_addr = 0;
    }

    // 初始化 RF
    for (int i = 0; i < 32; i++) {
        RF[i] = 0;
    }

    // 初始化 MEM
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 4; j++) {
            MEM[i][j] = 0;
        }
    }
}

void init_globals() {
    rename_table = malloc(NUM_RENAME_REGS * sizeof(rename_table_entry));

    regs = (int32_t *)malloc(NUM_REGS * sizeof(int32_t));
}





int load_program(const char *filename, int MEM[][4], int max_instr) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open file %s\n", filename);
        return 1;
    }

    int i = 0;
    while (i < max_instr && fscanf(file, "%x %x %x %x", &MEM[i][0], &MEM[i][1], &MEM[i][2], &MEM[i][3]) == 4) {
        i++;
    }

    fclose(file);
    return 0;
}


void Ld_cache(int32_t MEM[][4], Cache INSTR[], int P_m) {
    for (int i = 0; i < 4; i++) {
        // 将4个内存单元组合成一个32位值
        INSTR[P_m].INST = (MEM[P_m][0] << 24) | (MEM[P_m][1] << 16) | (MEM[P_m][2] << 8) | MEM[P_m][3];
        INSTR[P_m].MeM_addr = P_m;
        INSTR[P_m].j = 0;
        
    }
   // printf("INSTR[%d].INST: %08X\n", P_m , INSTR[P_m].INST);
}

const char* OpcodeNames[] = {
    "NOP", "ADD", "SUB", "MUL", "DIV", "AND", "OR", "XOR", "NOT", 
    "MOV", "LOAD", "STORE", "JZC", "JNZ", "JZL", "JMP", "HALT", 
    "ADDi", "SUBi", "MULi", "DIVi", "JZCi", "MOVi", "JZLi"
};


void fetch(Cache INSTR, bool *is_busy , int PC,int32_t *fetched){
	if (!*is_busy) {
        // 获取新指令
        printf("Fetching instruction at address \033[33m%d\033[0m\n", PC);
		*fetched = INSTR.INST;
        printf("Fetched instruction: \033[96m%08x\033[0m  ", *fetched);
		INSTR.j=INSTR.j+1;
       
    }

}


void decode(int32_t fetched, IssueQueue *Isq) {

    int op3 = fetched & 0xff;             // 从指令中提取操作数3,直接取8位
    int op2 = (fetched >> 8) & 0xff;     // 从指令中提取操作数2，右移8位取8位
    int op1 = (fetched >> 16) & 0xff;     // 从指令中提取操作数1,右移16位取8位
    int opcode = (fetched >> 24) & 0xff;  // 从指令中提取opcode,右移24位取剩下的


    printf("Decoded opcode: %x", opcode);
    switch(opcode)
	{
	case 0x80:Isq->Opcode=NOP;                                          break;
	case 0x21:Isq->Opcode=ADD,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]+R[s2]=R[r]
    case 0x11:Isq->Opcode=ADDi,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]+s2=R[r]
	case 0x22:Isq->Opcode=SUB,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]-R[s2]=R[r]
    case 0x12:Isq->Opcode=SUBi,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]-s2=R[r]
                
	case 0x23:Isq->Opcode=MUL,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]xR[s2]=R[r] 
    case 0x13:Isq->Opcode=MULi,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]xs2=R[r] 
	case 0x24:Isq->Opcode=DIV,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]/R[s2]=R[r]
    case 0x14:Isq->Opcode=DIVi,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]/s2=R[r]
	case 0x25:Isq->Opcode=AND,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]&&R[s2]=R[r]
	case 0x26:Isq->Opcode=OR,     Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]||R[s2]=R[r]
	case 0x27:Isq->Opcode=XOR,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//R[s1]^R[s2]=R[r]
	case 0x28:Isq->Opcode=NOT,    Isq->s1=op1,  Isq->r=op3;             break; //Invert by position store to r
	case 0x29:Isq->Opcode=MOV,    Isq->s1=op1,  Isq->s2=op2;            break;// move from R[S1] to R[S2]
    case 0x19:Isq->Opcode=MOVi,   Isq->s1=op1,  Isq->s2=op2;            break;// move from R[RF[S1]] to R[RF[S2]]
	case 0x39:Isq->Opcode=LOAD,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;//IF !s2==0, Loadi S1 to r 
	case 0x3a:Isq->Opcode=STORE,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;// STORE from R[S1]+s2 to M[r]
	case 0x3b:Isq->Opcode=JMP,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3; break;                     //UNCONDITIONAL_JUMP jump to s1
    case 0x3c:Isq->Opcode=JZL,    Isq->s1=op1,  Isq->s2=op2, Isq->r=op3;break;//CONDITIONAL_JUMP if S1>S2,          jump to r
    case 0x2c:Isq->Opcode=JZLi,   Isq->s1=op1,  Isq->s2=op2, Isq->r=op3;break;//CONDITIONAL_JUMP if RF[S1]>RF[S2],  jump to r
	case 0x3d:Isq->Opcode=JZC,    Isq->s1=op1,  Isq->s2=op2, Isq->r=op3;break; //BRANCH_ZERO iF R[op1]-R[op2]=0,    jump to r 
	case 0x1d:Isq->Opcode=JZCi,   Isq->s1=op1,  Isq->s2=op2, Isq->r=op3;break; //BRANCH_ZERO_direct iF R[op1]-op2=0,jump to r 
    case 0x3e:Isq->Opcode=JNZ,    Isq->s1=op1,  Isq->s2=op2, Isq->r=op3;break; //BRANCH_NOT_ZERO: if R[S1]-R[S2]!=0 ，jump to r
    case 0x3f:Isq->Opcode=HALT;     break;
    default:
    printf("Unknown opcode: %d\n", opcode);
    break;
	}
    printf(" =>\033[34m%s\033[0m\n", OpcodeNames[Isq->Opcode]);
}


// void init_rename_table() {
// for (int i = 0; i < NUM_RENAME_REGS; i++) {
// rename_table[i].rename_reg = i;
// rename_table[i].phys_reg = i;
// rename_table[i].SRC_ins_addr = -1; // 初始化源指令地址为-1，表示未分配
// }
// }
// void free_phys_reg(int32_t phys_reg) {
//     for (int i = 0; i < NUM_RENAME_REGS; i++) {
//         if (rename_table[i + NUM_RENAME_REGS].phys_reg == phys_reg) {
//         rename_table[i + NUM_RENAME_REGS].phys_reg = -1;
//         rename_table[i + NUM_RENAME_REGS].SRC_ins_addr = -1;
//         break;
//         }
//     }
// }
// int32_t update_rename_table(uint32_t reg, int32_t new_phys_reg, uint32_t ins_addr) {
// int32_t old_phys_reg = rename_table[reg].phys_reg;
// rename_table[reg].phys_reg = new_phys_reg;
// rename_table[reg].SRC_ins_addr = ins_addr; // 记录源指令地址
// free_phys_reg(old_phys_reg);
// return old_phys_reg;
// }
// int32_t read_reg(uint32_t reg) {
// int32_t phys_reg = rename_table[reg].phys_reg;
// return regs[phys_reg];
// }

// void write_reg(uint32_t reg, int32_t value, uint32_t ins_addr) {
// int32_t phys_reg = rename_table[reg].phys_reg;
// regs[phys_reg] = value;
// rename_table[reg].SRC_ins_addr = ins_addr; // 更新源指令地址
// }

// int32_t allocate_phys_reg(uint32_t ins_addr) {
// // 在物理寄存器池中查找一个空闲寄存器并返回其编号
// for (int i = 0; i < NUM_REGS; i++) {
// if (rename_table[i + NUM_RENAME_REGS].phys_reg == -1) {
// rename_table[i + NUM_RENAME_REGS].phys_reg = i;
// rename_table[i + NUM_RENAME_REGS].SRC_ins_addr = ins_addr; // 记录源指令地址
// return i;
// }
// }
// return -1; // 没有可用的物理寄存器
// }

// int32_t rename_reg(uint32_t reg, uint32_t ins_addr) {
//     int32_t phys_reg = allocate_phys_reg(ins_addr); // 分配一个空闲的物理寄存器
//     return update_rename_table(reg, phys_reg, ins_addr); // 返回新分配的物理寄存器编号
// }

void write_memory_to_file(const char* filename, int (*Mem)[4], uint32_t size) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("Error: Failed to open file for writing\n");
        return;
    }

    for (uint32_t i = 0; i < size; ++i) {
        fprintf(fp, "%x %x %x %x\n", Mem[i][0], Mem[i][1], Mem[i][2], Mem[i][3]);
    }

    fclose(fp);
}
void Execute(IssueQueue Isq, int *RF, int32_t (*MEM)[4], int PC,int *next_PC, _Bool *finished,int *error)
{
	
	switch(Isq.Opcode)
	{
		case ADD://21
			RF[Isq.r] = RF[Isq.s1] + RF[Isq.s2]; *next_PC=PC + 1; break;
        case SUB://22
			RF[Isq.r] = RF[Isq.s1] - RF[Isq.s2]; *next_PC=PC + 1; break;    

		case MUL://23
			RF[Isq.r] = RF[Isq.s1] * RF[Isq.s2]; *next_PC=PC + 1; break;
        case DIV://24
			RF[Isq.r] = RF[Isq.s1] / RF[Isq.s2]; *next_PC=PC + 1; break;
        case ADDi://11
			RF[Isq.r] = RF[Isq.s1] + Isq.s2; *next_PC=PC + 1; break;
        case SUBi://12
			RF[Isq.r] = RF[Isq.s1] - Isq.s2; *next_PC=PC + 1; break;    
		case MULi://13
			RF[Isq.r] = RF[Isq.s1] * Isq.s2; *next_PC=PC + 1; break;
        case DIVi://14
			RF[Isq.r] = RF[Isq.s1] / Isq.s2; *next_PC=PC + 1; break;
        case AND://25
			RF[Isq.r] = RF[Isq.s1] && RF[Isq.s2]; *next_PC=PC + 1; break; 
        case OR://26
			RF[Isq.r] = RF[Isq.s1] || RF[Isq.s2]; *next_PC=PC + 1; break;  
             
        case XOR://27
			RF[Isq.r] = RF[Isq.s1] ^ RF[Isq.s2]; *next_PC=PC + 1; break;
        case NOT://28
			RF[Isq.r] = ~RF[Isq.s1] ; *next_PC=PC + 1; break;
        case MOV://29
		RF[Isq.s2] =RF[Isq.s1]; *next_PC=PC + 1; break;
         case MOVi://19
                RF[RF[Isq.s2]] = RF[RF[Isq.s1]]; *next_PC=PC + 1; break;//printf("RF[Isq.r]: %x,RF[RF[Isq.r]]:%x\n",RF[Isq.r],RF[(RF[Isq.r])])
		case LOAD://39
        {   
            if (Isq.s2==0)//LOAD  Isq.r to Isq.s1v
            {
                RF[Isq.r] = (MEM[Isq.s1][0] << 24) | (MEM[Isq.s1][1] << 16) | (MEM[Isq.s1][2] << 8) | MEM[Isq.s1][3];
            *next_PC = PC + 1;
            }else{//LOADi//
                RF[Isq.r] = Isq.s1;
            *next_PC = PC + 1;
            }
            
        }
        break;

		case STORE://3a
        {   printf("s1: %d,s2:%d,r:%d\n",Isq.s1,Isq.s2,Isq.r);
            int32_t mem_addr = RF[Isq.s1] + Isq.s2-1;//save to memory output line : RF[s1]+s2
            int32_t data = RF[Isq.r];
            MEM[mem_addr][0] = (data >> 24) & 0xFF;
            MEM[mem_addr][1] = (data >> 16) & 0xFF;
            MEM[mem_addr][2] = (data >> 8) & 0xFF;
            MEM[mem_addr][3] = data & 0xFF;
            *next_PC = PC + 1;
        }
        break;
	    case JMP://UNCONDITIONAL_JUMP//3b
			*next_PC = Isq.r; break;

		case JZL://CONDITIONAL_JUMP//3C
			if (RF[Isq.s1] > RF[Isq.s2]) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
        case JZLi://CONDITIONAL_JUMP//2C
			if (RF[(RF[Isq.s1])] > RF[(RF[Isq.s2])]) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
        case JZC://CONDITIONAL_JUMP Equal/compare//3D
			if (RF[Isq.s1] - RF[Isq.s2] == 0) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
        case JZCi://CONDITIONAL_JUMP Equal/compar//1d
	    if (RF[Isq.s1] - Isq.s2 == 0) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
		case JNZ://BRANCH_NOT_ZERO not Equal/compare//3E
			if (RF[Isq.s1]-RF[Isq.s2] != 0) *next_PC = Isq.r; else *next_PC=PC + 1; break;

	
		case HALT://3F
			*finished = 1; *next_PC=PC;printf("Finished OP_mode : HALT") ;break;
        case NOP://80
             *next_PC= PC+1; break;
		default:
			printf("Error: OP_mode not recognised: %d\n",Isq.Opcode); *error = 1; break;
			
	}
	
}
