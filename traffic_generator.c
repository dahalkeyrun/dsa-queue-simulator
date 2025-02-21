#include "queue.h"
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MAX_VEHICLES 200
#define MAX_ACTIVE_VEHICLES 50
#define MAX_LANES 12
#define VEHICLE_SPAWN_INTERVAL 3000 // 3 seconds between spawns
#define SCREEN_WIDTH 2000
#define SCREEN_HEIGHT 1700
#define VEHICLE_WIDTH 50
#define VEHICLE_HEIGHT 100
#define VEHICLE_LOG "vehicles.log"

//extern Uint32 lastSpawnTime = 0;

void logVehicleToFile(Vehicle *v) {
    FILE *logFile = fopen(VEHICLE_LOG, "a");
    if (logFile == NULL) {
        printf("Failed to open log file for writing!\n");
        return;
    }
    fprintf(logFile, "%d:%c:%d:%d:%f:%f:%d\n", v->id, v->road, v->lane, v->direction, v->x, v->y, (int)(v->speed * DESIRED_FPS));
    fclose(logFile);
}

void initPriorityQueue(PriorityQueue *pq, int maxSize) {
    pq->data = (LanePriority *)malloc(sizeof(LanePriority) * maxSize);
    pq->size = 0;
    pq->capacity = maxSize;
}

void heapifyUp(PriorityQueue *pq, int index) {
    int parent = (index - 1) / 2;
    while (index > 0 && pq->data[parent].priority > pq->data[index].priority) {
        LanePriority temp = pq->data[parent];
        pq->data[parent] = pq->data[index];
        pq->data[index] = temp;
        index = parent;
        parent = (index - 1) / 2;
    }
}

void heapifyDown(PriorityQueue *pq, int index) {
    int smallest = index;
    int leftChild = 2 * index + 1;
    int rightChild = 2 * index + 2;

    if (leftChild < pq->size && pq->data[leftChild].priority < pq->data[smallest].priority)
        smallest = leftChild;

    if (rightChild < pq->size && pq->data[rightChild].priority < pq->data[smallest].priority)
        smallest = rightChild;

    if (smallest != index) {
        LanePriority temp = pq->data[index];
        pq->data[index] = pq->data[smallest];
        pq->data[smallest] = temp;
        heapifyDown(pq, smallest);
    }
}

void enqueuePriority(PriorityQueue *pq, LanePriority item) {
    if (pq->size == pq->capacity)
        return;
    pq->data[pq->size] = item;
    heapifyUp(pq, pq->size);
    pq->size++;
}

LanePriority dequeuePriority(PriorityQueue *pq) {
    if (pq->size == 0) {
        LanePriority empty = {'X', -1, -1};
        return empty;
    }
    LanePriority result = pq->data[0];
    pq->data[0] = pq->data[pq->size - 1];
    pq->size--;
    heapifyDown(pq, 0);
    return result;
}

bool isEmptyPriority(PriorityQueue *pq) {
    return pq->size == 0;
}

void updatePriority(PriorityQueue *pq, char road, int lane, int newPriority) {
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i].road == road && pq->data[i].lane == lane) {
            pq->data[i].priority = newPriority;
            heapifyUp(pq, i);
            break;
        }
    }
}

void generateVehicle(PriorityQueue *pq, Vehicle vehicles[], int *lastVehicleId) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastSpawnTime < VEHICLE_SPAWN_INTERVAL)
        return;

    // Count active vehicles
    int activeVehicles = 0;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id >= 0) {
            activeVehicles++;
        }
    }

    // Only generate a new vehicle if we have less than MAX_ACTIVE_VEHICLES
    if (activeVehicles >= MAX_ACTIVE_VEHICLES) {
        return;
    }

    lastSpawnTime = currentTime;

    int freeSlot = -1;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) {
            freeSlot = i;
            break;
        }
    }

    if (freeSlot == -1)
        return; // No free slots

    Vehicle *v = &vehicles[freeSlot];
    *lastVehicleId = *lastVehicleId + 1;
    v->id = *lastVehicleId;
    v->speed = (float)(rand() % 3 + 1) / DESIRED_FPS; // Speed between 1 and 3 units per second, adjusted for FPS
    v->direction = rand() % 4; 
    v->road = 'A' + (rand() % 4);
    v->lane = rand() % 3 + 1; 

    int laneWidth = 400 / 3;
    int laneHeight = 400 / 3;

    switch (v->direction) {
    case 0: // Down
        v->x = ROAD_X_START + (v->lane - 1) * laneWidth + (laneWidth - VEHICLE_WIDTH) / 2;
        v->y = -VEHICLE_HEIGHT;
        break;
    case 1: // Right
        v->x = -VEHICLE_HEIGHT;
        v->y = ROAD_Y_START + (v->lane - 1) * laneHeight + (laneHeight - VEHICLE_WIDTH) / 2;
        break;
    case 2: // Up
        v->x = ROAD_X_START + (v->lane - 1) * laneWidth + (laneWidth - VEHICLE_WIDTH) / 2;
        v->y = SCREEN_HEIGHT;
        break;
    case 3: // Left
        v->x = SCREEN_WIDTH;
        v->y = ROAD_Y_START + (v->lane - 1) * laneHeight + (laneHeight - VEHICLE_WIDTH) / 2;
        break;
    }

    logVehicleToFile(v);
}

void updateVehicles(PriorityQueue *pq, Vehicle vehicles[]) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        Vehicle *v = &vehicles[i];
        if (v->id < 0)
            continue;

        float maxMovePerFrame = 10.0f / DESIRED_FPS; // Adjust for frame rate
        float move = fminf(v->speed, maxMovePerFrame);

        switch (v->direction) {
        case 0: // Down
            v->y += move;
            if (v->y > SCREEN_HEIGHT) {
                v->id = -1; // Mark as inactive
                logVehicleToFile(v); // Log the vehicle's exit
            }
            break;
        case 1: // Right
            v->x += move;
            if (v->x > SCREEN_WIDTH) {
                v->id = -1;
                logVehicleToFile(v);
            }
            break;
        case 2: // Up
            v->y -= move;
            if (v->y + VEHICLE_HEIGHT < 0) {
                v->id = -1;
                logVehicleToFile(v);
            }
            break;
        case 3: // Left
            v->x -= move;
            if (v->x + VEHICLE_HEIGHT < 0) {
                v->id = -1;
                logVehicleToFile(v);
            }
            break;
        }

        // Simple collision avoidance
        for (int j = 0; j < MAX_VEHICLES; j++) {
            if (i != j && vehicles[j].id >= 0) {
                float dx = v->x - vehicles[j].x;
                float dy = v->y - vehicles[j].y;
                float dist = sqrt(dx*dx + dy*dy);
                if (dist < VEHICLE_WIDTH * 2) { // Arbitrary close distance, considering vehicle size
                    // Slow down if too close
                    v->speed *= 0.9f;
                }
            }
        }

        // Log the vehicle's current state after each update
        if (v->id != -1) {
            logVehicleToFile(v);
        }
    }
}