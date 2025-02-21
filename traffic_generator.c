#include "queue.h"
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MAX_ACTIVE_VEHICLES 8 // Increased slightly to populate all roads
#define MAX_LANES 12
#define VEHICLE_SPAWN_INTERVAL 2000 // 2 seconds between spawns for more frequent population

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
    static bool firstSpawn = true;
    static int spawnCounter = 0; // To cycle through roads
    if (!firstSpawn && (currentTime - lastSpawnTime < VEHICLE_SPAWN_INTERVAL)) {
        printf("Too soon to spawn: %u - %u < %d\n", currentTime, lastSpawnTime, VEHICLE_SPAWN_INTERVAL);
        return;
    }

    int activeVehicles = 0;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id >= 0) {
            activeVehicles++;
        }
    }
    if (activeVehicles >= MAX_ACTIVE_VEHICLES) {
        printf("Max active vehicles reached: %d\n", activeVehicles);
        return;
    }

    lastSpawnTime = currentTime;
    firstSpawn = false;

    int freeSlot = -1;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) {
            freeSlot = i;
            break;
        }
    }
    if (freeSlot == -1) {
        printf("No free slots available\n");
        return;
    }

    Vehicle *v = &vehicles[freeSlot];
    *lastVehicleId = *lastVehicleId + 1;
    v->id = *lastVehicleId;
    v->speed = 50.0f / DESIRED_FPS; // 50 pixels/sec

    // Cycle through roads A, B, C, D
    char roads[] = {'A', 'B', 'C', 'D'};
    int directions[] = {0, 1, 2, 3}; // Down, Right, Up, Left
    v->road = roads[spawnCounter % 4];
    v->direction = directions[spawnCounter % 4];
    v->lane = (spawnCounter % 3) + 1; // Cycle through lanes 1, 2, 3
    spawnCounter++;

    int laneWidth = 400 / 3; // Width of each lane
    int laneHeight = 400 / 3; // Height of each lane

    // Calculate the center of the lane for proper alignment
    switch (v->direction) {
    case 0: // Down (Road A)
        v->x = ROAD_X_START + (v->lane - 1) * laneWidth + (laneWidth - VEHICLE_WIDTH) / 2;
        v->y = -VEHICLE_HEIGHT;
        break;
    case 1: // Right (Road B)
        v->x = -VEHICLE_WIDTH;
        v->y = ROAD_Y_START + (v->lane - 1) * laneHeight + (laneHeight - VEHICLE_HEIGHT) / 2;
        break;
    case 2: // Up (Road C)
        v->x = ROAD_X_START + (v->lane - 1) * laneWidth + (laneWidth - VEHICLE_WIDTH) / 2;
        v->y = SCREEN_HEIGHT;
        break;
    case 3: // Left (Road D)
        v->x = SCREEN_WIDTH;
        v->y = ROAD_Y_START + (v->lane - 1) * laneHeight + (laneHeight - VEHICLE_HEIGHT) / 2;
        break;
    }

    v->rect = (SDL_Rect){(int)v->x, (int)v->y, VEHICLE_WIDTH, VEHICLE_HEIGHT};
    printf("Spawned vehicle %d: road=%c, lane=%d, dir=%d, pos=(%f, %f), speed=%f\n",
           v->id, v->road, v->lane, v->direction, v->x, v->y, v->speed);
    logVehicleToFile(v);
}
void updateVehicles(PriorityQueue *pq, Vehicle vehicles[]) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        Vehicle *v = &vehicles[i];
        if (v->id < 0)
            continue;

        float maxMovePerFrame = 50.0f / DESIRED_FPS;
        float move = fminf(v->speed, maxMovePerFrame);
        printf("Updating vehicle %d: speed=%f, move=%f, pos=(%f, %f)\n",
               v->id, v->speed, move, v->x, v->y);

        switch (v->direction) {
        case 0: // Down
            v->y += move;
            if (v->y > SCREEN_HEIGHT) {
                v->id = -1; // Remove vehicle if it goes off-screen
                logVehicleToFile(v);
            }
            break;
        case 1: // Right
            v->x += move;
            if (v->x > SCREEN_WIDTH) {
                v->id = -1; // Remove vehicle if it goes off-screen
                logVehicleToFile(v);
            }
            break;
        case 2: // Up
            v->y -= move;
            if (v->y + VEHICLE_HEIGHT < 0) {
                v->id = -1; // Remove vehicle if it goes off-screen
                logVehicleToFile(v);
            }
            break;
        case 3: // Left
            v->x -= move;
            if (v->x + VEHICLE_WIDTH < 0) {
                v->id = -1; // Remove vehicle if it goes off-screen
                logVehicleToFile(v);
            }
            break;
        }

        // Update the vehicle's rectangle for rendering
        v->rect.x = (int)v->x;
        v->rect.y = (int)v->y;

        if (v->id != -1) {
            logVehicleToFile(v);
        }
    }
}