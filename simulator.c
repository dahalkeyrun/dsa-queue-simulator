

#include "queue.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define VEHICLE_LOG "vehicles.log"
#define SCREEN_WIDTH 2000
#define SCREEN_HEIGHT 1700
#define VEHICLE_WIDTH 50
#define VEHICLE_HEIGHT 100
#define MAX_VEHICLES 200
#define MAX_LANES 12
#define LIGHT_SIZE 50
#define LIGHT_OFFSET 10
#define DESIRED_FPS 60
#define FRAME_DELAY (1000 / DESIRED_FPS)

const int ROAD_X_START = (SCREEN_WIDTH - 500) / 2;
const int ROAD_Y_START = (SCREEN_HEIGHT - 500) / 2;

Vehicle vehicles[MAX_VEHICLES];
int lastVehicleId = 0;
SDL_Texture *carTexture = NULL;
TrafficLight trafficLights[8]; // 2 lights per corner, 4 corners
Uint32 lastBlink = 0;
bool isLightRed = true;
Uint32 lastSpawnTime = 0;

void *readVehicleLog(void *arg);
void adjustVehicleMovementByLights(PriorityQueue *pq, Vehicle vehicles[]);
bool isNearLight(Vehicle *v);
bool isLightRedForVehicle(Vehicle *v);

void *readVehicleLog(void *arg) {
    FILE *logFile;
    char line[256];
    while(1) {
        logFile = fopen(VEHICLE_LOG, "r");
        if(logFile == NULL) {
            perror("Error opening log file");
            sleep(1); // Waits before trying again
            continue;
        }
        while (fgets(line, sizeof(line), logFile)) {
            int id, speed, direction;
            char road;
            int lane;
            float x, y;
            if (sscanf(line, "%d:%c:%d:%d:%f:%f:%d\n", &id, &road, &lane, &direction, &x, &y, &speed) == 7) {
                int found = 0;
                for (int i = 0; i < MAX_VEHICLES; i++) {
                    if (vehicles[i].id == id) {
                        vehicles[i].x = x;
                        vehicles[i].y = y;
                        vehicles[i].speed = (float)speed / DESIRED_FPS; // Adjust speed for frame rate
                        vehicles[i].direction = direction;
                        vehicles[i].road = road;
                        vehicles[i].lane = lane;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    for (int i = 0; i < MAX_VEHICLES; i++) {
                        if (vehicles[i].id == -1) {
                            vehicles[i].id = id;
                            vehicles[i].x = x;
                            vehicles[i].y = y;
                            vehicles[i].speed = (float)speed / DESIRED_FPS;
                            vehicles[i].direction = direction;
                            vehicles[i].road = road;
                            vehicles[i].lane = lane;
                            vehicles[i].rect = (SDL_Rect){(int)x, (int)y, VEHICLE_WIDTH, VEHICLE_HEIGHT};
                            break;
                        }
                    }
                }
            }
        }
        fclose(logFile);
        sleep(1); // Adjust this sleep to balance between responsiveness and CPU usage
    }
    return NULL;
}

void renderVehicles(SDL_Renderer *renderer)
{
    for (int i = 0; i < MAX_VEHICLES; i++)
{
 Vehicle *v = &vehicles[i];
        if (v->id < 0)
            continue;

        v->rect.x = (int)v->x;
        v->rect.y = (int)v->y;

        double angle = 0.0;
        switch (v->direction)
        {
        case 0:
            angle = 180.0;
            break; // Down
        case 1:
            angle = 90.0;
            break; // Right
        case 2:
            angle = 360.0;
            break; // Up
        case 3:
            angle = 270.0;
            break; // Left
        }
        SDL_RenderCopyEx(renderer, carTexture, NULL, &v->rect, angle, NULL, SDL_FLIP_NONE);
    }
}

void renderLane(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect horizontalRoad = {ROAD_X_START, 0, 400, SCREEN_HEIGHT};
    SDL_Rect verticalRoad = {0, ROAD_Y_START, SCREEN_WIDTH, 400};
    SDL_RenderFillRect(renderer, &horizontalRoad);
    SDL_RenderFillRect(renderer, &verticalRoad);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int dash = 40, gap = 20;
    for (int i = 1; i < 3; i++)
    {
        int xLane = ROAD_X_START + (400 / 3) * i;
        int yLane = ROAD_Y_START + (400 / 3) * i;
        for (int y = 0; y < SCREEN_HEIGHT; y += dash + gap)
        {
            if (y + dash < ROAD_Y_START || y > ROAD_Y_START + 400)
            {
                SDL_Rect dashLine = {xLane - 2, y, 5, dash};
                SDL_RenderFillRect(renderer, &dashLine);
            }
        }
        for (int x = 0; x < SCREEN_WIDTH; x += dash + gap)
        {
            if (x + dash < ROAD_X_START || x > ROAD_X_START + 400)
            {
                SDL_Rect dashLine = {x, yLane - 2, dash, 5};
                SDL_RenderFillRect(renderer, &dashLine);
            }
        }
    }
}

void renderTrafficLights(SDL_Renderer *renderer)
{
    SDL_Color color = isLightRed ? (SDL_Color){255, 0, 0, 255} : (SDL_Color){0, 255, 0, 255};

    for (int i = 0; i < 8; i += 2)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect light = {(int)trafficLights[i + (isLightRed ? 0 : 1)].x - LIGHT_SIZE / 2, (int)trafficLights[i + (isLightRed ? 0 : 1)].y - LIGHT_SIZE / 2, LIGHT_SIZE, LIGHT_SIZE};
        SDL_RenderFillRect(renderer, &light);
    }
}

bool isNearLight(Vehicle *v) {
    // Implement logic here, for now, simple check
    return v->y < 100 || v->y > SCREEN_HEIGHT - 100 || v->x < 100 || v->x > SCREEN_WIDTH - 100;
}

bool isLightRedForVehicle(Vehicle *v) {
    // For this example, assume lights alternate every 500ms
    return isLightRed;
}

void adjustVehicleMovementByLights(PriorityQueue *pq, Vehicle vehicles[]) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        Vehicle *v = &vehicles[i];
        if (v->id < 0) continue;

        if (isNearLight(v)) {
            if (isLightRedForVehicle(v)) {
                v->speed = 0; // Stop the vehicle
            } else {
                v->speed = fminf(v->speed, 3.0f / DESIRED_FPS); // Limit speed when near light
            }
        }
    }
}

int main()
{
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        printf("Failed to initialize: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Traffic Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    pthread_t vehicleLogReader;
    if (pthread_create(&vehicleLogReader, NULL, readVehicleLog, NULL) != 0) {
        printf("Error creating thread for reading vehicle log\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Surface *carSurface = IMG_Load("car1.png");
    if (!carSurface)
    {
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
trafficLights[2] = (TrafficLight){ROAD_X_START + 400 + LIGHT_OFFSET, ROAD_Y_START - LIGHT_OFFSET};
trafficLights[3] = (TrafficLight){ROAD_X_START + 400 + LIGHT_OFFSET, ROAD_Y_START + LIGHT_SIZE + LIGHT_OFFSET};
trafficLights[4] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START + 400 + LIGHT_OFFSET};
trafficLights[5] = (TrafficLight){ROAD_X_START - LIGHT_OFFSET, ROAD_Y_START + 400 - LIGHT_SIZE - LIGHT_OFFSET};
trafficLights[6] = (TrafficLight){ROAD_X_START + 400 + LIGHT_OFFSET, ROAD_Y_START + 400 + LIGHT_OFFSET};
trafficLights[7] = (TrafficLight){ROAD_X_START + 400 + LIGHT_OFFSET, ROAD_Y_START + 400 - LIGHT_SIZE - LIGHT_OFFSET};

    PriorityQueue pq;
    initPriorityQueue(&pq, MAX_LANES);
    // Setup initial priorities, if any

    bool quit = false;
    SDL_Event e;
    Uint32 frameStart, frameTime;

    while (!quit)
    {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;
        }

        if (SDL_GetTicks() - lastBlink > 500)
        { // Blink every 500ms
            isLightRed = !isLightRed;
            lastBlink = SDL_GetTicks();
        }

        generateVehicle(NULL, vehicles, &lastVehicleId); // Vehicle generation moved to traffic_generator
        updateVehicles(NULL, vehicles); // Move vehicles
        adjustVehicleMovementByLights(&pq, vehicles); // Control movement by lights

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderLane(renderer);
        renderVehicles(renderer);
        renderTrafficLights(renderer);

        SDL_RenderPresent(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY)
        {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
        
    }

    pthread_join(vehicleLogReader, NULL);

    SDL_DestroyTexture(carTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}