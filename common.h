#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>

#define MAX_VEHICLES 50
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define ROAD_X_START 200
#define ROAD_Y_START 200
#define ROAD_WIDTH 400       // Added
#define VEHICLE_SPEED 2.0f
#define VEHICLE_WIDTH 30
#define VEHICLE_HEIGHT 20
#define TOTAL_LANES 12       // Added
#define LIGHT_SIZE 20        // Added
#define LIGHT_OFFSET 30      // Added
#define LIGHT_DURATION 5000  // Added (5 seconds)
#define ZEBRA_CROSSING_WIDTH 10  // Added
#define ZEBRA_CROSSING_GAP 5     // Added
#define DESIRED_FPS 60       // Added

typedef enum {
    STRAIGHT,
    LEFT,
    RIGHT
} TurnDirection;

typedef struct {
    float x, y;          // Added TrafficLight struct
} TrafficLight;

typedef struct Vehicle {
    int id;
    float x, y;
    int direction;
    int lane;
    char road;
    char target_road;
    TurnDirection turn_direction;
    SDL_Rect rect;
    float speed;
    int priority;
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

#endif