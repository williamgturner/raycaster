#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FOV 60.0
#define WORLD_SCALE 64
#define MAP_WIDTH 15
#define MAP_HEIGHT 15
#define TO_RADIANS ( * M_PI/180)

SDL_Window *window;
SDL_Renderer *renderer;
uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
int quit = 0;

uint8_t map[SCREEN_HEIGHT * SCREEN_WIDTH] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 2, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1,
    1, 0, 2, 0, 3, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

float rotate(float theta, float delta)
{
    theta += delta;
    if (theta < 1) {
        theta += 360;
    } else if (theta > 360) {
        theta -= 360;
    }
    return theta;
}

void draw_vert(int y0, int y1, int column, int colour) {
    for (int i = y0; i < y1; i++) {
    pixels[i * SCREEN_WIDTH + column] =  colour;
    }
}
// Poo Test
int main(int argc, char *argv[]) {
    // Init SDL, create window, renderer, and texture for graphics rendering
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Couldn't initialise SDL_VIDEO");
        return 1;
    }
    
    window = SDL_CreateWindow(
        "Raycast Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        480,
        SDL_WINDOW_ALLOW_HIGHDPI);

    if(!window >= 0) {
        printf("Couldn't initialise window");
        return 1;  
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if(!renderer) {
        printf("Coudln't initialise renderer");
        return 1;
    
    }

    SDL_Texture *texture;
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);
    // init fonts
    TTF_Init();
    TTF_Font* Mono = TTF_OpenFont("src/Fixedsys62.ttf", 48);
    if (!Mono) {
            printf("Failed to load font: %s\n", TTF_GetError());
            return 1;
    }
    SDL_Color White = {255, 255, 255};
    char text_buffer[50]; // text buffer for debug readout

    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
        // Main Loop
        SDL_Delay(50);
        memset(pixels, 0, sizeof(pixels));
        render();
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
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
