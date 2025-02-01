#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_WIDTH 2000
#define SCREEN_HEIGHT 1700

// Function to render a lane
void renderLane(SDL_Renderer *renderer)
{
    // Ensure solid colors (Disable blending)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Set the road color (dark gray)
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    // Define road dimensions
    SDL_Rect horizontalRoad = {(SCREEN_WIDTH - 500) / 2, 0, 400, SCREEN_HEIGHT};
    SDL_Rect verticalRoad = {0, (SCREEN_HEIGHT - 500) / 2, SCREEN_WIDTH, 400};

    // Draw roads
    SDL_RenderFillRect(renderer, &horizontalRoad);
    SDL_RenderFillRect(renderer, &verticalRoad);

    // Set white for dashed lines
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int dashLength = 40;
    int gapLength = 20;
    int laneWidth = horizontalRoad.w / 3; // Corrected lane width calculation
    int laneHeight = verticalRoad.h / 3;  // Corrected lane height calculation

    // 🔹 Centered lane positions
    int xLane1 = horizontalRoad.x + laneWidth;     // Center of first horizontal lane
    int xLane2 = horizontalRoad.x + 2 * laneWidth; // Center of second horizontal lane

    int yLane1 = verticalRoad.y + laneHeight;     // Center of first vertical lane
    int yLane2 = verticalRoad.y + 2 * laneHeight; // Center of second vertical lane

    int junctionStartX = horizontalRoad.x;
    int junctionEndX = horizontalRoad.x + horizontalRoad.w;
    int junctionStartY = verticalRoad.y;
    int junctionEndY = verticalRoad.y + verticalRoad.h;

    //  Corrected Dashed lines for horizontal lanes
    for (int x = 0; x < SCREEN_WIDTH; x += dashLength + gapLength)
    {

        if (x + dashLength < junctionStartX || x > junctionEndX)
        {

            SDL_Rect dash1 = {x, yLane1 - 2, dashLength, 5}; // Adjust for exact center
            SDL_Rect dash2 = {x, yLane2 - 2, dashLength, 5};
            SDL_RenderFillRect(renderer, &dash1);
            SDL_RenderFillRect(renderer, &dash2);
        }
    }

    //  Corrected Dashed lines for vertical lanes
    for (int y = 0; y < SCREEN_HEIGHT; y += dashLength + gapLength)
    {
        if (y + dashLength < junctionStartY || y > junctionEndY)
        {

            SDL_Rect dash1 = {xLane1 - 2, y, 5, dashLength}; // Adjust for exact center
            SDL_Rect dash2 = {xLane2 - 2, y, 5, dashLength};
            SDL_RenderFillRect(renderer, &dash1);
            SDL_RenderFillRect(renderer, &dash2);
        }
    }
}

int main()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error : %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Traffic Simulator UI",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow Error : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(window);
        printf("SDL_CreateRenderer Error : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Main loop flag
    bool quit = false;

    // Event handler
    SDL_Event e;

    // While application is running
    while (!quit)
    {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            // User requests quit
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render the lane
        renderLane(renderer);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // Destroy window and renderer
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Quit SDL subsystems
    SDL_Quit();

    return 0;
}