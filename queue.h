#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define DESIRED_FPS 60
#define VEHICLE_LOG "vehicles.log"
#define MAX_VEHICLES 200
#define SCREEN_WIDTH 2000
#define SCREEN_HEIGHT 1700
#define VEHICLE_WIDTH 50
#define VEHICLE_HEIGHT 100

typedef struct {
    float x, y;
} TrafficLight;

typedef struct Vehicle {
    int id;
    char road;
    int lane;
    float x, y; // Use float for smoother movement
    float speed; // Speed per frame
    int direction; // 0: down, 1: right, 2: up, 3: left
    SDL_Rect rect; // Keep this for rendering
} Vehicle;

typedef struct {
    char road;
    int lane;
    int priority;
} LanePriority;

typedef struct {
    LanePriority *data;
    int size;
    int capacity;
} PriorityQueue;

// External variables
extern const int ROAD_X_START;
extern const int ROAD_Y_START;
extern Vehicle vehicles[MAX_VEHICLES];
extern int lastVehicleId;
extern SDL_Texture *carTexture;
extern TrafficLight trafficLights[8];
extern Uint32 lastBlink;
extern bool isLightRed;
extern Uint32 lastSpawnTime;

// Function prototypes
void initPriorityQueue(PriorityQueue *pq, int maxSize);
void enqueuePriority(PriorityQueue *pq, LanePriority item);
LanePriority dequeuePriority(PriorityQueue *pq);
bool isEmptyPriority(PriorityQueue *pq);
void updatePriority(PriorityQueue *pq, char road, int lane, int newPriority);
void generateVehicle(PriorityQueue *pq, Vehicle vehicles[], int *lastVehicleId);
void updateVehicles(PriorityQueue *pq, Vehicle vehicles[]);
void *readVehicleLog(void *arg);
void logVehicleToFile(Vehicle *v);

#endif // QUEUE_H