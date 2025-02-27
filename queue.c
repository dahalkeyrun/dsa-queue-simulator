#include "queue.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

void initPriorityQueue(PriorityQueue *pq, int maxSize) {
    pq->data = (LanePriority *)malloc(sizeof(LanePriority) * maxSize);
    pq->size = 0;
    pq->capacity = maxSize;
}

void enqueuePriority(PriorityQueue *pq, LanePriority item) {
    if (pq->size == pq->capacity)
        return;
    pq->data[pq->size++] = item;
}

LanePriority dequeuePriority(PriorityQueue *pq) {
    if (pq->size == 0) {
        // Return an invalid record if the queue is empty
        return (LanePriority){ 'X', -1, -1 };
    }
    
    // Find the highest priority lane
    int maxPriorityIndex = 0;
    for (int i = 1; i < pq->size; i++) {
        if (pq->data[i].priority > pq->data[maxPriorityIndex].priority) {
            maxPriorityIndex = i;
        }
    }
    
    LanePriority item = pq->data[maxPriorityIndex];
    // Remove the item by replacing it with the last element
    pq->data[maxPriorityIndex] = pq->data[pq->size - 1];
    pq->size--;
    
    return item;
}

bool isEmptyPriority(PriorityQueue *pq) {
    return pq->size == 0;
}

void updatePriority(PriorityQueue *pq, char road, int lane, int newPriority) {
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i].road == road && pq->data[i].lane == lane) {
            pq->data[i].priority = newPriority;
            return;
        }
    }
}

int countWaitingVehicles(Vehicle vehicles[], char road) {
    int count = 0;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id != -1 && vehicles[i].road == road && vehicles[i].speed == 0)
            count++;
    }
    return count;
}

// New helper: count waiting vehicles for a specific road and lane
int countWaitingVehiclesLane(Vehicle vehicles[], char road, int lane) {
    int count = 0;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id != -1 && vehicles[i].road == road && vehicles[i].lane == lane && vehicles[i].speed == 0)
            count++;
    }
    return count;
}

void handlePriorityRoads(PriorityQueue *pq, Vehicle vehicles[]) {
    // For Road A, specifically lane 2 (AL2) has priority conditions
    int waitingAL2 = countWaitingVehiclesLane(vehicles, 'A', 2);
    
    // Thresholds for priority adjustments (these can be tuned)
    const int PRIORITY_THRESHOLD = 10;
    const int NORMAL_THRESHOLD = 5;
    
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i].road == 'A' && pq->data[i].lane == 2) {
            if (waitingAL2 > PRIORITY_THRESHOLD)
                pq->data[i].priority = 10; // High priority
            else if (waitingAL2 < NORMAL_THRESHOLD)
                pq->data[i].priority = 1;  // Normal priority
        } else {
            pq->data[i].priority = 1;
        }
    }
}

void updateVehicles(Vehicle vehicles[]) {
    // Update vehicle positions based on their direction and lane
    // Assuming 4 lanes per road; horizontal roads (A & C) use ROAD_Y_START,
    // vertical roads (B & D) use ROAD_X_START.
    int hLaneWidth = ROAD_HEIGHT / 4; // For horizontal roads (A, C)
    int vLaneWidth = ROAD_WIDTH / 4;  // For vertical roads (B, D)
    
    // Precompute lane centers for 4 lanes (0-indexed)
    int horizontalLaneCenters[4];
    int verticalLaneCenters[4];
    for (int i = 0; i < 4; i++) {
        horizontalLaneCenters[i] = ROAD_Y_START + (i * hLaneWidth) + (hLaneWidth / 2);
        verticalLaneCenters[i] = ROAD_X_START + (i * vLaneWidth) + (vLaneWidth / 2);
    }
    
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1)
            continue;
        
        Vehicle *v = &vehicles[i];
        
        // Determine if the vehicle should stop at a red light.
        // (isLightRedForVehicle is assumed to be defined elsewhere.)
        bool inIntersection = (v->x >= ROAD_X_START && v->x <= ROAD_X_START + ROAD_WIDTH &&
                               v->y >= ROAD_Y_START && v->y <= ROAD_Y_START + ROAD_HEIGHT);
        if (isLightRedForVehicle(v) && !inIntersection)
            v->speed = 0;
        else
            v->speed = VEHICLE_SPEED;
        
        // Update positions based on the vehicle's direction:
        // 0 = Down (Road B), 1 = Right (Road A), 2 = Up (Road D), 3 = Left (Road C)
        switch (v->direction) {
            case 0: // Down (Road B)
                v->x = verticalLaneCenters[v->lane - 1] - (VEHICLE_WIDTH / 2);
                v->y += v->speed;
                if (v->y > SCREEN_HEIGHT)
                    v->id = -1;
                break;
            case 1: // Right (Road A)
                v->x += v->speed;
                v->y = horizontalLaneCenters[v->lane - 1] - (VEHICLE_HEIGHT / 2);
                if (v->x > SCREEN_WIDTH)
                    v->id = -1;
                break;
            case 2: // Up (Road D)
                v->x = verticalLaneCenters[v->lane - 1] - (VEHICLE_WIDTH / 2);
                v->y -= v->speed;
                if (v->y + VEHICLE_HEIGHT < 0)
                    v->id = -1;
                break;
            case 3: // Left (Road C)
                v->x -= v->speed;
                v->y = horizontalLaneCenters[v->lane - 1] - (VEHICLE_HEIGHT / 2);
                if (v->x + VEHICLE_WIDTH < 0)
                    v->id = -1;
                break;
        }
        
        // Update the vehicle's rendering rectangle
        v->rect.x = (int)v->x;
        v->rect.y = (int)v->y;
    }
}
