#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FOV 60.0
#define WORLD_SCALE 64
#define MAP_WIDTH 15
#define MAP_HEIGHT 13
#define TO_RADIANS (M_PI/180)

SDL_Window *window;
SDL_Renderer *renderer;
uint32_t pixels[SCREEN_WIDTH* SCREEN_HEIGHT];
int quit = 0;

typedef struct {
    double x;
    double y;
    double theta;
} camera;

int8_t map[MAP_HEIGHT * MAP_WIDTH] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

camera cam = {12 * WORLD_SCALE, 6 * WORLD_SCALE, 0.0};

/**
 * Rotate an angle in degrees by given angle
 *
 * Used to clamp degrees such that 0 < Theta <= 360
 * @param float theta angle (in degrees) to be rotated
 * @param float delta angle (in degrees) to increment by
 * @return float theta result of rotation
 */
 void rotate(double* theta, double delta)
{
    *theta += delta;
    if (*theta < 0) {
        *theta += 360;
    } else if (*theta >= 360) {
        *theta -= 360;
    }
}

/** Calculate distance between to coordinates
 * @param x0
 * @param y0
 * @param x1
 * @param y1
 * @return distance
 */
double distance_between_points(double x0, double y0, double x1, double y1)
{
    return floor(sqrt(pow(x0-x1, 2) + pow(y0-y1, 2)));
}
/**
* Draws a vertical line to the pixel buffer
* @param y0 start y ord for pixel buffer
* @param y1 end y ord for pixel buffer
* @param x ord for pixel buffer
* @param colour to be drawn
* */
void draw_vert(int y0, int y1, int x, int colour) {
    for (int i = y0; i < y1; i++) {
        if (i * SCREEN_WIDTH + x < SCREEN_WIDTH * SCREEN_HEIGHT) {

            pixels[i * SCREEN_WIDTH + x] =  colour;
        }
    }
}

/**
 * Cast ray to find nearest wall
 * @return distance the distance in units to nearest wall
 */
double cast_ray(double ray_theta)
{
    double distance;

        // if ray is facing RIGHT (checking horizontals)
        double ray_x;
        double ray_y;
        double x_delta;
        double y_delta;
        
    if (fabs(ray_theta - 90) <= 0.00001 || fabs(ray_theta - 270) <= 0.00001) {
        return DBL_MAX;
    }
        if (ray_theta < 90 || ray_theta > 270) {
             ray_x = floor(cam.x/WORLD_SCALE) * WORLD_SCALE + WORLD_SCALE;
             x_delta = WORLD_SCALE;
             ray_y = cam.y + tan(ray_theta * TO_RADIANS) * (ray_x - cam.x);
             y_delta = tan(ray_theta * TO_RADIANS) * WORLD_SCALE;
        } else {
             ray_x = floor(cam.x/WORLD_SCALE) * WORLD_SCALE - 1;
             x_delta = -WORLD_SCALE;
             ray_y = cam.y + tan(ray_theta * TO_RADIANS) * fabs(ray_x - cam.x);
             y_delta = tan(ray_theta * TO_RADIANS) * WORLD_SCALE;
        }
        
        int grid_x = floor(ray_x / WORLD_SCALE);
        int grid_y = floor(ray_y / WORLD_SCALE);
        while (grid_x >= 0 && grid_x < MAP_WIDTH && grid_y >= 0 && grid_y < MAP_HEIGHT) {
            if (map[grid_x + (grid_y * MAP_WIDTH)] != 0) {
                return distance_between_points(cam.x,  cam.y, ray_x, ray_y);
            }
            ray_x += x_delta;
            ray_y += y_delta;
            grid_x = floor(ray_x / WORLD_SCALE);
            grid_y = floor(ray_y / WORLD_SCALE);
            
        }
    return DBL_MAX;
}

double cast_ray_vertical(double ray_theta) {
    double ray_x;
    double ray_y;
    double x_delta;
    double y_delta;
     if (fabs(ray_theta - 180) <= 0.00001 || fabs(ray_theta) <= 0.00001) { // ignore rays straight left or right
       return DBL_MAX;
    }


    if (ray_theta < 180) { // if facing UP
        y_delta = -WORLD_SCALE; // move up by 1 tile
        ray_y = floor(cam.y / WORLD_SCALE) * WORLD_SCALE -1; // start at bottom of current tile
        // ray_x = cam.y - ray_y / tan(ray_theta * TO_RADIANS);
        ray_x = cam.x + (cam.y - ray_y) / tan(ray_theta * TO_RADIANS);
        x_delta = 64 / tan(ray_theta * TO_RADIANS);
    } else { // iffacing DOWN
        y_delta = WORLD_SCALE;
        ray_y = floor(cam.y / WORLD_SCALE) * WORLD_SCALE + WORLD_SCALE;
        ray_x = cam.x + fabs(cam.y - ray_y) / tan(ray_theta * TO_RADIANS);
        x_delta = WORLD_SCALE / tan(ray_theta * TO_RADIANS);
    }
 int grid_x = floor(ray_x / WORLD_SCALE);
        int grid_y = floor(ray_y / WORLD_SCALE);
        while (grid_x >= 0 && grid_x < MAP_WIDTH && grid_y >= 0 && grid_y < MAP_HEIGHT) {
            if (map[grid_x + (grid_y * MAP_WIDTH)] != 0) {
                return distance_between_points(cam.x,  cam.y, ray_x, ray_y);
            }
            ray_x += x_delta;
            ray_y += y_delta;
            grid_x = floor(ray_x / WORLD_SCALE);
            grid_y = floor(ray_y / WORLD_SCALE);
            
        }
    return DBL_MAX;
}

void render()
{
    double ray_theta = cam.theta;
    rotate(&ray_theta, -FOV / 2);
    double ray_delta = FOV / SCREEN_WIDTH;
    for (int x = 0; x < SCREEN_WIDTH; x ++) {
        double distance = cast_ray(ray_theta);
        double distance1 = cast_ray_vertical(ray_theta);
        distance = distance * cos((cam.theta - ray_theta) * TO_RADIANS);
        distance1 = distance1 * cos((cam.theta - ray_theta) * TO_RADIANS);
        int wall_height;
        int y0;
        int colour;
        if (distance < distance1 - 0.0001) {
            wall_height = WORLD_SCALE / distance * 277;
            colour = 0xFF0000;
        } else {
            wall_height = WORLD_SCALE / distance1 * 277;
            colour = 0xFFFF00;
        }
        if (wall_height > SCREEN_HEIGHT) {
            wall_height = SCREEN_HEIGHT;
        }
        if (wall_height < 0) {
            wall_height = 0;
        }
        y0 = SCREEN_HEIGHT / 2 - wall_height /2;
       draw_vert(y0, y0 + wall_height, x, colour);
       rotate(&ray_theta, ray_delta);
    }
}
int main(int argc, char *argv[]) {
    // Init SDL, create window, renderer, and texture for graphics rendering
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
//    if (!SDL_Init(SDL_INIT_VIDEO)) {
//        printf("Couldn't initialise SDL_VIDEO");
//        return 1;
//   }
    window = SDL_CreateWindow(
        "Raycast Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        480,
        SDL_WINDOW_ALLOW_HIGHDPI);

//    if(!window >= 0) {
//        printf("Couldn't initialise window");
//        return 1;  
//    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
//    if(!renderer) {
//        printf("Coudln't initialise renderer");
//        return 1;
//    }

    SDL_Texture *texture;
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);
    // init fonts
    TTF_Init();
    //this opens a font style and sets a size
    TTF_Font* Mono = TTF_OpenFont("src/Fixedsys62.ttf", 48);
    if (Mono == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }
    // and it will be your text's color
    SDL_Color White = {255, 255, 255};
    char buffer[50];
 
    double player_speed = 10;

    SDL_Rect Message_rect; //create a rect
    Message_rect.x = 0;  //controls the rect's x coordinate 
    Message_rect.y = 900; // controls the rect's y coordinte
    Message_rect.w = 600; // controls the width of the rect
    Message_rect.h = 35; // controls the height of the rect
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
        //draw_vert(0, SCREEN_HEIGHT, 10, 0xFF0000);
        SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * 4);
        sprintf(buffer, "X: %f | Y: %f | Theta: %f", cam.x, cam.y, cam.theta);
        SDL_Surface* surfaceMessage =
        TTF_RenderText_Solid(Mono, buffer, White); 

        SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_RenderCopyEx(
            renderer,
            texture,
            NULL,
            NULL,
            0.0,
            NULL,
            SDL_FLIP_NONE);
        SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
        SDL_RenderPresent(renderer);

        const uint8_t *keystate = SDL_GetKeyboardState(NULL);
        double x_vec = cos(cam.theta * TO_RADIANS);
        double y_vec = sin(cam.theta * TO_RADIANS);

        if (keystate[SDL_SCANCODE_UP]) {
            if (!(map[((int) ((cam.y + y_vec * player_speed) / WORLD_SCALE)) * MAP_WIDTH + (int) (cam.x + x_vec * player_speed / WORLD_SCALE)] != 0)) {
                    cam.x += x_vec * player_speed;
                cam.y += y_vec * player_speed;
            }
        }

        if (keystate[SDL_SCANCODE_DOWN]) {
            if (!(map[(int) ((cam.y - y_vec*player_speed / WORLD_SCALE)) * MAP_WIDTH + (int) (cam.x - x_vec * player_speed / WORLD_SCALE)] != 0)) {
                    cam.x -= x_vec * player_speed;
                    cam.y -= y_vec * player_speed;
            }
        }

        if (keystate[SDL_SCANCODE_Q]) {
            rotate(&cam.theta, 5);
        }

        if (keystate[SDL_SCANCODE_E]) {
            rotate(&cam.theta, -5);
        }
        SDL_DestroyTexture(Message);
        SDL_FreeSurface(surfaceMessage);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
