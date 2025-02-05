// queue.h
#ifndef QUEUE_H
#define QUEUE_H

#include 

typedef struct Vehicle {
    int id;
    char road;
    int lane;
    SDL_Rect rect;
    int speed;
    int direction;  // 0: down, 1: right, 2: up, 3: left
} Vehicle;

typedef struct {
    char road;
    int lane;
    int priority;
} LanePriority;

typedef struct {
    LanePriority* data;
    int size;
    int capacity;
} PriorityQueue;

void initPriorityQueue(PriorityQueue* pq, int maxSize);
void enqueuePriority(PriorityQueue* pq, LanePriority item);
LanePriority dequeuePriority(PriorityQueue* pq);
bool isEmptyPriority(PriorityQueue* pq);
void updatePriority(PriorityQueue* pq, char road, int lane, int newPriority);
void generateVehicle(PriorityQueue* pq, Vehicle vehicles[], int* lastVehicleId);
void updateVehicles(PriorityQueue* pq, Vehicle vehicles[]);

#endif // QUEUE_H