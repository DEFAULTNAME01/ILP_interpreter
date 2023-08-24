// IssueQueue.c

#include "IssueQueue.h"
#include <stdio.h>
#include <stdlib.h>

IssueQueue* initiateIQ() {
    IssueQueue* iq = (IssueQueue*)malloc(sizeof(IssueQueue));
    iq->IQ = (IQEntry*)malloc(IQ_SIZE * sizeof(IQEntry));
    iq->size = IQ_SIZE;
    iq->front = 0;
    iq->rear = 0;
    return iq;
}

void DispatchEnqueueIQ(IssueQueue* iq, IQEntry entry) {
    iq->IQ[iq->rear] = entry;
    iq->rear = (iq->rear + 1) % iq->size;
}

IQEntry exe_DequeueIQ(IssueQueue* iq) {
    IQEntry Iqitem = iq->IQ[iq->front];
    iq->front = (iq->front + 1) % iq->size;
    return Iqitem;
}

bool updateIQ(IssueQueue* iq, uint8_t PC_from, IQEntry newEntry) {
    int current = iq->front;
    while (current != iq->rear) {
        if (iq->IQ[current].PC_from == PC_from) {
            iq->IQ[current] = newEntry;  // 更新条目
            return true;  // 成功更新
        }
        current = (current + 1) % iq->size;
    }
    return false;  // 没有找到匹配的PC_from
}

void broadcastResultToIQ(IssueQueue* iq, uint8_t tag, uint8_t result) {
    for (int i = 0; i < iq->size; i++) {
        // Check if Op1 is waiting for this result
        if (!iq->IQ[i].valid_Op1 && iq->IQ[i].tag_Op1 == tag) {
            iq->IQ[i].Op1 = result;
            iq->IQ[i].valid_Op1 = true;
        }
        // Check if Op2 is waiting for this result
        if (!iq->IQ[i].valid_Op2 && iq->IQ[i].tag_Op2 == tag) {
            iq->IQ[i].Op2 = result;
            iq->IQ[i].valid_Op2 = true;
        }
    }
}
