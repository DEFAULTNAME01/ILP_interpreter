#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "function.h"
#include <stdlib.h>
#include <time.h>


int32_t MEM[1024][4];//the memory 4 channel 

Cache INSTR[512];
int32_t RF[32];
void printINSTR(Cache INSTR[], int size) {
    int per_line = 8; // 每行的元素数量
    for (int i = 0; i < size; i += per_line) {
        // print the header for each line
        printf("INSTR[%03d:%03d]: ", i, i + per_line - 1 < size ? i + per_line - 1 : size - 1);

        // print the elements for each line
        for (int j = i; j < i + per_line && j < size; j++) {
            printf("%08X ", INSTR[j].INST);
           // printf("%03X ", INSTR[j].MeM_addr);
        }
        printf("\n"); // 每打印完一行，就换行
    }
}

void printRF(int RF[], int size) {
    int per_line = 8; // 每行的元素数量
     printf("\n"); 
    for (int i = 0; i < size; i += per_line) {
        // print the header for each line
        printf("RF[%3d:%3d]: ", i, i + per_line - 1 < size ? i + per_line - 1 : size - 1);

        // print the elements for each line
        for (int j = i; j < i + per_line && j < size; j++) {
            printf(" %9d ", RF[j]);
        }
        printf("\n"); // 每打印完一行，就换行
    }
}
int main(int argc, char *argv[]){
    // Variables initial
    init_memory();
    init_globals();
   
    int executed_instructions = 0; // 3.1 the number of instructions executed
    int total_cycles = 0;  //3.2 the number of cycles taken for the whole run
    double per_cycles =0;
    bool continuous_run = false;
    int ch;
    bool is_busy = false;
    DecodedInstruction decoded;
    uint32_t opcode, op1, op2, op3;
    bool finished = false;
    int cycles = 0;
    int instructions = 0;
    int PC = 0;//where the program start the initial address
    int next_PC,error =0;
    int32_t fetched=0;
    IssueQueue Isq;
    if (argc < 2) {
        printf("Usage: %s <instructions_file>\n", argv[0]);
        return 1;
    }
    // Load instructions from the file
    if (load_program(argv[1], MEM, 1024) != 0) {
        printf("Error: Failed to load program\n");
        return 1;
    }

    // // Call fetch for all instructions in MEM
    for (int i = 0; i < 1024; i++) {
        if (MEM[i][0] == 0 && MEM[i][1] == 0 && MEM[i][2] == 0 && MEM[i][3] == 0) {
            break; // Stop calling Ld_cache when an empty instruction is encountered
        }
        Ld_cache(MEM, INSTR, i);
    }

    // //Print INSTR array contents 打印整个缓存
    // for (int i = 0; i < 512; i++) {
    //     printf("%02X ", INSTR[i].INST);
    //     printf("%02X\n ", INSTR[i].MeM_addr);
    // }

printINSTR(INSTR, 512);
    


    // Outer loop for handling user input
   
    // Wait for user input
     ch = getchar();
    if (ch == 'p'||ch == 'P') {
        continuous_run = true;
    } else if (ch == 'L'||ch == 'l') {
        continuous_run = false;
    }

    // 在程序开头记录开始时间
    time_t start_time = clock();
     // Main loop
    while (!finished ) {
        //
        printf("============================================\n");
        printf("\n");
        printf("============================================\n");
        //pipelines
        fetch(INSTR[PC], &is_busy, PC, &fetched);
        total_cycles++;
        decode(fetched, &Isq);
        total_cycles++;
        Execute(Isq, RF, MEM, PC, &next_PC, &finished, &error);
        printRF(RF, 32);
        executed_instructions++;
        total_cycles++;
        if (error == 1) {
            // Handle the error, for example, print an error message and exit the program
            printf("An error occurred during instruction execution.");
            finished = 1;
        }

        // refresh PC register state machine 更新程序计数器（PC）寄存器
        PC = next_PC;
      
        

        if (ch == 'L'||ch == 'l') {//Breakpoint running
            if (getchar()=='k'){//exit 
           
            break;
            }
        }else if (ch == 'p'||ch == 'P') {//Continuous operation
        continue;
        } 
    
    }
    // 程序结束时记录结束时间
    time_t end_time = clock();
    // 计算按下'p'到程序结束的时间差
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000.0;
    // Display results
    printf("\n\033[46mRegister file value in decimal.\033[0m\n");
    printf("running_times :%.4f ms\n",elapsed_time);
    //printf("Calculate result : %02X\n", RF[03]); 
    printf("total_cycles :\033[95m %d\033[0m\n",total_cycles);
    printf("Executed_instructions :\033[95m %d\033[0m\n",executed_instructions);
    per_cycles =(float)executed_instructions/total_cycles;
    printf("instructions per cycles : \033[95m%f\033[0m\n",per_cycles);
    write_memory_to_file("output.txt", MEM, 1024);
    printf("\n\033[46mOutput.txt running result saved.\033[0m\n");
    // Free memory
    free(rename_table);
    free(regs);
    return 0;
}
