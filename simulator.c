#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 2000
#define SCREEN_HEIGHT 1700
#define VEHICLE_WIDTH 50
#define VEHICLE_HEIGHT 100
#define MAX_VEHICLES 200
#define MAX_LANES 12
#define LIGHT_SIZE 50
#define LIGHT_OFFSET 10
#define VEHICLE_SPAWN_INTERVAL 1000 // Time between vehicle spawns in milliseconds

typedef struct Vehicle {
    int id;
    char road;
    int lane;
    SDL_Rect rect;
    int speed;
    int direction;  // 0: down, 1: right, 2: up, 3: left
} Vehicle;

typedef struct {
    char road;
    int lane;
    int priority;
} LanePriority;

typedef struct {
    LanePriority* data;
    int size;
    int capacity;
} PriorityQueue;

typedef struct {
    int x, y;
} TrafficLight;

Vehicle vehicles[MAX_VEHICLES];
const int ROAD_X_START = (SCREEN_WIDTH - 500) / 2;
const int ROAD_Y_START = (SCREEN_HEIGHT - 500) / 2;

SDL_Texture *carTexture;
TrafficLight trafficLights[8]; // 2 lights per corner, 4 corners
Uint32 lastBlink = 0;
bool isLightRed = true;
Uint32 lastSpawnTime = 0;

// Function prototypes
void initPriorityQueue(PriorityQueue* pq, int maxSize);
void heapifyUp(PriorityQueue* pq, int index);
void heapifyDown(PriorityQueue* pq, int index);
void enqueuePriority(PriorityQueue* pq, LanePriority item);
LanePriority dequeuePriority(PriorityQueue* pq);
bool isEmptyPriority(PriorityQueue* pq);
void updatePriority(PriorityQueue* pq, char road, int lane, int newPriority);
void generateVehicle();
void updateVehicles(PriorityQueue* pq);
void renderVehicles(SDL_Renderer *renderer);
void renderLane(SDL_Renderer *renderer);
void renderTrafficLights(SDL_Renderer *renderer);

// Initialize Priority Queue
void initPriorityQueue(PriorityQueue* pq, int maxSize) {
    pq->data = (LanePriority*)malloc(sizeof(LanePriority) * maxSize);
    pq->size = 0;
    pq->capacity = maxSize;
}

void heapifyUp(PriorityQueue* pq, int index) {
    int parent = (index - 1) / 2;
    while (index > 0 && pq->data[parent].priority > pq->data[index].priority) {
        LanePriority temp = pq->data[parent];
        pq->data[parent] = pq->data[index];
        pq->data[index] = temp;
        index = parent;
        parent = (index - 1) / 2;
    }
}

void heapifyDown(PriorityQueue* pq, int index) {
    int smallest = index;
    int leftChild = 2 * index + 1;
    int rightChild = 2 * index + 2;

    if (leftChild < pq->size && pq->data[leftChild].priority < pq->data[smallest].priority)
        smallest = leftChild;

    if (rightChild < pq->size && pq->data[rightChild].priority < pq->data[smallest].priority)
        smallest = rightChild;

    if (smallest != index) {
        LanePriority temp = pq->data[index];
        pq->data[index] = pq->data[smallest];
        pq->data[smallest] = temp;
        heapifyDown(pq, smallest);
    }
}

void enqueuePriority(PriorityQueue* pq, LanePriority item) {
    if (pq->size == pq->capacity) return;
    pq->data[pq->size] = item;
    heapifyUp(pq, pq->size);
    pq->size++;
}

LanePriority dequeuePriority(PriorityQueue* pq) {
    if (pq->size == 0) {
        LanePriority empty = {'X', -1, -1};
        return empty;
    }
    LanePriority result = pq->data[0];
    pq->data[0] = pq->data[pq->size - 1];
    pq->size--;
    heapifyDown(pq, 0);
    return result;
}

bool isEmptyPriority(PriorityQueue* pq) {
    return pq->size == 0;
}

void updatePriority(PriorityQueue* pq, char road, int lane, int newPriority) {
    for (int i = 0; i < pq->size; i++) {
        if (pq->data[i].road == road && pq->data[i].lane == lane) {
            pq->data[i].priority = newPriority;
            heapifyUp(pq, i);
            break;
        }
    }
}
void generateVehicle() {
    static int id = 0;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastSpawnTime < VEHICLE_SPAWN_INTERVAL) return;
    lastSpawnTime = currentTime;

    // Find an inactive vehicle slot
    int freeSlot = -1;
    for (int i = 0; i < MAX_VEHICLES; i++) {
        if (vehicles[i].id == -1) {
            freeSlot = i;
            break;
        }
    }

    if (freeSlot == -1) return; // No free slots

    Vehicle *v = &vehicles[freeSlot];
    v->id = id++;
    v->speed = rand() % 3 + 2;  // Speed between 2 and 4
    v->direction = rand() % 4;  // 0: down, 1: right, 2: up, 3: left
    v->road = 'A' + (rand() % 4); 
    v->lane = rand() % 3 + 1;  // Random lane from 1 to 3

    int laneWidth = 400 / 3;
    int laneHeight = 400 / 3;

    switch (v->direction) {
        case 0:  // Down
            v->rect = (SDL_Rect){ROAD_X_START + (v->lane - 1) * laneWidth + (laneWidth - VEHICLE_WIDTH) / 2, -VEHICLE_HEIGHT, VEHICLE_WIDTH, VEHICLE_HEIGHT};
            break;
        case 1:  // Right
            v->rect = (SDL_Rect){-VEHICLE_HEIGHT, ROAD_Y_START + (v->lane - 1) * laneHeight + (laneHeight - VEHICLE_WIDTH) / 2, VEHICLE_HEIGHT, VEHICLE_WIDTH};
            break;
        case 2:  // Up
            v->rect = (SDL_Rect){ROAD_X_START + (v->lane - 1) * laneWidth + (laneWidth - VEHICLE_WIDTH) / 2, SCREEN_HEIGHT, VEHICLE_WIDTH, VEHICLE_HEIGHT};
            v->rect.y -= VEHICLE_HEIGHT;
            break;
        case 3:  // Left
            v->rect = (SDL_Rect){SCREEN_WIDTH, ROAD_Y_START + (v->lane - 1) * laneHeight + (laneHeight - VEHICLE_WIDTH) / 2, VEHICLE_HEIGHT, VEHICLE_WIDTH};
            v->rect.x -= VEHICLE_HEIGHT;
            break;
    }
}

void updateVehicles(PriorityQueue* pq) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        Vehicle *v = &vehicles[i];
        if (v->id < 0) continue;

        switch (v->direction) {
            case 0:  // Down
                v->rect.y += v->speed;
                if (v->rect.y > SCREEN_HEIGHT) v->id = -1;
                break;
            case 1:  // Right
                v->rect.x += v->speed;
                if (v->rect.x > SCREEN_WIDTH) v->id = -1;
                break;
            case 2:  // Up
                v->rect.y -= v->speed;
                if (v->rect.y + VEHICLE_HEIGHT < 0) v->id = -1;
                break;
            case 3:  // Left
                v->rect.x -= v->speed;
                if (v->rect.x + VEHICLE_HEIGHT < 0) v->id = -1;
                break;
        }
    }
}

void renderVehicles(SDL_Renderer *renderer) {
    for (int i = 0; i < MAX_VEHICLES; i++) {
        Vehicle *v = &vehicles[i];
        if (v->id < 0) continue;

        double angle = 0.0;
        switch (v->direction) {
            case 0: angle = 180.0; break;  // Down
            case 1: angle = 90.0; break;   // Right
            case 2: angle = 360.0; break; // Up
            case 3: angle = 270.0; break; // Left
        }
        SDL_RenderCopyEx(renderer, carTexture, NULL, &v->rect, angle, NULL, SDL_FLIP_NONE);
    }
}

void renderLane(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect horizontalRoad = {ROAD_X_START, 0, 400, SCREEN_HEIGHT};
    SDL_Rect verticalRoad = {0, ROAD_Y_START, SCREEN_WIDTH, 400};
    SDL_RenderFillRect(renderer, &horizontalRoad);
    SDL_RenderFillRect(renderer, &verticalRoad);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int dash = 40, gap = 20;
    for (int i = 1; i < 3; i++) {
        int xLane = ROAD_X_START + (400 / 3) * i;
        int yLane = ROAD_Y_START + (400 / 3) * i;
        for (int y = 0; y < SCREEN_HEIGHT; y += dash + gap) {
            if (y + dash < ROAD_Y_START || y > ROAD_Y_START + 400) {
                SDL_Rect dashLine = {xLane - 2, y, 5, dash};
                SDL_RenderFillRect(renderer, &dashLine);
            }
        }
        for (int x = 0; x < SCREEN_WIDTH; x += dash + gap) {
            if (x + dash < ROAD_X_START || x > ROAD_X_START + 400) {
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
        SDL_Rect light = {trafficLights[i + (isLightRed ? 0 : 1)].x - LIGHT_SIZE/2, trafficLights[i + (isLightRed ? 0 : 1)].y - LIGHT_SIZE/2, LIGHT_SIZE, LIGHT_SIZE};
        SDL_RenderFillRect(renderer, &light);
    }
}


int main() {
    srand(time(NULL));
    if(SDL_Init(SDL_INIT_VIDEO) < 0 || IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("Failed to initialize: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Traffic Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
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
    SDL_FreeSurface(carSurface);

    // Initialize vehicles
    for (int i = 0; i < MAX_VEHICLES; i++) vehicles[i].id = -1;

    // Initialize traffic lights
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
    for (char road = 'A'; road <= 'D'; road++) {
        for (int lane = 1; lane <= 3; lane++) {
            LanePriority lp = {road, lane, 1};
            enqueuePriority(&pq, lp);
        }
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }

        if (SDL_GetTicks() - lastBlink > 500) { // Blink every 500ms
            isLightRed = !isLightRed;
            lastBlink = SDL_GetTicks();
        }

        generateVehicle(); // Continuous generation
        
        LanePriority lp = dequeuePriority(&pq);
        if (lp.road == 'A' && lp.lane == 2 && lp.priority == 1) { 
            updatePriority(&pq, 'A', 2, 0); 
        }
        enqueuePriority(&pq, lp); 

        updateVehicles(&pq);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderLane(renderer);
        renderVehicles(renderer);
        renderTrafficLights(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    free(pq.data);
    SDL_DestroyTexture(carTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}