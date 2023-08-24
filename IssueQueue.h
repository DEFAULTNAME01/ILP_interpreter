// IssueQueue.h

#ifndef ISSUEQUEUE_H
#define ISSUEQUEUE_H

#include <stdbool.h>
#include <stdint.h>

#define IQ_SIZE 4  // 全局变量，设置IssueQueue的大小

typedef struct {
    bool isSpeculate;
    uint8_t PC_from;
    uint8_t Op1;
    bool valid_Op1;
    uint8_t tag_Op1;
    uint8_t Op2;
    bool valid_Op2;
    uint8_t tag_Op2;
    uint8_t destination; // 当指令执行完成后，结果的目标寄存器
} IQEntry;

typedef struct {
    IQEntry *IQ;
    int size;
    int front;
    int rear;
} IssueQueue;

// Function prototypes
IssueQueue* initiateIQ();
void DispatchEnqueueIQ(IssueQueue* iq, IQEntry entry);
IQEntry exe_DequeueIQ(IssueQueue* iq);
bool updateIQ(IssueQueue* iq, uint8_t PC_from, IQEntry newEntry);
void broadcastResultToIQ(IssueQueue* iq, uint8_t tag, uint8_t result);

#endif // ISSUEQUEUE_H
