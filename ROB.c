#include "ROB.h"
#include <stdio.h>
#include <stdlib.h>

ReorderBuffer* initiateRoB() {
    ReorderBuffer* rob = (ReorderBuffer*)malloc(sizeof(ReorderBuffer));
    rob->RoB = (RoBEntry*)malloc(ROB_SIZE * sizeof(RoBEntry));
    rob->size = ROB_SIZE;
    rob->front = 0;
    rob->rear = 0;
    return rob;
}

void DispatchEnqueueROB(ReorderBuffer* rob, RoBEntry entry) {
    rob->RoB[rob->rear] = entry;
    rob->rear = (rob->rear + 1) % rob->size;
}

RoBEntry CommitDequeueROB(ReorderBuffer* rob) {
    RoBEntry RoBitem = rob->RoB[rob->front];
    rob->front = (rob->front + 1) % rob->size;
    return RoBitem;
}

bool updateRoB(ReorderBuffer* rob, uint8_t PC_from, RoBEntry newEntry) {
    int current = rob->front;
    while (current != rob->rear) {
        if (rob->RoB[current].PC_from == PC_from) {
            rob->RoB[current] = newEntry;  // 更新条目
            return true;  // 成功更新
        }
        current = (current + 1) % rob->size;
    }
    return false;  // 没有找到匹配的PC_from
}
