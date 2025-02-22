#include "queue.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define VEHICLE_SPAWN_INTERVAL 2000 // 2 seconds

void generateVehicle(PriorityQueue *pq, Vehicle vehicles[], int *lastVehicleId) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastSpawnTime < VEHICLE_SPAWN_INTERVAL) return;

    int freeSlot = -1;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) {
            freeSlot = i;
            break;
        }
    }
    if (freeSlot == -1) return;

    Vehicle *v = &vehicles[freeSlot];
    *lastVehicleId = *lastVehicleId + 1;
    v->id = *lastVehicleId;
    v->speed = VEHICLE_SPEED;

    char roads[] = {'A', 'B', 'C', 'D'};
    int directions[] = {0, 1, 2, 3}; // Down, Right, Up, Left
    v->road = roads[rand() % 4];
    v->direction = directions[rand() % 4];
    v->lane = (rand() % 3) + 1;

    // Set initial position based on direction
    switch (v->direction) {
        case 0: // Down
            v->x = ROAD_X_START + (v->lane * 100);
            v->y = 0;
            break;
        case 1: // Right
            v->x = 0;
            v->y = ROAD_Y_START + (v->lane * 100);
            break;
        case 2: // Up
            v->x = ROAD_X_START + (v->lane * 100);
            v->y = SCREEN_HEIGHT;
            break;
        case 3: // Left
            v->x = SCREEN_WIDTH;
            v->y = ROAD_Y_START + (v->lane * 100);
            break;
    }

    // Initialize the vehicle's rectangle
    v->rect = (SDL_Rect){(int)v->x, (int)v->y, VEHICLE_WIDTH, VEHICLE_HEIGHT};

    // Add vehicle to the priority queue
    enqueuePriority(pq, (LanePriority){v->road, v->lane, 0});

    printf("Added vehicle: ID=%d, Road=%c, Lane=%d, Direction=%d\n", v->id, v->road, v->lane, v->direction); // Debug print

    lastSpawnTime = currentTime;
}