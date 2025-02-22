#ifndef TRAFFIC_GENERATOR_H
#define TRAFFIC_GENERATOR_H

#include "queue.h"

extern Vehicle vehicles[MAX_VEHICLES];
extern int lastVehicleId;
extern Uint32 lastSpawnTime;

void generateVehicle(PriorityQueue *pq, Vehicle vehicles[], int *lastVehicleId);

#endif