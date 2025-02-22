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

// Update vehicle positions
void updateVehicles(PriorityQueue *pq, Vehicle vehicles[]) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) continue;

        switch (vehicles[i].direction) {
        case 0: // Down
            vehicles[i].y += vehicles[i].speed;
            if (vehicles[i].y > SCREEN_HEIGHT) vehicles[i].id = -1;
            break;
        case 1: // Right
            vehicles[i].x += vehicles[i].speed;
            if (vehicles[i].x > SCREEN_WIDTH) vehicles[i].id = -1;
            break;
        case 2: // Up
            vehicles[i].y -= vehicles[i].speed;
            if (vehicles[i].y + VEHICLE_HEIGHT < 0) vehicles[i].id = -1;
            break;
        case 3: // Left
            vehicles[i].x -= vehicles[i].speed;
            if (vehicles[i].x + VEHICLE_WIDTH < 0) vehicles[i].id = -1;
            break;
        }
    }
}