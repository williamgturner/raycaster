#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define WORLD_SCALE 64
#define FOV 60.0

SDL_Window *window;
SDL_Renderer *renderer;
uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
int quit = 0;

uint8_t map[36] = {
    1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 1,
    1, 0, 0, 2, 0, 1,
    1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1,
};

typedef struct {
    float x;
    float y;
    float theta;
} observer;

observer camera = {3 * 64, 3 * 64, 0.0};

void cast_rays() {
    static float ray_delta = FOV / (float) SCREEN_WIDTH;
    float ray_theta = (camera.theta - 30);
    int ray_x = 0;
    int ray_y = 0;

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        ray_theta += i * ray_delta;
        if (ray_theta >= 270 || ray_theta <= 90) {
            ray_y = ((int) camera.y / WORLD_SCALE) * 64 + 64;
        } else {
            ray_y = ((int) camera.y / WORLD_SCALE) * 64 - 1;
        }
    }
}

int main(int argc, char *argv[]) {
    //init 
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);

    window = SDL_CreateWindow(
        "Raycast Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        480,
        SDL_WINDOW_ALLOW_HIGHDPI);
    
    assert(window >= 0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    assert(renderer >=0);
    // SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    // assert (window_surface != NULL);

    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Texture *texture;
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);

    // debug
    for (int y = 0; y <= SCREEN_HEIGHT; y++) {
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