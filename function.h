#ifndef FUNCTION_H
#define FUNCTION_H
// LRU Strategy_self refreshable Cache，包含一个 int32_t cache_instruction_data变量和uint16_t Absolute memory address变量和Cache Addressing Count变量


extern int NUM_RENAME_REGS;
extern int NUM_REGS;




#define NUM_RENAME_REGS 64
#define NUM_REGS 128

extern int32_t MEM[1024][4];//the memory 4 channel 
typedef struct {
  int32_t INST;
  uint16_t MeM_addr;
  uint16_t j;
} Cache;


extern Cache INSTR[512];


extern int32_t RF[32];

typedef struct {
    uint32_t opcode;
    uint32_t op1;
    uint32_t op2;
    uint32_t op3;
} DecodedInstruction;

typedef struct {
    uint32_t rename_reg;    // 重命名寄存器编号
    uint32_t phys_reg;      // 物理寄存器编号
    uint32_t SRC_ins_addr;// 源指令地址
 //   uint32_t hash_addr;     // 哈希地址
} rename_table_entry;


extern rename_table_entry *rename_table;
extern int32_t *regs;

void init_memory();
void init_globals() ;
// void init_rename_table();
// int32_t rename_reg(uint32_t reg, uint32_t ins_addr);
// void free_phys_reg(int32_t phys_reg) ;

// int32_t update_rename_table(uint32_t reg, int32_t new_phys_reg, uint32_t ins_addr);

typedef enum {
    NOP,ADD,SUB,MUL,DIV,AND,OR,XOR,NOT,MOV,LOAD,STORE,JZC,JNZ,JZL,JMP, HALT,
    ADDi,SUBi,MULi,DIVi,JZCi,MOVi,JZLi,
} OpCode;

typedef struct {
    OpCode Opcode;
    int s1;//RF addr1
    int s2;//RF addr2
    int r;//result
}IssueQueue;



int load_program(const char *filename, int MEM[][4], int max_instr);

void Ld_cache(int32_t MEM[][4],Cache INSTR[], int P_m);
void fetch(Cache INSTR, bool *is_busy,int PC,int32_t *fetched);
void decode(int32_t fetched, IssueQueue *Isq);
void write_memory_to_file(const char* filename, int (*Mem)[4], uint32_t size);
void Execute(IssueQueue Isq, int *RF, int32_t (*MEM)[4], int PC,int *next_PC, _Bool *finished,int *error);



#endif
