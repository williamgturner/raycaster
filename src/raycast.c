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

observer camera = {64 * 1, 64 * 3, 68.0};

float rotate(float theta, float delta) {
    theta += delta;
    if (theta >= 360) {
        theta -= 360;
    } else if (theta < 0) {
        theta += 360;
    }

    return theta;
}

float square(float x) {
    return x * x;
}

int distance_between_points(float ray_x, float ray_y) {
    float distance = (float) sqrt(square(camera.x - ray_x) + square(camera.y - ray_y));
    return distance;
}

void draw_vert(float distance, int column) {
    printf(" DISTANCE: %f", distance);
    int height = (int) (64 / distance * 277);
    int start_pos = (SCREEN_HEIGHT - height) / 2;
    int end_pos = start_pos + height;
    //printf("HEIGHT: %d\nSTART: %d\nEND:%d\n", height,start_pos,end_pos);
    if (height >= SCREEN_HEIGHT) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            pixels[(y * SCREEN_WIDTH) + column] = 0xFF0000FF;
        }
    } else {
        for (int i = start_pos; (i < end_pos); i++) {
        //printf("pixel: %d\nheight:%d", i * SCREEN_WIDTH + column, height);
        pixels[i * SCREEN_WIDTH + column] =  0xFF0000FF;
        //printf("%d\n", i);
        }
    }
    printf(" HEIGHT: %d\n", height);
}
float check_horizontal_collisions(float ray_theta) {
    int grid_x, grid_y, ray_y, ray_x, y_delta, x_delta;

    ray_y = ((int) (camera.y / WORLD_SCALE)) * 64;
    if (ray_theta <= 180) {
        ray_y -= 1;
        y_delta = -WORLD_SCALE;
    } else {
        ray_y += 64;
        y_delta = WORLD_SCALE;
    }

    x_delta = WORLD_SCALE/tan(ray_theta);
    ray_x = (int) (camera.x + ((camera.y - ray_y)/tan(ray_theta)));
    grid_x = ray_x/WORLD_SCALE;
    grid_y = ray_y/WORLD_SCALE;

    if((grid_x + (grid_y * 6 )) > 35 || (grid_x + (grid_y * 6 )) < 0) {
            return 9999999999999;
    }
    while (map[grid_x + (grid_y * 6 )] == 0) {
        ray_x += x_delta;
        ray_y += y_delta;
        grid_x = ray_x/WORLD_SCALE;
        grid_y = ray_y/WORLD_SCALE;
        if((grid_x + (grid_y * 6 )) > 35 || (grid_x + (grid_y * 6 )) < 0) {
            return 9999999999999;
    }
    }

    return distance_between_points(ray_x, ray_y);
}

float check_vertical_collisions(float ray_theta) {
    int grid_x, grid_y, ray_y, ray_x, y_delta, x_delta;

    ray_x = ((int) (camera.x/WORLD_SCALE)) * 64;
    if (ray_theta <= 90 || ray_theta >= 270) {
        ray_x += 64;
        x_delta = WORLD_SCALE;
    } else {
        ray_x -= 1; 
        x_delta = -WORLD_SCALE;
    }

    ray_y = camera.y + ((camera.x - ray_x) * tan(ray_theta));
    y_delta = WORLD_SCALE * tan(ray_theta);

    grid_x = ray_x/WORLD_SCALE;
    grid_y = ray_y/WORLD_SCALE;

    if((grid_x + (grid_y * 6 )) > 35 || (grid_x + (grid_y * 6 )) < 0) {
            return 9999999999999;
    }

    printf("COORDS: %d", grid_x + (grid_y  * 6 ));
    while (map[grid_x + (grid_y  * 6 )] == 0) {
        ray_x += x_delta;
        ray_y += y_delta;
        grid_x = ray_x/WORLD_SCALE;
        grid_y = ray_y/WORLD_SCALE;
        if((grid_x + (grid_y * 6 )) > 35 || (grid_x + (grid_y * 6 )) < 0) {
            return 9999999999999;
    }
    }

    return distance_between_points(ray_x, ray_y);
}

void render() {
    static const float ray_delta = FOV / SCREEN_WIDTH; // angle between subsequent rays
    float ray_theta = rotate(camera.theta, -FOV / 2); // ray angle

    for (int i = 0; i < 320; i++) { // cast 320 rays
        float horizontal_distance = check_horizontal_collisions(ray_theta);
        float vertical_distance = check_vertical_collisions(ray_theta);

        printf("Theta: %f H: %f, V: %f", ray_theta, horizontal_distance, vertical_distance);
        if (horizontal_distance < vertical_distance) {
            draw_vert(horizontal_distance, i);
        } else {
            draw_vert(vertical_distance, i);
        }

        ray_theta = rotate(ray_theta, ray_delta); // increment ray angle
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
    //for (int y = 50; y <= SCREEN_HEIGHT; y++) {
    //    pixels[(y * SCREEN_WIDTH) + 50] = 0xFF0000FF;
    //}
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


    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
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
            const uint8_t *keystate = SDL_GetKeyboardState(NULL);
            if (keystate[SDL_SCANCODE_LEFT]) {
                camera.theta = rotate(camera.theta,-1.0);
            }

            if (keystate[SDL_SCANCODE_RIGHT]) {
                camera.theta = rotate(camera.theta, 1.0);
            }
        }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}