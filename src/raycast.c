#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define WORLD_SCALE 64
#define MAP_WIDTH 7
#define MAP_HEIGHT 6
#define FOV 60.0

SDL_Window *window;
SDL_Renderer *renderer;
uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
int quit = 0;

uint8_t map[SCREEN_HEIGHT * SCREEN_WIDTH] = {
    1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1,
};

typedef struct {
    float x;
    float y;
    float theta;
} observer;

observer camera = {4 * WORLD_SCALE, 3 * WORLD_SCALE, 45.0};

float square(float x) {
    return x * x;
}

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
    for (int i = x; i < x + size; i++) {
        for (int j = 0; j < size; j++) {
            pixels[(y + j) * SCREEN_WIDTH + i] = colour; // green
        }
    }
}

void draw_wall (int column, float distance) {
    int height =  64.0f / distance * 277;
    int top = 120 + (height / 2);
    if (height > 240) {
        height = 240;
    }
    for (int i = (SCREEN_HEIGHT - height) / 2; i < top; i++) {
        pixels[i * SCREEN_WIDTH + column] = 0xFF0000FF;
    }
}

// Implementation of Bresenham's line algorithim
void generate_line_points(int startX, int startY, int endX, int endY) {
    int dx = endX - startX;
    int dy = endY - startY;
    int absdx = (int) (dx);
    int absdy = (int) (dy);

    int x = startX;
    int y = startY;
    pixels[y * SCREEN_WIDTH + x] = 0xFFFFFF;

    // slope < 1
    if (absdx > absdy) {

        int d = 2*absdy - absdx;

        for(int i = 0; i < absdx; i++) {
            x = dx < 0 ? x-1: x+1;
            if(d < 0) {
                d = d + 2*absdy;
            } else {
                y = dy < 0 ? y-1 : y+1;
                d = d + ( 2*absdy - 2*absdx); 
            }
            if (y * SCREEN_WIDTH + x < SCREEN_WIDTH * SCREEN_HEIGHT) {
                pixels[y * SCREEN_WIDTH + x] = 0xFFFFFF;
            }
        }
    } else { // case when slope is greater than or equals to 1
        int d = 2*absdx - absdy;

        for(int i = 0; i < absdy ; i++)
        {
            y = dy < 0 ? y-1 : y + 1;
            if(d < 0)
                d = d + 2*absdx;
            else
            {
                x = dx < 0 ? x-1 : x + 1;
                d = d + (2*absdx) - (2*absdy);
            }
            if (y * SCREEN_WIDTH + x < SCREEN_WIDTH * SCREEN_HEIGHT) {
                pixels[y * SCREEN_WIDTH + x] = 0xFFFFFF;
            }
        }
    }
}

void render_minimap()
{
    // bottom left = SCREEN_WDITH * SCREEN_HEIGHT - 25 * 6 - (25*6*SCREEN_WIDTH)
    // printf("Test");
    static const int CELL_SIZE = 5;
    for (int row = 0; row < MAP_HEIGHT; row ++) {
        for (int column = 0; column < MAP_WIDTH; column++) {
            if (map[row * MAP_WIDTH + column] != 0){
                draw_square(SCREEN_WIDTH - (CELL_SIZE * MAP_WIDTH) + column * CELL_SIZE, SCREEN_HEIGHT - CELL_SIZE - row * CELL_SIZE, CELL_SIZE, 0x00FF00);
            }
            // printf("X: %d | Y: %d\n", SCREEN_WIDTH - (10 * 6) + column * 10, SCREEN_HEIGHT - 10 - row * 10);
        }
    }

    // camera pos in minimap scale
    float mini_x = camera.x / 64 * CELL_SIZE;
    float mini_y = camera.y / 64 * CELL_SIZE;
    // draw camera pos
    draw_square((SCREEN_WIDTH - (CELL_SIZE * MAP_WIDTH)) + mini_x, SCREEN_HEIGHT - mini_y, 2, 0xFFFFFF);
    mini_x = (SCREEN_WIDTH - (CELL_SIZE * MAP_WIDTH)) + mini_x;
    mini_y = SCREEN_HEIGHT - mini_y;
    // draw angle of camera
    int vision_end_x = mini_x + 20 * cos((camera.theta) * M_PI / 180);
    int vision_end_y = mini_y + 20 * sin(camera.theta  * M_PI / 180);

    generate_line_points(mini_x, mini_y, vision_end_x, vision_end_y);

}
int distance_between_points(float ray_x, float ray_y) {
    float distance = (float) sqrt(square(camera.x - ray_x) + square(camera.y - ray_y));
    return distance;
}

void draw_vert(float distance, int column) {
    //printf(" DISTANCE: %f", distance);
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
    //printf(" HEIGHT: %d\n", height);
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

    if((grid_x + (grid_y * 7 )) > 42 || (grid_x + (grid_y * 7 )) < 0) {
            return 1e30;
    }
    while (map[grid_x + (grid_y * 7 )] == 0) {
        ray_x += x_delta;
        ray_y += y_delta;
        grid_x = ray_x/WORLD_SCALE;
        grid_y = ray_y/WORLD_SCALE;
        if((grid_x + (grid_y * 7 )) > 42 || (grid_x + (grid_y * 7 )) < 0) {
            return 1e30;
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

    if((grid_x + (grid_y * 7 )) > 42 || (grid_x + (grid_y * 7 )) < 0) {
            return 1e30;
    }

    //printf("COORDS: %d", grid_x + (grid_y  * 6 ));
    while (map[grid_x + (grid_y  * 7 )] == 0) {
        ray_x += x_delta;
        ray_y += y_delta;
        grid_x = ray_x/WORLD_SCALE;
        grid_y = ray_y/WORLD_SCALE;
        if((grid_x + (grid_y * 7 )) > 42 || (grid_x + (grid_y * 7 )) < 0) {
            return 1e30;
    }
    }

    return distance_between_points(ray_x, ray_y);
}

void render() {
    static const float ray_delta = FOV / SCREEN_WIDTH; // angle between subsequent rays
    float ray_theta = rotate(camera.theta, -FOV / 2); // ray angle

    for (int i = 0; i < 320; i++) { // cast 320 rays
        float horizontal_distance = check_horizontal_collisions(ray_theta * (M_PI/180));
        float vertical_distance = check_vertical_collisions(ray_theta * (M_PI/180));

        //printf("Theta: %f H: %f, V: %f", ray_theta, horizontal_distance, vertical_distance);
        if (horizontal_distance < vertical_distance) {
            draw_vert(horizontal_distance, i);
        } else {
            draw_vert(vertical_distance, i);
        }

        ray_theta = rotate(ray_theta, ray_delta); // increment ray angle
    }
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
        render();
        render_minimap();
        //generate_line_points(10, 10, 50, 30);
        SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * 4);
                TTF_Init();
                //this opens a font style and sets a size
                TTF_Font* Mono = TTF_OpenFont("src/Fixedsys62.ttf", 48);
                if (Mono == NULL) {
                    printf("Failed to load font: %s\n", TTF_GetError());
                    return 1;
                }
                // maxing out all would give you the color white,
                // and it will be your text's color
                SDL_Color White = {255, 255, 255};

                // as TTF_RenderText_Solid could only be used on
                // SDL_Surface then you have to create the surface first
                char buffer[50];
                sprintf(buffer, "X: %f | Y: %f | Theta: %f", camera.x, camera.y, camera.theta);
                SDL_Surface* surfaceMessage =
                    TTF_RenderText_Solid(Mono, buffer, White); 

                // now you can convert it into a texture
                SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

                SDL_Rect Message_rect; //create a rect
                Message_rect.x = 0;  //controls the rect's x coordinate 
                Message_rect.y = 900; // controls the rect's y coordinte
                Message_rect.w = 600; // controls the width of the rect
                Message_rect.h = 35; // controls the height of the rect
                        
                        
        SDL_RenderCopyEx(
            renderer,
            texture,
            NULL,
            NULL,
            0.0,
            NULL,
            SDL_FLIP_VERTICAL);
        SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
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

        if (keystate[SDL_SCANCODE_Q]) {
            camera.theta = rotate(camera.theta, 5);
        }

        if (keystate[SDL_SCANCODE_E]) {
            camera.theta = rotate(camera.theta, -5);
        }
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(Message);
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
