#include "queue.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define VEHICLE_SPAWN_INTERVAL 2000 // 2 seconds
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define VEHICLE_WIDTH 50
#define VEHICLE_HEIGHT 100
#define ROAD_WIDTH 400
#define ROAD_X_START 100
#define ROAD_Y_START 100
extern Uint32 lastSpawnTime;
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
    int lanes[] = {0, 1, 2}; // Change to 0-based indexing
    v->road = roads[rand() % 4];
    v->lane = lanes[rand() % 3];

    int laneWidth = ROAD_WIDTH / 3; // Each lane is ~133.33 pixels
    int laneCenterOffset = laneWidth / 2; // Center within the lane

    switch (v->road) {
    case 'A': // Left to right (horizontal)
        v->direction = 1;
        v->x = 0; // Start off-screen left
        v->y = ROAD_Y_START + (v->lane * laneWidth) + laneCenterOffset - (VEHICLE_HEIGHT / 2);
        break;
    case 'B': // Top to bottom (vertical)
        v->direction = 0;
        v->x = ROAD_X_START + (v->lane * laneWidth) + laneCenterOffset - (VEHICLE_WIDTH / 2);
        v->y = 0; // Start off-screen top
        break;
    case 'C': // Right to left (horizontal)
        v->direction = 3;
        v->x = SCREEN_WIDTH; // Start off-screen right
        v->y = ROAD_Y_START + (v->lane * laneWidth) + laneCenterOffset - (VEHICLE_HEIGHT / 2);
        break;
    case 'D': // Bottom to top (vertical)
        v->direction = 2;
        v->x = ROAD_X_START + (v->lane * laneWidth) + laneCenterOffset - (VEHICLE_WIDTH / 2);
        v->y = SCREEN_HEIGHT; // Start off-screen bottom
        break;
    }

    v->rect = (SDL_Rect){(int)v->x, (int)v->y, VEHICLE_WIDTH, VEHICLE_HEIGHT};
    enqueuePriority(pq, (LanePriority){v->road, v->lane + 1, 0}); // +1 to match 1,2,3 for priority

    printf("Added vehicle: ID=%d, Road=%c, Lane=%d, Direction=%d, X=%d, Y=%d\n", 
           v->id, v->road, v->lane, v->direction, (int)v->x, (int)v->y);
           lastSpawnTime = currentTime;
}