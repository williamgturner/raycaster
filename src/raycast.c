#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>


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

observer camera = {2 * 64, 3 * 64, 0.0};

void draw_wall (int column, float distance) {
    int height =  64.0f / distance * 277;
    int top = 120 + (height / 2);
    for (int i = (SCREEN_HEIGHT - height) / 2; i < top; i++) {
        pixels[i * SCREEN_WIDTH + column] = 0xFF0000FF;
    }
}

float check_horizontal_collisions(float ray_theta) {
    int ray_x = 0;
    int ray_y = 0;
    int Ya = 0;
    int Xa = 0;
    int dist_to_wall = 0;

    if (ray_theta >= 180 || ray_theta <= 90) {
            ray_y = ((int) camera.y / WORLD_SCALE) * 64 - 1;
            Ya = -WORLD_SCALE;
        } else {
            ray_y = ((int) camera.y / WORLD_SCALE) * 64 + 64;
            Ya = WORLD_SCALE;
        }
        ray_x = camera.x + ((camera.y - ray_y)/(tan(ray_theta)));
        Xa = (int) 64/(tan(ray_theta));

        while (map[(ray_x / WORLD_SCALE) + ((ray_y / WORLD_SCALE) * 6)] == 0) {
            ray_x += Xa;
            ray_y += Ya;
            if (ray_y / 64 >= 7 * 64) {
                printf("Lol");
                break;
            }
            if (ray_x / 64 >= 7 * 64) {
                printf("Lol");
                break;
            }
        }

    return (int) (camera.x - ray_x / cos(ray_theta));
}

float check_vertical_collions(float ray_theta) {
    int ray_x = 0;
    int ray_y = 0;
    int Ya = 0;
    int Xa = 0;
    int dist_to_wall = 0;

    if (ray_theta >= 0 || ray_theta <= 180) {
        ray_x = (int) (camera.x/64);
        ray_x = ray_x * 64 + 64;
        Xa = WORLD_SCALE;
    } else {
        ray_x = (int) (camera.x/64) * 64;
        ray_x -= 1;
        Xa = -WORLD_SCALE;
    }
    Ya = 64 * tan(ray_theta);
    ray_y = camera.y + ((camera.x-ray_x) * tan(ray_theta));

    while (map[(ray_x / WORLD_SCALE) + ((ray_y / WORLD_SCALE) * 6)] == 0) {
            ray_x += Xa;
            ray_y += Ya;
            if (ray_y / 64 >= 7 * 64) {
                break;
            }
            if (ray_x / 64 >= 7 * 64) {
                break;
            }
    }

    return (int) (camera.x - ray_x / cos(ray_theta));

}

void cast_rays() {
    static float ray_delta = FOV / (float) SCREEN_WIDTH;
    float ray_theta = (camera.theta - 30);
    int ray_x = 0;
    int ray_y = 0;
    int Ya = 0;
    int Xa = 0;

    for (int i = 0; i < 60; i ++) { // Cast as many rays as pixel columns
        ray_theta += i * ray_delta;

        int ray_x_dist = check_horizontal_collisions(ray_theta);
        int ray_y_dist = check_vertical_collions(ray_theta);

        printf("X: %d Y: %d\n", ray_x_dist, ray_y_dist); // debug

        if (ray_x_dist <= ray_y_dist){ // draw closer wall
            draw_wall(i, ray_x_dist);
        } else {
            draw_wall(i, ray_y_dist);
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
   // for (int y = 0; y <= SCREEN_HEIGHT; y++) {
    //    pixels[(y * SCREEN_WIDTH) + 100] = 0xFF0000FF;
    //}
    cast_rays();

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