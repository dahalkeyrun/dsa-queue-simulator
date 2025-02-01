#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_WIDTH 1500
#define SCREEN_HEIGHT 1200

// Function to render a lane
void renderLane(SDL_Renderer *renderer)
{
    // Draw the road
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect roadRect = {0, 500, SCREEN_WIDTH, 500};
    SDL_RenderFillRect(renderer, &roadRect);

    // Draw a dashed white line in the center of the road
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int dashWidth = 20;
    int gapWidth = 20;
    int yPosition1 = 700;
    int yPosition2 = 850; // Adjusted for a 3-lane road
    for (int x = 0; x < SCREEN_WIDTH; x += dashWidth + gapWidth)
    {
        SDL_Rect dash = {x, yPosition1, dashWidth, 5};
        SDL_RenderFillRect(renderer, &dash);
    }

    for (int x = 0; x < SCREEN_WIDTH; x += dashWidth + gapWidth)
    {
        SDL_Rect dash = {x, yPosition2, dashWidth, 5};
        SDL_RenderFillRect(renderer, &dash);
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