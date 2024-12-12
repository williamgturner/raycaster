#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
SDL_Window *window;
SDL_Renderer *renderer;
uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
int quit = 0;

uint8_t map[36] = {
    1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1,
};

typedef struct {
    float x;
    float y;
    float theta;
} observer;

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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    assert(renderer >=0);
    // SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    // assert (window_surface != NULL);

    SDL_SetRenderDrawColor(renderer, 0xff,0x00,0x00,SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Texture *texture;
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);

    pixels[300] = 0xFF0000FF;
    for (int y = 50; y <= 300; y++) {
        pixels[(y * SCREEN_WIDTH) + 100] = 0xFF0000FF;
    }

    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * 4);
    
    SDL_RenderCopyEx(
            renderer,
            texture,
            NULL,
            NULL,
            0.0,
            NULL,
            SDL_FLIP_VERTICAL);
    SDL_RenderPresent(renderer);

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