#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define WORLD_SCALE 64
#define MAP_WIDTH 6
#define MAP_HEIGHT 6
#define FOV 60.0

SDL_Window *window;
SDL_Renderer *renderer;
uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
int quit = 0;

uint8_t map[SCREEN_HEIGHT * SCREEN_WIDTH] = {
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

observer camera = {4 * WORLD_SCALE, 3 * WORLD_SCALE, 0.0};

float rotate(float theta, float delta)
{
    theta += delta;
    if (theta < 1) {
        theta += 360;
    } else if (theta > 359) {
        theta -= 360;
    }
    return theta;
}

void draw_square(int x, int y, int size, int colour)
{
    for(int i = x; i < x + size; i++) {
        for (int j = 0; j < size; j++) {
            pixels[(y + j) * SCREEN_WIDTH + i] = colour; // green
        }
    }
}

// Implementation of Bresenham's line algorithim
void generate_line_points(int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int x = x0;
    int y = y0;
    int p = 2 * dy - dx;

    while (x < x1) {
        if (p >= 0) {
            pixels[y * SCREEN_WIDTH + x] = 0xFFFFFF;
            y += 1;
            p += 2 * dy - 2*dx;
        } else {
            pixels[y * SCREEN_WIDTH + x] = 0xFFFFFF;
            p += 2*dy;
        }
        x+= 1;
    }
}
void render_minimap()
{
    // bottom left = SCREEN_WDITH * SCREEN_HEIGHT - 25 * 6 - (25*6*SCREEN_WIDTH)
    // printf("Test");
    static const int CELL_SIZE = 10;
    for (int row = 0; row < MAP_HEIGHT; row ++) {
        for (int column = 0; column < MAP_WIDTH; column++) {
            if (map[row * MAP_WIDTH + column] != 0){
                draw_square(SCREEN_WIDTH - (CELL_SIZE * MAP_WIDTH) + column * CELL_SIZE, SCREEN_HEIGHT - CELL_SIZE - row * CELL_SIZE, CELL_SIZE, 0x00FF00);
            }
            // printf("X: %d | Y: %d\n", SCREEN_WIDTH - (10 * 6) + column * 10, SCREEN_HEIGHT - 10 - row * 10);
        }
    }

    // player pos in minimap scale
    int mini_x = camera.x / 64 * 10;
    int mini_y = camera.y / 64 * 10;

    // draw player pos
    draw_square((SCREEN_WIDTH - (CELL_SIZE * MAP_WIDTH)) + mini_x, SCREEN_HEIGHT - mini_y, 2, 0xFFFFFF);

    // draw cone of vision

}

int main(int argc, char *argv[]) {
    // Init 
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
    assert(renderer >= 0);

    SDL_Texture *texture;
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);

    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
        SDL_Delay(50);
        memset(pixels, 0, sizeof(pixels));
        //draw_square(300, 270, 10, 0x00FF00);
        //draw_square(SCREEN_WIDTH - (10 * 6), SCREEN_HEIGHT - (10 * 6), 10);
        render_minimap();
        generate_line_points(10, 10, 50, 30);
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
        
        const uint8_t *keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_LEFT]) {
            camera.x -= 10;
        }

        if (keystate[SDL_SCANCODE_RIGHT]) {
            camera.x += 10;
        }

        if (keystate[SDL_SCANCODE_UP]) {
            camera.y -= 10;
        }

        if (keystate[SDL_SCANCODE_DOWN]) {
            camera.y += 10;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
