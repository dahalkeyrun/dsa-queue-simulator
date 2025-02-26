#include "queue.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define VEHICLE_SPAWN_INTERVAL 2000 // 2 seconds
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
    v->isPriority = false;

    // Decide road (A = horizontal, B = vertical) randomly
    char roads[] = {'A', 'B'};
    v->road = roads[rand() % 2];
    
    // All vehicles start in the center lane (lane 1)
    v->lane = 1;

    int laneWidth = ROAD_WIDTH / 3;
    int laneCenterOffset = laneWidth / 2;

    // Place vehicles at the edge of the screen in the center lane
    switch (v->road) {
        case 'A': // Horizontal road
            // Direction: left to right (1)
            v->direction = 1;
            v->x = 0; // Start from the left edge
            v->y = ROAD_Y_START + (v->lane * laneWidth) + laneCenterOffset - (VEHICLE_HEIGHT / 2);
            break;
        case 'B': // Vertical road
            // Direction: top to bottom (0)
            v->direction = 0;
            v->x = ROAD_X_START + (v->lane * laneWidth) + laneCenterOffset - (VEHICLE_WIDTH / 2);
            v->y = 0; // Start from the top edge
            break;
    }

    v->rect = (SDL_Rect){(int)v->x, (int)v->y, VEHICLE_WIDTH, VEHICLE_HEIGHT};
    enqueuePriority(pq, (LanePriority){v->road, v->lane, 0});

    printf("Generated vehicle: ID=%d, Road=%c, Lane=%d, Direction=%d, X=%d, Y=%d\n", 
           v->id, v->road, v->lane, v->direction, (int)v->x, (int)v->y);
    lastSpawnTime = currentTime;
}

// Decide if a vehicle should redirect to another lane when at intersection
bool shouldRedirect() {
    // 30% chance to redirect when at intersection
    return (rand() % 100) < 30;
}

// Function to check if vehicle is at the intersection
bool isAtIntersection(Vehicle *v) {
    return (v->x >= ROAD_X_START - VEHICLE_WIDTH && 
            v->x <= ROAD_X_START + ROAD_WIDTH && 
            v->y >= ROAD_Y_START - VEHICLE_HEIGHT && 
            v->y <= ROAD_Y_START + ROAD_WIDTH);
}

// Redirect a vehicle to another lane at the intersection
void redirectVehicle(Vehicle *v) {
    // Only redirect if the vehicle is at the intersection
    if (!isAtIntersection(v)) {
        return;
    }
    
    // Only redirect if traffic light is green
    if (isLightRedForVehicle(v)) {
        return;
    }
    
    int laneWidth = ROAD_WIDTH / 3;
    int laneCenterOffset = laneWidth / 2;
    
    // Choose a new lane (0 or 2, not current lane 1)
    int newLane = (rand() % 2) == 0 ? 0 : 2;
    v->lane = newLane;
    
    // Adjust position based on new lane
    switch (v->direction) {
        case 0: // Down
            v->x = ROAD_X_START + (v->lane * laneWidth) + laneCenterOffset - (VEHICLE_WIDTH / 2);
            break;
        case 1: // Right
            v->y = ROAD_Y_START + (v->lane * laneWidth) + laneCenterOffset - (VEHICLE_HEIGHT / 2);
            break;
    }
    
    printf("Redirected vehicle at intersection: ID=%d, Road=%c, New Lane=%d, X=%d, Y=%d\n", 
           v->id, v->road, v->lane, (int)v->x, (int)v->y);
}