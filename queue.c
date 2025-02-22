#include "queue.h"
#include <stdlib.h>
#include <math.h>

// Initialize a priority queue
void initPriorityQueue(PriorityQueue *pq, int maxSize) {
    pq->data = (LanePriority *)malloc(sizeof(LanePriority) * maxSize);
    pq->size = 0;
    pq->capacity = maxSize;
}

// Enqueue an item into the priority queue
void enqueuePriority(PriorityQueue *pq, LanePriority item) {
    if (pq->size == pq->capacity) return; // Queue is full
    pq->data[pq->size++] = item;
}

// Dequeue an item from the priority queue
LanePriority dequeuePriority(PriorityQueue *pq) {
    if (pq->size == 0) {
        LanePriority empty = {'X', -1, -1};
        return empty;
    }
    LanePriority item = pq->data[0];
    for (int i = 1; i < pq->size; i++) {
        pq->data[i - 1] = pq->data[i];
    }
    pq->size--;
    return item;
}

// Check if the priority queue is empty
bool isEmptyPriority(PriorityQueue *pq) {
    return pq->size == 0;
}

// Update the priority of a lane in the priority queue
void updatePriority(PriorityQueue *pq, char road, int lane, int newPriority) {
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i].road == road && pq->data[i].lane == lane) {
            pq->data[i].priority = newPriority;
            break;
        }
    }
}