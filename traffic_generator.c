#include "queue.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define VEHICLE_SPAWN_INTERVAL 2000 // 2 seconds
#define MIN_VEHICLE_SPACING 100     // Minimum spacing between vehicles

extern Uint32 lastSpawnTime;

// Helper function to calculate distance between two vehicles
float distanceBetweenVehicles(Vehicle *v1, Vehicle *v2) {
    return sqrt(pow(v1->x - v2->x, 2) + pow(v1->y - v2->y, 2));
}

void generateVehicle(PriorityQueue *pq, Vehicle vehicles[], int *lastVehicleId) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastSpawnTime < VEHICLE_SPAWN_INTERVAL)
        return;

    // Find a free slot in the vehicles array
    int freeSlot = -1;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) {
            freeSlot = i;
            break;
        }
    }
    if (freeSlot == -1)
        return;

    Vehicle *v = &vehicles[freeSlot];
    (*lastVehicleId)++;
    v->id = *lastVehicleId;
    v->speed = VEHICLE_SPEED;
    v->isPriority = false;
    v->arrivalTime = currentTime; // Record the arrival time

    // Randomly decide if the vehicle will take a left turn (30% chance)
    bool willTurnLeft = shouldRedirect();
    v->turningLeft = willTurnLeft;

    // Choose a road randomly from A, B, C, D
    char roads[] = {'A', 'B', 'C', 'D'};
    v->road = roads[rand() % 4];

    // Randomly assign a lane from 1 to 4 (L1–L4)
    int laneIndex = rand() % 4;
    v->lane = laneIndex + 1;  // Use 1-indexed lanes

    // Determine the initial position based on the road
    int laneWidth, laneCenterOffset;
    switch (v->road) {
        case 'A': // Horizontal left-to-right
            v->direction = 1;  // Right
            laneWidth = ROAD_HEIGHT / 4;
            laneCenterOffset = laneWidth / 2;
            v->x = 0; // Start from the left edge
            v->y = ROAD_Y_START + (laneIndex * laneWidth) + laneCenterOffset - (VEHICLE_HEIGHT / 2);
            break;
        case 'B': // Vertical top-to-bottom
            v->direction = 0;  // Down
            laneWidth = ROAD_WIDTH / 4;
            laneCenterOffset = laneWidth / 2;
            v->x = ROAD_X_START + (laneIndex * laneWidth) + laneCenterOffset - (VEHICLE_WIDTH / 2);
            v->y = 0; // Start from the top edge
            break;
        case 'C': // Horizontal right-to-left
            v->direction = 3;  // Left
            laneWidth = ROAD_HEIGHT / 4;
            laneCenterOffset = laneWidth / 2;
            v->x = SCREEN_WIDTH - VEHICLE_WIDTH; // Start from the right edge
            v->y = ROAD_Y_START + (laneIndex * laneWidth) + laneCenterOffset - (VEHICLE_HEIGHT / 2);
            break;
        case 'D': // Vertical bottom-to-top
            v->direction = 2;  // Up
            laneWidth = ROAD_WIDTH / 4;
            laneCenterOffset = laneWidth / 2;
            v->x = ROAD_X_START + (laneIndex * laneWidth) + laneCenterOffset - (VEHICLE_WIDTH / 2);
            v->y = SCREEN_HEIGHT - VEHICLE_HEIGHT; // Start from the bottom edge
            break;
    }

    // Check spacing with existing vehicles on the same road and lane
    bool tooClose = false;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (i == freeSlot || vehicles[i].id == -1 ||
            vehicles[i].road != v->road || vehicles[i].lane != v->lane)
            continue;
        if (distanceBetweenVehicles(v, &vehicles[i]) < MIN_VEHICLE_SPACING) {
            tooClose = true;
            break;
        }
    }
    if (tooClose) {
        v->id = -1; // Cancel spawn if vehicles are too close
        return;
    }

    // Initialize the vehicle's rectangle for rendering
    v->rect = (SDL_Rect){ (int)v->x, (int)v->y, VEHICLE_WIDTH, VEHICLE_HEIGHT };

    // Enqueue a lane priority record (initial priority is 0)
    enqueuePriority(pq, (LanePriority){ v->road, v->lane, 0 });

    // Write vehicle data to the appropriate lane file (e.g., "laneA.txt" for road A)
    char filename[20];
    switch (v->road) {
        case 'A': sprintf(filename, "laneA.txt"); break;
        case 'B': sprintf(filename, "laneB.txt"); break;
        case 'C': sprintf(filename, "laneC.txt"); break;
        case 'D': sprintf(filename, "laneD.txt"); break;
    }
    FILE *fp = fopen(filename, "a");
    if (fp) {
        // Format current time as HH:MM:SS
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timeStr[9];
        strftime(timeStr, 9, "%H:%M:%S", tm_info);

        char laneId[10];
        sprintf(laneId, "%cL%d", v->road, v->lane);
        fprintf(fp, "V%d,%s,%s,%s\n", v->id, timeStr, laneId, (willTurnLeft ? "left" : "straight"));
        fclose(fp);
    }

    printf("Generated vehicle: ID=%d, Road=%c, Lane=%d, Direction=%s, X=%d, Y=%d\n", 
           v->id, v->road, v->lane, (willTurnLeft ? "left" : "straight"), (int)v->x, (int)v->y);
    lastSpawnTime = currentTime;
}

// Decide if a vehicle should redirect (simulate left-turn) at the intersection
bool shouldRedirect() {
    return (rand() % 100) < 30;
}

// Check if the vehicle is within the intersection bounds
bool isAtIntersection(Vehicle *v) {
    return (v->x >= ROAD_X_START - VEHICLE_WIDTH && 
            v->x <= ROAD_X_START + ROAD_WIDTH && 
            v->y >= ROAD_Y_START - VEHICLE_HEIGHT && 
            v->y <= ROAD_Y_START + ROAD_HEIGHT);
}

// Redirect a vehicle smoothly at the intersection (if the light is green)
void redirectVehicle(Vehicle *v) {
    if (!isAtIntersection(v) || isLightRedForVehicle(v)) {
        return;
    }
    
    // For a left-turn, assign a free lane (e.g., lane 3) for smoother turning
    int newLane = (v->turningLeft) ? 3 : v->lane;
    if (v->lane != newLane) {
        v->lane = newLane;
    }
    
    printf("Redirected vehicle at intersection: ID=%d, Road=%c, New Lane=%d, X=%d, Y=%d\n", 
           v->id, v->road, v->lane, (int)v->x, (int)v->y);
}
