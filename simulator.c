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
#define GREEN_DURATION 8000
#define RED_DURATION 8000

// Define global variables
Vehicle vehicles[MAX_VEHICLES];
int lastVehicleId = 0;
SDL_Texture *carTexture = NULL;
TrafficLight trafficLights[8];
Uint32 lastBlink = 0;
bool isLightRed = true;
Uint32 lastSpawnTime = 0;
Uint32 clearingStartTime = 0;

// Function prototypes
void renderZebraCrossing(SDL_Renderer *renderer);
void renderLane(SDL_Renderer *renderer);
void renderTrafficLights(SDL_Renderer *renderer);
void renderVehicles(SDL_Renderer *renderer);
bool isNearLight(Vehicle *v);
bool isLightRedForVehicle(Vehicle *v);
void adjustVehicleMovementByLights(PriorityQueue *pq, Vehicle vehicles[]);

void renderZebraCrossing(SDL_Renderer *renderer) {
    for (int i = 0; i < ROAD_WIDTH; i += (ZEBRA_CROSSING_WIDTH + ZEBRA_CROSSING_GAP)) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
        SDL_Rect topWhiteStripe = {ROAD_X_START + i, ROAD_Y_START - ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &topWhiteStripe);
        SDL_Rect bottomWhiteStripe = {ROAD_X_START + i, ROAD_Y_START + ROAD_WIDTH, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &bottomWhiteStripe);
        SDL_Rect leftWhiteStripe = {ROAD_X_START - ZEBRA_CROSSING_WIDTH, ROAD_Y_START + i, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &leftWhiteStripe);
        SDL_Rect rightWhiteStripe = {ROAD_X_START + ROAD_WIDTH, ROAD_Y_START + i, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &rightWhiteStripe);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
        SDL_Rect topBlackStripe = {ROAD_X_START + i + ZEBRA_CROSSING_WIDTH + ZEBRA_CROSSING_GAP, ROAD_Y_START - ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &topBlackStripe);
        SDL_Rect bottomBlackStripe = {ROAD_X_START + i + ZEBRA_CROSSING_WIDTH + ZEBRA_CROSSING_GAP, ROAD_Y_START + ROAD_WIDTH, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &bottomBlackStripe);
        SDL_Rect leftBlackStripe = {ROAD_X_START - ZEBRA_CROSSING_WIDTH, ROAD_Y_START + i + ZEBRA_CROSSING_WIDTH + ZEBRA_CROSSING_GAP, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &leftBlackStripe);
        SDL_Rect rightBlackStripe = {ROAD_X_START + ROAD_WIDTH, ROAD_Y_START + i + ZEBRA_CROSSING_WIDTH + ZEBRA_CROSSING_GAP, ZEBRA_CROSSING_WIDTH, ZEBRA_CROSSING_WIDTH};
        SDL_RenderFillRect(renderer, &rightBlackStripe);
    }
}

void renderLane(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Gray color for road fill
    SDL_Rect horizontalRoad = {ROAD_X_START, 0, ROAD_WIDTH, SCREEN_HEIGHT};
    SDL_Rect verticalRoad = {0, ROAD_Y_START, SCREEN_WIDTH, ROAD_WIDTH};
    SDL_RenderFillRect(renderer, &horizontalRoad);
    SDL_RenderFillRect(renderer, &verticalRoad);

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // Darker gray for border
    SDL_Rect horizontalBorder = {ROAD_X_START - 2, -2, ROAD_WIDTH + 4, SCREEN_HEIGHT + 4};
    SDL_Rect verticalBorder = {-2, ROAD_Y_START - 2, SCREEN_WIDTH + 4, ROAD_WIDTH + 4};
    SDL_RenderDrawRect(renderer, &horizontalBorder);
    SDL_RenderDrawRect(renderer, &verticalBorder);

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Bright yellow for lane markings
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
    for (int i = 0; i < 8; i += 2) {
        int lightIndex = isLightRed ? 0 : 1;
        float x = trafficLights[i + lightIndex].x;
        float y = trafficLights[i + lightIndex].y;
        if (isLightRed) SDL_SetRenderDrawColor(renderer, 255, 50, 50, 100);
        else SDL_SetRenderDrawColor(renderer, 50, 255, 50, 100);
        for (int r = LIGHT_SIZE + 5; r > LIGHT_SIZE; r -= 2) {
            SDL_Rect glow = {(int)x - r / 2, (int)y - r / 2, r, r};
            SDL_RenderFillRect(renderer, &glow);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect border = {(int)x - LIGHT_SIZE / 2 - 2, (int)y - LIGHT_SIZE / 2 - 2, LIGHT_SIZE + 4, LIGHT_SIZE + 4};
        SDL_RenderDrawRect(renderer, &border);
        if (isLightRed) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        else SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect light = {(int)x - LIGHT_SIZE / 2, (int)y - LIGHT_SIZE / 2, LIGHT_SIZE, LIGHT_SIZE};
        SDL_RenderFillRect(renderer, &light);
    }
}

void renderVehicles(SDL_Renderer *renderer) {
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
        Vehicle *v = &vehicles[i];
        if (v->id < 0) continue;

        if (v->lane < 0 || v->lane > 2) {
            printf("Invalid lane %d for Vehicle ID %d, clamping to 0\n", v->lane, v->id);
            v->lane = 0;
        }

        switch (v->direction) {
        case 0: // Down
            v->x = verticalLaneCenters[v->lane] - (VEHICLE_WIDTH / 2);
            v->rect.x = (int)v->x;
            v->rect.y = (int)v->y;
            break;
        case 1: // Right
            v->y = horizontalLaneCenters[v->lane] - (VEHICLE_HEIGHT / 2);
            v->rect.x = (int)v->x;
            v->rect.y = (int)v->y;
            break;
        case 2: // Up
            v->x = verticalLaneCenters[v->lane] - (VEHICLE_WIDTH / 2);
            v->rect.x = (int)v->x;
            v->rect.y = (int)v->y;
            break;
        case 3: // Left
            v->y = horizontalLaneCenters[v->lane] - (VEHICLE_HEIGHT / 2);
            v->rect.x = (int)v->x;
            v->rect.y = (int)v->y;
            break;
        }

        double angle = 0.0;
        switch (v->direction) {
        case 0: angle = 180.0; break;
        case 1: angle = 90.0; break;
        case 2: angle = 0.0; break;
        case 3: angle = 270.0; break;
        }
        if (SDL_RenderCopyEx(renderer, carTexture, NULL, &v->rect, angle, NULL, SDL_FLIP_NONE) < 0) {
            printf("SDL_RenderCopyEx failed: %s\n", SDL_GetError());
        }
    }
}

bool isNearLight(Vehicle *v) {
    return (v->x > ROAD_X_START - 50 && v->x < ROAD_X_START + ROAD_WIDTH + 50 &&
            v->y > ROAD_Y_START - 50 && v->y < ROAD_Y_START + ROAD_WIDTH + 50);
}

bool isLightRedForVehicle(Vehicle *v) {
    return isLightRed && isNearLight(v);
}

void adjustVehicleMovementByLights(PriorityQueue *pq, Vehicle vehicles[]) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) continue;
        Vehicle *v = &vehicles[i];
        bool inIntersection = (v->x >= ROAD_X_START && v->x <= ROAD_X_START + ROAD_WIDTH &&
                               v->y >= ROAD_Y_START && v->y <= ROAD_Y_START + ROAD_WIDTH);
        if (isLightRedForVehicle(v) && !inIntersection) {
            v->speed = 0;
        } else {
            v->speed = VEHICLE_SPEED;
        }
    }
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

    for (int i = 0; i < MAX_VEHICLES; i++) vehicles[i].id = -1;

    trafficLights[0] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START - LIGHT_OFFSET};
    trafficLights[1] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START + LIGHT_SIZE + LIGHT_OFFSET};
    trafficLights[2] = (TrafficLight){ROAD_X_START + ROAD_WIDTH + LIGHT_OFFSET, ROAD_Y_START - LIGHT_OFFSET};
    trafficLights[3] = (TrafficLight){ROAD_X_START + ROAD_WIDTH + LIGHT_OFFSET, ROAD_Y_START + LIGHT_SIZE + LIGHT_OFFSET};
    trafficLights[4] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START + ROAD_WIDTH + LIGHT_OFFSET};
    trafficLights[5] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START + ROAD_WIDTH - LIGHT_SIZE - LIGHT_OFFSET};
    trafficLights[6] = (TrafficLight){ROAD_X_START + ROAD_WIDTH + LIGHT_OFFSET, ROAD_Y_START + ROAD_WIDTH + LIGHT_OFFSET};
    trafficLights[7] = (TrafficLight){ROAD_X_START + ROAD_WIDTH + LIGHT_OFFSET, ROAD_Y_START + ROAD_WIDTH - LIGHT_SIZE - LIGHT_OFFSET};

    PriorityQueue pq;
    initPriorityQueue(&pq, 12);

    bool quit = false;
    SDL_Event e;
    Uint32 frameStart, frameTime;

    while (!quit) {
        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsed = currentTime - lastBlink;
        if (isLightRed && elapsed > RED_DURATION) {
            isLightRed = false;
            lastBlink = currentTime;
            clearingStartTime = currentTime;
        } else if (!isLightRed && elapsed > GREEN_DURATION) {
            isLightRed = false;
            lastBlink = currentTime;
        }
        generateVehicle(&pq, vehicles, &lastVehicleId);
        updateVehicles(&pq, vehicles);
        adjustVehicleMovementByLights(&pq, vehicles);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderLane(renderer);
        renderZebraCrossing(renderer);
        renderVehicles(renderer);
        renderTrafficLights(renderer);
        SDL_RenderPresent(renderer);
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) SDL_Delay(FRAME_DELAY - frameTime);
    }

    free(pq.data);
    SDL_DestroyTexture(carTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}