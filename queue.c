#include "queue.h"
#include <stdlib.h>
#include <math.h>

void initPriorityQueue(PriorityQueue *pq, int maxSize) {
    pq->data = (LanePriority *)malloc(sizeof(LanePriority) * maxSize);
    pq->size = 0;
    pq->capacity = maxSize;
}

void enqueuePriority(PriorityQueue *pq, LanePriority item) {
    if (pq->size == pq->capacity) return;
    pq->data[pq->size++] = item;
}

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

bool isEmptyPriority(PriorityQueue *pq) {
    return pq->size == 0;
}

void updatePriority(PriorityQueue *pq, char road, int lane, int newPriority) {
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i].road == road && pq->data[i].lane == lane) {
            pq->data[i].priority = newPriority;
            break;
        }
    }
}

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

    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) continue;
        Vehicle *v = &vehicles[i];

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
    }
}