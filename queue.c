#include "queue.h"
#include <stdlib.h>
#include <math.h>

// Initialize the priority queue
void initPriorityQueue(PriorityQueue *pq, int maxSize) {
    pq->data = (LanePriority *)malloc(sizeof(LanePriority) * maxSize);
    pq->size = 0;
    pq->capacity = maxSize;
}

// Enqueue an item into the priority queue
void enqueuePriority(PriorityQueue *pq, LanePriority item) {
    if (pq->size == pq->capacity) return;
    pq->data[pq->size++] = item;
}

// Dequeue the highest priority item from the queue
LanePriority dequeuePriority(PriorityQueue *pq, Vehicle vehicles[]) {
    if (pq->size == 0) {
        // Return an invalid priority if queue is empty
        return (LanePriority){'X', -1, -1};
    }
    
    // Find the highest priority lane
    int maxPriorityIndex = 0;
    for (int i = 1; i < pq->size; i++) {
        if (pq->data[i].priority > pq->data[maxPriorityIndex].priority) {
            maxPriorityIndex = i;
        }
    }
    
    // Get the highest priority item
    LanePriority item = pq->data[maxPriorityIndex];
    
    // Remove the item from the queue by replacing it with the last item
    pq->data[maxPriorityIndex] = pq->data[pq->size - 1];
    pq->size--;
    
    return item;
}

// Check if the priority queue is empty
bool isEmptyPriority(PriorityQueue *pq) {
    return pq->size == 0;
}

// Update the priority of a specific lane in the queue
void updatePriority(PriorityQueue *pq, char road, int lane, int newPriority) {
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i].road == road && pq->data[i].lane == lane) {
            pq->data[i].priority = newPriority;
            return;
        }
    }
}

// Count the number of waiting vehicles on a specific road
int countWaitingVehicles(Vehicle vehicles[], char road) {
    int count = 0;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id != -1 && vehicles[i].road == road && vehicles[i].speed == 0) {
            count++;
        }
    }
    return count;
}

// Handle priority roads based on vehicle count
void handlePriorityRoads(PriorityQueue *pq, Vehicle vehicles[]) {
    int waitingA = countWaitingVehicles(vehicles, 'A');
    int waitingB = countWaitingVehicles(vehicles, 'B');
    
    static char priorityRoad = 'X'; // No priority initially
    
    if (priorityRoad == 'X') {
        // Check if any road needs priority
        if (waitingA > PRIORITY_THRESHOLD) {
            priorityRoad = 'A';
            printf("Road A has high priority (waiting vehicles: %d)\n", waitingA);
        } else if (waitingB > PRIORITY_THRESHOLD) {
            priorityRoad = 'B';
            printf("Road B has high priority (waiting vehicles: %d)\n", waitingB);
        }
    } else {
        // Check if priority can be lifted
        if ((priorityRoad == 'A' && waitingA < NORMAL_THRESHOLD) ||
            (priorityRoad == 'B' && waitingB < NORMAL_THRESHOLD)) {
            printf("Returning to normal conditions (Road %c waiting vehicles: %d)\n", 
                  priorityRoad, priorityRoad == 'A' ? waitingA : waitingB);
            priorityRoad = 'X';
        }
    }
    
    // Update priorities in the queue
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i].road == priorityRoad) {
            pq->data[i].priority = 10; // High priority
        } else {
            pq->data[i].priority = 1;  // Normal priority
        }
    }
}

// Update vehicle positions based on the priority queue
void updateVehicles(PriorityQueue *pq, Vehicle vehicles[]) {
    int laneWidth = ROAD_WIDTH / 3;
    int verticalLaneCenters[3] = {
        ROAD_X_START + (0 * laneWidth) + (laneWidth / 2),
        ROAD_X_START + (1 * laneWidth) + (laneWidth / 2),
        ROAD_X_START + (2 * laneWidth) + (laneWidth / 2)
    };
    int horizontalLaneCenters[3] = {
        ROAD_Y_START + (0 * laneWidth) + (laneWidth / 2),
        ROAD_Y_START + (1 * laneWidth) + (laneWidth / 2),
        ROAD_Y_START + (2 * laneWidth) + (laneWidth / 2)
    };

    // Process all vehicles regardless of queue
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) continue;
        
        Vehicle *v = &vehicles[i];
        
        // Check if vehicle should be stopped at red light
        bool inIntersection = (v->x >= ROAD_X_START && v->x <= ROAD_X_START + ROAD_WIDTH &&
                              v->y >= ROAD_Y_START && v->y <= ROAD_Y_START + ROAD_WIDTH);
        
        if (isLightRedForVehicle(v) && !inIntersection) {
            v->speed = 0;
        } else {
            v->speed = VEHICLE_SPEED;
        }

        // Update vehicle position based on direction
        switch (v->direction) {
        case 0: // Down
            v->x = verticalLaneCenters[v->lane] - (VEHICLE_WIDTH / 2); // Lock x
            v->y += v->speed;
            if (v->y > SCREEN_HEIGHT) v->id = -1;
            break;
        case 1: // Right
            v->x += v->speed;
            v->y = horizontalLaneCenters[v->lane] - (VEHICLE_HEIGHT / 2); // Lock y
            if (v->x > SCREEN_WIDTH) v->id = -1;
            break;
        case 2: // Up
            v->x = verticalLaneCenters[v->lane] - (VEHICLE_WIDTH / 2); // Lock x
            v->y -= v->speed;
            if (v->y + VEHICLE_HEIGHT < 0) v->id = -1;
            break;
        case 3: // Left
            v->x -= v->speed;
            v->y = horizontalLaneCenters[v->lane] - (VEHICLE_HEIGHT / 2); // Lock y
            if (v->x + VEHICLE_WIDTH < 0) v->id = -1;
            break;
        }
        
        // Update the vehicle's rect for rendering
        v->rect.x = (int)v->x;
        v->rect.y = (int)v->y;
    }
}