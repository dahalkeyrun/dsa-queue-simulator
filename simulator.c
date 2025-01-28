#include <SDL2/SDL.h>

int main()
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    int pixel_size = 10;
    SDL_Rect rect;
    rect.x = 250;
    rect.y = 250;
    rect.w = pixel_size;
    rect.h = pixel_size;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(500, 500, 0, &window, &renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);
    SDL_Delay(10000);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
