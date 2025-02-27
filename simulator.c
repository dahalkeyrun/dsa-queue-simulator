#include "queue.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#define FRAME_DELAY (1000 / DESIRED_FPS)
#define TIME_PER_VEHICLE 1000  // milliseconds allocated per vehicle to pass

// Global simulation variables
Vehicle vehicles[MAX_VEHICLES];
int lastVehicleId = 0;
SDL_Texture *carTexture = NULL;
TrafficLight trafficLights[8];
Uint32 lastBlink = 0;
Uint32 lastSpawnTime = 0;
Uint32 clearingStartTime = 0;

// New globals for managing the intersection light state:
char currentGreenRoad = 'A';
Uint32 currentGreenStartTime = 0;
Uint32 currentGreenDuration = 4000; // Default duration (ms)
int currentRoadIndex = 0;
char roads[4] = {'A', 'B', 'C', 'D'};

// Function prototypes
void renderZebraCrossing(SDL_Renderer *renderer);
void renderLane(SDL_Renderer *renderer);
void renderTrafficLights(SDL_Renderer *renderer);
void renderVehicles(SDL_Renderer *renderer);
bool isNearLight(Vehicle *v);

// Update isLightRedForVehicle so that vehicles on the current green road have a green light.
bool isLightRedForVehicle(Vehicle *v) {
    if (!isNearLight(v))
        return false;
    // If the vehicle's road is currently green, then its light is not red.
    if (v->road == currentGreenRoad)
        return false;
    // Otherwise, if the vehicle is approaching the intersection, return true.
    bool approachingIntersection = false;
    switch (v->direction) {
        case 0: approachingIntersection = (v->y < ROAD_Y_START); break;  // Down: hasn't reached intersection
        case 1: approachingIntersection = (v->x < ROAD_X_START); break;  // Right: hasn't reached intersection
        case 2: approachingIntersection = (v->y > ROAD_Y_START + ROAD_WIDTH); break; // Up: not reached
        case 3: approachingIntersection = (v->x > ROAD_X_START + ROAD_WIDTH); break; // Left: not reached
    }
    return approachingIntersection;
}

void renderZebraCrossing(SDL_Renderer *renderer) {
    for (int i = 0; i < ROAD_WIDTH; i += (ZEBRA_CROSSING_WIDTH + ZEBRA_CROSSING_GAP)) {
        // Top and bottom stripes
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect topStripe = {ROAD_X_START + i, ROAD_Y_START - ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &topStripe);
        SDL_Rect bottomStripe = {ROAD_X_START + i, ROAD_Y_START + ROAD_WIDTH, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &bottomStripe);
        
        // Left and right stripes
        SDL_Rect leftStripe = {ROAD_X_START - ZEBRA_CROSSING_WIDTH, ROAD_Y_START + i, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &leftStripe);
        SDL_Rect rightStripe = {ROAD_X_START + ROAD_WIDTH, ROAD_Y_START + i, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &rightStripe);
    }
}

void renderLane(SDL_Renderer *renderer) {
    // Render road fills
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect horizontalRoad = {ROAD_X_START, 0, ROAD_WIDTH, SCREEN_HEIGHT};
    SDL_Rect verticalRoad = {0, ROAD_Y_START, SCREEN_WIDTH, ROAD_WIDTH};
    SDL_RenderFillRect(renderer, &horizontalRoad);
    SDL_RenderFillRect(renderer, &verticalRoad);
    
    // Render borders
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_Rect horizontalBorder = {ROAD_X_START - 2, -2, ROAD_WIDTH + 4, SCREEN_HEIGHT + 4};
    SDL_Rect verticalBorder = {-2, ROAD_Y_START - 2, SCREEN_WIDTH + 4, ROAD_WIDTH + 4};
    SDL_RenderDrawRect(renderer, &horizontalBorder);
    SDL_RenderDrawRect(renderer, &verticalBorder);
    
    // Render lane markings (using dashed lines)
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    int dash = 30, gap = 15;
    for (int i = 1; i < 3; i++) {
        int xLane = ROAD_X_START + (ROAD_WIDTH / 3) * i;
        int yLane = ROAD_Y_START + (ROAD_WIDTH / 3) * i;
        for (int y = 0; y < SCREEN_HEIGHT; y += dash + gap) {
            if (y + dash < ROAD_Y_START || y > ROAD_Y_START + ROAD_WIDTH) {
                SDL_Rect dashLine = {xLane - 3, y, 6, dash};
                SDL_RenderFillRect(renderer, &dashLine);
            }
        }
        for (int x = 0; x < SCREEN_WIDTH; x += dash + gap) {
            if (x + dash < ROAD_X_START || x > ROAD_X_START + ROAD_WIDTH) {
                SDL_Rect dashLine = {x, yLane - 3, dash, 6};
                SDL_RenderFillRect(renderer, &dashLine);
            }
        }
    }
}

void renderTrafficLights(SDL_Renderer *renderer) {
    // Assume:
    //   Lights 0-1 belong to Road A,
    //   Lights 2-3 to Road B,
    //   Lights 4-5 to Road C,
    //   Lights 6-7 to Road D.
    for (int i = 0; i < 8; i++) {
        char lightRoad;
        if (i < 2)
            lightRoad = 'A';
        else if (i < 4)
            lightRoad = 'B';
        else if (i < 6)
            lightRoad = 'C';
        else
            lightRoad = 'D';

        // Set color based on whether this light's road is currently green.
        if (lightRoad == currentGreenRoad) {
            SDL_SetRenderDrawColor(renderer, 50, 255, 50, 255); // Green
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); // Red
        }
        int x = (int)trafficLights[i].x;
        int y = (int)trafficLights[i].y;
        SDL_Rect lightRect = {x - LIGHT_SIZE/2, y - LIGHT_SIZE/2, LIGHT_SIZE, LIGHT_SIZE};
        SDL_RenderFillRect(renderer, &lightRect);
        
        // Draw border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect border = {x - LIGHT_SIZE/2 - 2, y - LIGHT_SIZE/2 - 2, LIGHT_SIZE + 4, LIGHT_SIZE + 4};
        SDL_RenderDrawRect(renderer, &border);
    }
}

void renderVehicles(SDL_Renderer *renderer) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        Vehicle *v = &vehicles[i];
        if (v->id < 0)
            continue;
        
        double angle = 0.0;
        switch (v->direction) {
            case 0: angle = 180.0; break; // Down
            case 1: angle = 90.0;  break; // Right
            case 2: angle = 0.0;   break; // Up
            case 3: angle = 270.0; break; // Left
        }
        
        if (SDL_RenderCopyEx(renderer, carTexture, NULL, &v->rect, angle, NULL, SDL_FLIP_NONE) < 0) {
            printf("SDL_RenderCopyEx failed: %s\n", SDL_GetError());
        }
    }
}

bool isNearLight(Vehicle *v) {
    return (v->x > ROAD_X_START - 100 && v->x < ROAD_X_START + ROAD_WIDTH + 100 &&
            v->y > ROAD_Y_START - 100 && v->y < ROAD_Y_START + ROAD_WIDTH + 100);
}

int main() {
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }
    SDL_Window *window = SDL_CreateWindow("Traffic Simulator", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    SDL_Surface *carSurface = IMG_Load("car1.png");
    if (!carSurface) {
        printf("Failed to load car1.png: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    carTexture = SDL_CreateTextureFromSurface(renderer, carSurface);
    if (!carTexture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(carSurface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }
    SDL_FreeSurface(carSurface);

    // Initialize vehicles array
    for (int i = 0; i < MAX_VEHICLES; i++)
        vehicles[i].id = -1;

    // Initialize traffic light positions (assumed positions around the intersection)
    trafficLights[0] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START - LIGHT_OFFSET};
    trafficLights[1] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START + LIGHT_SIZE + LIGHT_OFFSET};
    trafficLights[2] = (TrafficLight){ROAD_X_START + ROAD_WIDTH + LIGHT_OFFSET, ROAD_Y_START - LIGHT_OFFSET};
    trafficLights[3] = (TrafficLight){ROAD_X_START + ROAD_WIDTH + LIGHT_OFFSET, ROAD_Y_START + LIGHT_SIZE + LIGHT_OFFSET};
    trafficLights[4] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START + ROAD_WIDTH + LIGHT_OFFSET};
    trafficLights[5] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START + ROAD_WIDTH - LIGHT_SIZE - LIGHT_OFFSET};
    trafficLights[6] = (TrafficLight){ROAD_X_START + ROAD_WIDTH + LIGHT_OFFSET, ROAD_Y_START + ROAD_WIDTH + LIGHT_OFFSET};
    trafficLights[7] = (TrafficLight){ROAD_X_START + ROAD_WIDTH + LIGHT_OFFSET, ROAD_Y_START + ROAD_WIDTH - LIGHT_SIZE - LIGHT_OFFSET};

    // Initialize the priority queue
    PriorityQueue pq;
    initPriorityQueue(&pq, 12);

    // Initialize our green-light timer and road rotation
    currentGreenRoad = roads[currentRoadIndex];
    currentGreenStartTime = SDL_GetTicks();
    currentGreenDuration = 4000; // initial default

    bool quit = false;
    SDL_Event e;
    Uint32 frameStart, frameTime;

    while (!quit) {
        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
        }
        
        Uint32 currentTime = SDL_GetTicks();
        
        // --- Traffic Light Control: Determine which road gets the green light ---
        if (currentTime - currentGreenStartTime >= currentGreenDuration) {
            // First, update the priority queue for any high-priority lane.
            handlePriorityRoads(&pq, vehicles);
            bool priorityFound = false;
            // Check if any lane in the priority queue has the high priority value.
            for (int i = 0; i < pq.size; i++) {
                if (pq.data[i].priority == 10) {
                    currentGreenRoad = pq.data[i].road;
                    priorityFound = true;
                    break;
                }
            }
            // If no priority lane is found, use round-robin rotation.
            if (!priorityFound) {
                currentRoadIndex = (currentRoadIndex + 1) % 4;
                currentGreenRoad = roads[currentRoadIndex];
            }
            // Determine how many vehicles are waiting on the current green road.
            int vehiclesToServe = countWaitingVehicles(vehicles, currentGreenRoad);
            if (vehiclesToServe <= 0)
                vehiclesToServe = 2;  // minimum duration if no vehicles are waiting
            currentGreenDuration = vehiclesToServe * TIME_PER_VEHICLE;
            currentGreenStartTime = currentTime;
            printf("Green light for Road %c for %d ms (waiting vehicles: %d)\n",
                   currentGreenRoad, currentGreenDuration, vehiclesToServe);
        }
        
        // --- Traffic Generation and Queue Management ---
        generateVehicle(&pq, vehicles, &lastVehicleId);
        handlePriorityRoads(&pq, vehicles);
        
        // --- Update Vehicle Positions ---
        // updateVehicles now uses our updated isLightRedForVehicle logic.
        updateVehicles(vehicles);
        
        // --- Vehicle Redirection at Intersection ---
        for (int i = 0; i < MAX_VEHICLES; i++) {
            if (vehicles[i].id != -1) {
                // Check if the vehicle is within the intersection bounds.
                bool inIntersection = (
                    vehicles[i].x >= ROAD_X_START && 
                    vehicles[i].x <= ROAD_X_START + ROAD_WIDTH &&
                    vehicles[i].y >= ROAD_Y_START && 
                    vehicles[i].y <= ROAD_Y_START + ROAD_WIDTH
                );
                if (inIntersection && shouldRedirect()) {
                    redirectVehicle(&vehicles[i]);
                }
            }
        }
        
        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderLane(renderer);
        renderZebraCrossing(renderer);
        renderVehicles(renderer);
        renderTrafficLights(renderer);
        SDL_RenderPresent(renderer);
        
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY)
            SDL_Delay(FRAME_DELAY - frameTime);
    }

    free(pq.data);
    SDL_DestroyTexture(carTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
