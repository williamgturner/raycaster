#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

SDL_Window *window;
uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
int quit = 0;

int main(int argc, char *argv[]) {
    //init 
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);

    window = SDL_CreateWindow(
        "Raycast Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_ALLOW_HIGHDPI);
    
    assert(window >= 0);
    // SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    // assert (window_surface != NULL);

    SDL_UpdateWindowSurface(window);

    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
        SDL_Delay(100);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}