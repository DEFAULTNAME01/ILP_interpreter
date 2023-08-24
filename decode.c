#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "IssueQueue.h"
#include "ROB.h"
#define Cache_SIZE 512
#define BPT_SIZE 1024  
#define PCbuf_SIZE 64  
#define FETCH_GROUP_SIZE 4

#define NUM_LOGICAL_REGISTERS 32 // 假设有32个逻辑寄存器
#define NUM_PHYSICAL_REGISTERS 64

int physical_register_pool[NUM_PHYSICAL_REGISTERS];
int physical_register_pointer = 0; // 指向当前可用的物理寄存器
typedef struct {
  int32_t INST;
  uint16_t MeM_addr;
  uint16_t j;
} Cache;


extern Cache INSTR[Cache_SIZE];

typedef struct {
    uint32_t address;   // 存储指令地址PC
    uint8_t counter;    // 2-bit saturating counter  History
} BPTEntry;

BPTEntry branchPredictionTable[BPT_SIZE];

typedef struct {
    uint32_t address;   // 存储指令地址PC
    bool isSpeculate;
} PredictedPCEntry;

typedef struct {
    PredictedPCEntry *predictedPCs;
    int size;
    int front;
    int rear;
} PC_buffer;

typedef struct {
    int logical_register;   // 逻辑寄存器索引
    int physical_register;  // 物理寄存器索引
    int valid;              // 有效位，例如使用1表示有效，0表示无效
    bool isSpeculative;
} RAT_Entry;


RAT_Entry RAT[NUM_LOGICAL_REGISTERS];
void initialize_RAT() {
    for (int i = 0; i < NUM_LOGICAL_REGISTERS; i++) {
        RAT[i].logical_register = i;
        RAT[i].physical_register = -1; // -1表示当前没有映射到物理寄存器
        RAT[i].valid = 0;              // 0表示无效
        RAT[i].isSpeculative = false;
    }
}
int get_physical_register(int logical_register) {
    if (RAT[logical_register].valid) {
        return RAT[logical_register].physical_register;
    } else {
        return -1; // 返回-1表示没有映射到物理寄存器
    }
}

void initialize_physical_registers() {
    for (int i = 0; i < NUM_PHYSICAL_REGISTERS; i++) {
        physical_register_pool[i] = i;
    }
    physical_register_pointer = 0;
}

int allocate_physical_register() {
    if (physical_register_pointer < NUM_PHYSICAL_REGISTERS) {
        return physical_register_pool[physical_register_pointer++];
    } else {
        // 物理寄存器耗尽
        return -1;
    }
}

void release_physical_register(int physical_register) {
    physical_register_pool[--physical_register_pointer] = physical_register;
}

void update_mapping(int logical_register,bool speculative) {
    // 在更新映射前，先释放之前的物理寄存器
    if (RAT[logical_register].valid) {
        release_physical_register(RAT[logical_register].physical_register);
    }

    int new_physical_register = allocate_physical_register();
    if (new_physical_register != -1) {
        RAT[logical_register].physical_register = new_physical_register;
        RAT[logical_register].valid = 1;
        RAT[logical_register].isSpeculative = speculative;
    } else {
        // 处理物理寄存器耗尽的情况
    }

}
// 当分支预测错误或其他撤销情况发生时
void rollback_speculative_mappings() {
    for (int i = 0; i < NUM_LOGICAL_REGISTERS; i++) {
        if (RAT[i].isSpeculative) {
            RAT[i].physical_register = -1;
            RAT[i].valid = false;
            RAT[i].isSpeculative = false;
        }
    }
}
PC_buffer* initiatePC_buffer() {
    PC_buffer* PC_b = (PC_buffer*)malloc(sizeof(PC_buffer));
    
    // 分配内存空间给PredictedPCEntry数组
    PC_b->predictedPCs = (PredictedPCEntry*)malloc(PCbuf_SIZE * sizeof(PredictedPCEntry));
    
    // 初始化PredictedPCEntry数组的每一个元素
    for(int i = 0; i < PCbuf_SIZE; i++) {
        PC_b->predictedPCs[i].address = 0;           // 为address字段设置一个默认值，例如0
        PC_b->predictedPCs[i].isSpeculate = false;   // 设置isSpeculate字段为false
    }
    
    PC_b->size = PCbuf_SIZE;
    PC_b->front = 0;
    PC_b->rear = 0;

    return PC_b;
}



uint32_t getPCForFetch(PC_buffer* buffer) {
    // 取出当前front指向的地址
    uint32_t fetchedPC = buffer->predictedPCs[buffer->front].address;

    // 将队列的头部指针前移
    buffer->front++;
    if (buffer->front >= buffer->size) {
        buffer->front = 0; // 如果到达队列尾部，就回到开始位置
    }

    // 在队列尾部添加一个新元素，这个元素的地址是前一个元素的地址+1，并且设置isSpeculate为true
    buffer->predictedPCs[buffer->rear].address = fetchedPC + 1;
    buffer->predictedPCs[buffer->rear].isSpeculate = true;

    // 更新队列的尾部指针
    buffer->rear++;
    if (buffer->rear >= buffer->size) {
        buffer->rear = 0; // 如果到达队列尾部，就回到开始位置
    }

    return fetchedPC;
}


typedef struct {
    int32_t inst;
    int pc_from; // instruction address
    bool is_speculative; // if true that may flush when branch misprediction
} FetchedEntry;

FetchedEntry fetchGroup[FETCH_GROUP_SIZE];

void fetch(Cache INSTR, bool *is_busy, PC_buffer* buffer, FetchedEntry fetchedGroup[FETCH_GROUP_SIZE]) {
    if (!*is_busy) {
        for (int i = 0; i < FETCH_GROUP_SIZE; i++) {
            // 获取新的PC值
            uint32_t PC = getPCForFetch(buffer);
            
            // 获取新指令
            printf("Fetching instruction at address \033[33m%d\033[0m\n", PC);
            fetchedGroup[i].inst = INSTR.INST;
            fetchedGroup[i].pc_from = PC;
            fetchedGroup[i].is_speculative = buffer->predictedPCs[buffer->front].isSpeculate;
            printf("Fetched instruction: \033[96m%08x\033[0m  ", fetchedGroup[i].inst);
            
            
            INSTR.j = INSTR.j + 1;
        }
    }
}
typedef enum {
    NOP,ADD,SUB,MUL,DIV,AND,OR,XOR,NOT,MOV,LOAD,STORE,JZC,JNZ,JZL,JMP, HALT,
    ADDi,SUBi,MULi,DIVi,JZCi,MOVi,JZLi,begin, end,
} OpCode;

const char* OpcodeNames[] = {
    "NOP", "ADD", "SUB", "MUL", "DIV", "AND", "OR", "XOR", "NOT", 
    "MOV", "LOAD", "STORE", "JZC", "JNZ", "JZL", "JMP", "HALT", 
    "ADDi", "SUBi", "MULi", "DIVi", "JZCi", "MOVi", "JZLi","begin","end",
};
void decode(FetchedEntry fetched,  IssueQueue *Isq, ReorderBuffer* rob) {

    IQEntry iqEntry = {0};
    RoBEntry robEntry = {0};

    int op3 = fetched.inst & 0xff;             // 从指令中提取操作数3,直接取8位
    int op2 = (fetched.inst >> 8) & 0xff;     // 从指令中提取操作数2，右移8位取8位
    int op1 = (fetched.inst >> 16) & 0xff;     // 从指令中提取操作数1,右移16位取8位
    int opcode = (fetched.inst >> 24) & 0xff;  // 从指令中提取opcode,右移24位取剩下的
    OpCode operation;
    int r;
    switch(opcode)
	{
    case 0x00:operation=end;                                          break;    
    case 0x10:operation=begin;                                        break;
	case 0x80:operation=NOP;                                          break;
	case 0x21:operation=ADD; break;//R[s1]+R[s2]=R[r]
    case 0x11:operation=ADDi,iqEntry.valid_Op2=true; break;//R[s1]+s2=R[r]
	case 0x22:operation=SUB; break;//R[s1]-R[s2]=R[r]
    case 0x12:operation=SUBi,iqEntry.valid_Op2=true; break;//R[s1]-s2=R[r]         
	case 0x23:operation=MUL; break;//R[s1]xR[s2]=R[r] 
    case 0x13:operation=MULi,iqEntry.valid_Op2=true; break;//R[s1]xs2=R[r] 
	case 0x24:operation=DIV; break;//R[s1]/R[s2]=R[r]
    case 0x14:operation=DIVi,iqEntry.valid_Op2=true; break;//R[s1]/s2=R[r]
	case 0x25:operation=AND; break;//R[s1]&&R[s2]=R[r]
	case 0x26:operation=OR; break;//R[s1]||R[s2]=R[r]
	case 0x27:operation=XOR; break;//R[s1]^R[s2]=R[r]
	case 0x28:operation=NOT;             break; //Invert by position store to r
	case 0x29:operation=MOV;            break;// move from R[S1] to R[S2]
    case 0x19:operation=MOVi,iqEntry.valid_Op2=true;            break;// move from R[RF[S1]] to R[RF[S2]]
	case 0x39:operation=LOAD; break;//IF !s2==0, Loadi S1 to r 
	case 0x3a:operation=STORE; break;// STORE from R[S1]+s2 to M[r]
	case 0x3b:operation=JMP,  r=op1; break;                     //UNCONDITIONAL_JUMP jump to s1
    case 0x3c:operation=JZL,  r=op3;break;//CONDITIONAL_JUMP if S1>S2,          jump to r
    case 0x2c:operation=JZLi,iqEntry.valid_Op2=true, r=op3;break;//CONDITIONAL_JUMP if RF[S1]>RF[S2],  jump to r
	case 0x3d:operation=JZC,  r=op3;break; //BRANCH_ZERO iF R[op1]-R[op2]=0,    jump to r 
	case 0x1d:operation=JZCi,iqEntry.valid_Op2=true, r=op3;break; //BRANCH_ZERO_direct iF R[op1]-op2=0,jump to r 
    case 0x3e:operation=JNZ,  r=op3;break; //BRANCH_NOT_ZERO: if R[S1]-R[S2]!=0 ，jump to r
    case 0x3f:operation=HALT;     break;
    default:
    operation=NOP;printf("Unknown opcode: %d\n", opcode);
    break;
	}
    
    
    printf("Decoded Instraction: \033[34m%s\033[0m %x %x %x\n", OpcodeNames[operation],op1,op2,op3);
    iqEntry.Op1 = op1;
    iqEntry.Op2 = op2;
    iqEntry.destination = op3;
    iqEntry.isSpeculate = fetched.is_speculative;
    iqEntry.PC_from = fetched.pc_from;
    robEntry.Operand1 = op1;
    robEntry.Operand2 = op2;
    robEntry.isSpeculate = fetched.is_speculative;
    robEntry.PC_from = fetched.pc_from;

// 将IQEntry和RoBEntry加入到相应的队列中
    DispatchEnqueueIQ(Isq, iqEntry);
    DispatchEnqueueROB(rob, robEntry);
}
