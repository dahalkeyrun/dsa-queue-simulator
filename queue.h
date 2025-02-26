#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#define DESIRED_FPS 60
#define MAX_VEHICLES 200
#define SCREEN_WIDTH 2000
#define SCREEN_HEIGHT 1700
#define VEHICLE_WIDTH 50
#define VEHICLE_HEIGHT 100
#define LIGHT_SIZE 50
#define LIGHT_OFFSET 10
#define LIGHT_DURATION 5000
#define ZEBRA_CROSSING_WIDTH 20
#define ZEBRA_CROSSING_GAP 10
#define VEHICLE_SPEED 2.0f
#define ROAD_WIDTH 400
#define ROAD_X_START ((SCREEN_WIDTH - ROAD_WIDTH) / 2)
#define ROAD_Y_START ((SCREEN_HEIGHT - ROAD_WIDTH) / 2)
#define PRIORITY_THRESHOLD 10
#define NORMAL_THRESHOLD 5
#define CLEARING_TIME 2000
#define MIN_VEHICLE_SPACING 100

typedef struct {
    float x, y;
} TrafficLight;

typedef struct Vehicle {
    int id;
    char road;
    int lane;
    float x, y;
    float speed;
    int direction;
    SDL_Rect rect;
    bool isPriority;
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

// External declarations for global variables
extern Vehicle vehicles[MAX_VEHICLES];
extern int lastVehicleId;
extern SDL_Texture *carTexture;
extern TrafficLight trafficLights[8];
extern Uint32 lastBlink;
extern bool isLightRed;
extern Uint32 lastSpawnTime;
extern Uint32 clearingStartTime;

// Function prototypes
void initPriorityQueue(PriorityQueue *pq, int maxSize);
void enqueuePriority(PriorityQueue *pq, LanePriority item);
LanePriority dequeuePriority(PriorityQueue *pq, Vehicle vehicles[]);
bool isEmptyPriority(PriorityQueue *pq);
void updatePriority(PriorityQueue *pq, char road, int lane, int newPriority);

void generateVehicle(PriorityQueue *pq, Vehicle vehicles[], int *lastVehicleId);
void updateVehicles(PriorityQueue *pq, Vehicle vehicles[]);
void adjustVehicleMovementByLights(PriorityQueue *pq, Vehicle vehicles[]);
bool isNearLight(Vehicle *v);
bool isLightRedForVehicle(Vehicle *v);

// Vehicle redirection functions
void redirectVehicle(Vehicle *v);
bool shouldRedirect();

// Priority road management
int countWaitingVehicles(Vehicle vehicles[], char road);
void handlePriorityRoads(PriorityQueue *pq, Vehicle vehicles[]);

#endif