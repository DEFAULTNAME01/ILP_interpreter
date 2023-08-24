#ifndef ROB_H
#define ROB_H

#include <stdbool.h>
#include <stdint.h>

#define ROB_SIZE 10  // 全局变量，设置ROB的大小

// RoBEntry结构定义
typedef struct {
    bool isSpeculate;
    uint8_t PC_from;
    uint8_t Operand1;
    uint8_t Operand2;
    uint32_t result;
    bool valid_result;
} RoBEntry;

// ReorderBuffer结构定义
typedef struct {
    RoBEntry *RoB;
    int size;
    int front;
    int rear;
} ReorderBuffer;

// 函数原型声明
ReorderBuffer* initiateRoB(void);
void DispatchEnqueueROB(ReorderBuffer* rob, RoBEntry entry);
RoBEntry CommitDequeueROB(ReorderBuffer* rob);
bool updateRoB(ReorderBuffer* rob, uint8_t PC_from, RoBEntry newEntry);

#endif // ROB_H
