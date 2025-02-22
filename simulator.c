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

// Define global variables
Vehicle vehicles[MAX_VEHICLES];
int lastVehicleId = 0;
SDL_Texture *carTexture = NULL;
TrafficLight trafficLights[8];
Uint32 lastBlink = 0;
bool isLightRed = true;
Uint32 lastSpawnTime = 0;


void renderZebraCrossing(SDL_Renderer *renderer) {
    // Define constants if not already defined in queue.h
    #ifndef ZEBRA_CROSSING_WIDTH
    #define ZEBRA_CROSSING_WIDTH 20  // Width/height of each stripe
    #endif
    #ifndef ZEBRA_CROSSING_GAP
    #define ZEBRA_CROSSING_GAP 10    // Gap between stripes
    #endif

    // Set the color for zebra crossing stripes (white)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Zebra crossing for the horizontal road (left to right)
    for (int i = 0; i < ROAD_WIDTH; i += (ZEBRA_CROSSING_WIDTH + ZEBRA_CROSSING_GAP)) {
        // Top zebra crossing (above the vertical road)
        SDL_Rect topStripe = {
            ROAD_X_START + i,                     // X position (starts at ROAD_X_START)
            ROAD_Y_START - ZEBRA_CROSSING_WIDTH,  // Y position (above the vertical road)
            ZEBRA_CROSSING_WIDTH,                 // Width of the stripe
            ZEBRA_CROSSING_WIDTH                  // Height of the stripe
        };
        SDL_RenderFillRect(renderer, &topStripe);

        // Bottom zebra crossing (below the vertical road)
        SDL_Rect bottomStripe = {
            ROAD_X_START + i,                     // X position (starts at ROAD_X_START)
            ROAD_Y_START + ROAD_WIDTH,            // Y position (below the vertical road)
            ZEBRA_CROSSING_WIDTH,                 // Width of the stripe
            ZEBRA_CROSSING_WIDTH                  // Height of the stripe
        };
        SDL_RenderFillRect(renderer, &bottomStripe);
    }

    // Zebra crossing for the vertical road (top to bottom)
    for (int i = 0; i < ROAD_WIDTH; i += (ZEBRA_CROSSING_WIDTH + ZEBRA_CROSSING_GAP)) {
        // Left zebra crossing (left of the horizontal road)
        SDL_Rect leftStripe = {
            ROAD_X_START - ZEBRA_CROSSING_WIDTH,  // X position (left of the horizontal road)
            ROAD_Y_START + i,                     // Y position (starts at ROAD_Y_START)
            ZEBRA_CROSSING_WIDTH,                 // Width of the stripe
            ZEBRA_CROSSING_WIDTH                  // Height of the stripe
        };
        SDL_RenderFillRect(renderer, &leftStripe);

        // Right zebra crossing (right of the horizontal road)
        SDL_Rect rightStripe = {
            ROAD_X_START + ROAD_WIDTH,            // X position (right of the horizontal road)
            ROAD_Y_START + i,                     // Y position (starts at ROAD_Y_START)
            ZEBRA_CROSSING_WIDTH,                 // Width of the stripe
            ZEBRA_CROSSING_WIDTH                  // Height of the stripe
        };
        SDL_RenderFillRect(renderer, &rightStripe);
    }
}

void renderLane(SDL_Renderer *renderer) {
    // Roads
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect horizontalRoad = {ROAD_X_START, 0, ROAD_WIDTH, SCREEN_HEIGHT};
    SDL_Rect verticalRoad = {0, ROAD_Y_START, SCREEN_WIDTH, ROAD_WIDTH};
    SDL_RenderFillRect(renderer, &horizontalRoad);
    SDL_RenderFillRect(renderer, &verticalRoad);

    // Lane markings
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int dash = 40, gap = 20;
    for (int i = 1; i < 3; i++) {
        int xLane = ROAD_X_START + (ROAD_WIDTH / 3) * i;
        int yLane = ROAD_Y_START + (ROAD_WIDTH / 3) * i;
        for (int y = 0; y < SCREEN_HEIGHT; y += dash + gap) {
            if (y + dash < ROAD_Y_START || y > ROAD_Y_START + ROAD_WIDTH) {
                SDL_Rect dashLine = {xLane - 2, y, 5, dash};
                SDL_RenderFillRect(renderer, &dashLine);
            }
        }
        for (int x = 0; x < SCREEN_WIDTH; x += dash + gap) {
            if (x + dash < ROAD_X_START || x > ROAD_X_START + ROAD_WIDTH) {
                SDL_Rect dashLine = {x, yLane - 2, dash, 5};
                SDL_RenderFillRect(renderer, &dashLine);
            }
        }
    }
}

void renderTrafficLights(SDL_Renderer *renderer) {
    SDL_Color color = isLightRed ? (SDL_Color){255, 0, 0, 255} : (SDL_Color){0, 255, 0, 255};
    for (int i = 0; i < 8; i += 2) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect light = {(int)trafficLights[i + (isLightRed ? 0 : 1)].x - LIGHT_SIZE / 2,
                          (int)trafficLights[i + (isLightRed ? 0 : 1)].y - LIGHT_SIZE / 2,
                          LIGHT_SIZE, LIGHT_SIZE};
        SDL_RenderFillRect(renderer, &light);
    }
}

void renderVehicles(SDL_Renderer *renderer) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        Vehicle *v = &vehicles[i];
        if (v->id < 0)
            continue;

        v->rect.x = (int)v->x;
        v->rect.y = (int)v->y;
        double angle = 0.0;
        switch (v->direction) {
        case 0: angle = 180.0; break; // Down
        case 1: angle = 90.0; break;  // Right
        case 2: angle = 0.0; break;   // Up
        case 3: angle = 270.0; break; // Left
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
        if (vehicles[i].id != -1 && isLightRedForVehicle(&vehicles[i])) {
            vehicles[i].speed = 0;
        } else {
            vehicles[i].speed = VEHICLE_SPEED;
        }
    }
}

void updateVehicles(PriorityQueue *pq, Vehicle vehicles[]) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) continue;

        switch (vehicles[i].direction) {
        case 0: // Down
            vehicles[i].y += vehicles[i].speed;
            if (vehicles[i].y > SCREEN_HEIGHT) vehicles[i].id = -1;
            break;
        case 1: // Right
            vehicles[i].x += vehicles[i].speed;
            if (vehicles[i].x > SCREEN_WIDTH) vehicles[i].id = -1;
            break;
        case 2: // Up
            vehicles[i].y -= vehicles[i].speed;
            if (vehicles[i].y + VEHICLE_HEIGHT < 0) vehicles[i].id = -1;
            break;
        case 3: // Left
            vehicles[i].x -= vehicles[i].speed;
            if (vehicles[i].x + VEHICLE_WIDTH < 0) vehicles[i].id = -1;
            break;
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
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
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

    for (int i = 0; i < MAX_VEHICLES; i++)
        vehicles[i].id = -1;

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
            if (e.type == SDL_QUIT)
                quit = true;
        }

        if (SDL_GetTicks() - lastBlink > LIGHT_DURATION) {
            isLightRed = !isLightRed;
            lastBlink = SDL_GetTicks();
        }

        // Generate vehicles
        generateVehicle(&pq, vehicles, &lastVehicleId);
        printf("Generated vehicle with ID: %d\n", lastVehicleId); // Debug print

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
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    free(pq.data);
    SDL_DestroyTexture(carTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}