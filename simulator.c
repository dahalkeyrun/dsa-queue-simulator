#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 2000
#define SCREEN_HEIGHT 1700

// Function to render a lane
void renderLane(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    SDL_Rect horizontalRoad = {(SCREEN_WIDTH - 500) / 2, 0, 400, SCREEN_HEIGHT};
    SDL_Rect verticalRoad = {0, (SCREEN_HEIGHT - 500) / 2, SCREEN_WIDTH, 400};

    SDL_RenderFillRect(renderer, &horizontalRoad);
    SDL_RenderFillRect(renderer, &verticalRoad);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int dashLength = 40, gapLength = 20;
    int laneWidth = horizontalRoad.w / 3;
    int laneHeight = verticalRoad.h / 3;

    int xLane1 = horizontalRoad.x + laneWidth;
    int xLane2 = horizontalRoad.x + 2 * laneWidth;
    int yLane1 = verticalRoad.y + laneHeight;
    int yLane2 = verticalRoad.y + 2 * laneHeight;

    int junctionStartX = horizontalRoad.x, junctionEndX = horizontalRoad.x + horizontalRoad.w;
    int junctionStartY = verticalRoad.y, junctionEndY = verticalRoad.y + verticalRoad.h;

    for (int x = 0; x < SCREEN_WIDTH; x += dashLength + gapLength)
    {
        if (x + dashLength < junctionStartX || x > junctionEndX)
        {
            SDL_Rect dash1 = {x, yLane1 - 2, dashLength, 5};
            SDL_Rect dash2 = {x, yLane2 - 2, dashLength, 5};
            SDL_RenderFillRect(renderer, &dash1);
            SDL_RenderFillRect(renderer, &dash2);
        }
    }

    for (int y = 0; y < SCREEN_HEIGHT; y += dashLength + gapLength)
    {
        if (y + dashLength < junctionStartY || y > junctionEndY)
        {
            SDL_Rect dash1 = {xLane1 - 2, y, 5, dashLength};
            SDL_Rect dash2 = {xLane2 - 2, y, 5, dashLength};
            SDL_RenderFillRect(renderer, &dash1);
            SDL_RenderFillRect(renderer, &dash2);
        }
    }
}

// Image rendering
void renderImage(SDL_Renderer *renderer, SDL_Texture *texture)
{
    int original_width, original_height;
    SDL_QueryTexture(texture, NULL, NULL, &original_width, &original_height);

    // Scaling factor
    float scale_factor = 0.1;
    int new_width = original_width * scale_factor;
    int new_height = original_height * scale_factor;

    // Center the image
    SDL_Rect dstRect = {
        (SCREEN_WIDTH - new_width) / 2,   // X position
        (SCREEN_HEIGHT - new_height) / 2, // Y position
        new_width,
        new_height};
    
    // Render the image
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error : %s\n", SDL_GetError());
        return 1;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Traffic Simulator UI",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("SDL_CreateWindow Error : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        SDL_DestroyWindow(window);
        printf("SDL_CreateRenderer Error : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface *image_surface = IMG_Load("car1.png");
    if (!image_surface)
    {
        printf("Unable to load image! SDL_image Error : %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    SDL_FreeSurface(image_surface);
    if (!texture)
    {
        printf("Unable to create texture from surface! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Rect dstRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100, 200, 200};

    bool quit = false;
    SDL_Event e;

    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderLane(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &dstRect); 

        // Call the renderImage function
        renderImage(renderer, texture);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture); 
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
