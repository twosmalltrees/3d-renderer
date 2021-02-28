#include <SDL2/SDL.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
bool is_running = false;

uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;

int window_width = 800;
int window_height = 600;

int *renderer_output_width = NULL;
int *renderer_output_height = NULL;

typedef struct {
    int x;
    int y;
} point_xy;

bool initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    };

    // Use SDL to query full screen maximum width or height
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);

    window_width = display_mode.w;
    window_height = display_mode.h;

    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    int fullscreen = SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (fullscreen != 0) {
        printf("Window could not fullscreen");
        return false;
    }

    return true;
}

void setup(void) {
    renderer_output_width = malloc(sizeof(uint32_t));
    renderer_output_height = malloc(sizeof(uint32_t));
    SDL_GetRendererOutputSize(renderer, renderer_output_width, renderer_output_height);

    color_buffer = malloc(sizeof(uint32_t) * *renderer_output_width * *renderer_output_height);

    printf("Output width: %i", *renderer_output_width);

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        *renderer_output_width,
        *renderer_output_height);
}

void processInput(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            break;
    }
}

void update(void) {
    // TODO
}

void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < *renderer_output_height; ++y) {
        for (int x = 0; x < *renderer_output_width; ++x) {
            int position = (y * *renderer_output_width) + x;
            color_buffer[position] = color;
        }
    }
}

void draw_pixel(point_xy pixel, uint32_t color) {
    int position = (pixel.y * *renderer_output_width) + pixel.x;
    color_buffer[position] = color;
}

void draw_grid(uint32_t grid_color, uint32_t grid_spacing) {
    for (int y = 0; y < *renderer_output_height; y += grid_spacing) {
        for (int x = 0; x < *renderer_output_width; x += grid_spacing) {
            point_xy pixel = {x, y};
            draw_pixel(pixel, grid_color);
        }
    }
}

int y_given_x(int x, float gradient, point_xy point) {
    return round(gradient * (x - point.x) + point.y);
};

int x_given_y(int y, float gradient, point_xy point) {
    return round((y - point.y) / gradient + point.x);
};

void draw_line(point_xy first_point, point_xy second_point, uint32_t color) {
    if (first_point.x == second_point.x && first_point.y == second_point.y) {
        // Is a point, not a line. Draw a point.
        point_xy pixel_position = {first_point.x, first_point.y};
        draw_pixel(pixel_position, color);
        return;
    }

    if (first_point.x == second_point.x) {
        // Is a vertical line; Need to handle this case first, as gradient formula won't handle
        // division by zero.
        if (first_point.y < second_point.y) {
            // Line direction is down.
            for (int y = first_point.y; y <= second_point.y; y++) {
                point_xy pixel_position = {first_point.x, y};
                draw_pixel(pixel_position, color);
            }
            return;
        } else {
            // Line direction is up.
            for (int y = first_point.y; y >= second_point.y; y--) {
                point_xy pixel_position = {first_point.x, y};
                draw_pixel(pixel_position, color);
            }
            return;
        }
    }

    float gradient = (float)(first_point.y - second_point.y) / (first_point.x - second_point.x);

    // Is a horizontal line
    if (gradient == 0) {
        if (first_point.x < second_point.x) {
            // Line direction is left -> right.
            for (int x = first_point.x; x <= second_point.x; x++) {
                point_xy pixel_position = {x, first_point.y};
                draw_pixel(pixel_position, color);
            }
            return;
        } else {
            // Line direction is right -> left.
            for (int x = first_point.x; x >= second_point.x; x--) {
                point_xy pixel_position = {x, first_point.y};
                draw_pixel(pixel_position, color);
            }
            return;
        }
    };

    if (fabs(gradient) <= 1) {
        // Iterate over X values (as may produce multiple pixels on same Y coord)
        if (first_point.x < second_point.x) {
            for (int x = first_point.x; x <= second_point.x; x++) {
                int y = y_given_x(x, gradient, first_point);
                point_xy pixel_position = {x, y};
                draw_pixel(pixel_position, color);
            }
            return;
        } else {
            for (int x = first_point.x; x >= second_point.x; x--) {
                int y = y_given_x(x, gradient, first_point);
                point_xy pixel_position = {x, y};
                draw_pixel(pixel_position, color);
            }
            return;
        }
    } else {
        // Iterate over Y values (as may produce multiple pixels on same X coord)
        if (first_point.y < second_point.y) {
            for (int y = first_point.y; y <= second_point.y; y++) {
                int x = x_given_y(y, gradient, first_point);
                point_xy pixel_position = {x, y};
                draw_pixel(pixel_position, color);
            }
            return;
        } else {
            for (int y = first_point.y; y >= second_point.y; y--) {
                int x = x_given_y(y, gradient, first_point);
                point_xy pixel_position = {x, y};
                draw_pixel(pixel_position, color);
            }
            return;
        }
    }
};

void render_color_buffer(void) {
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (sizeof(uint32_t)) * *renderer_output_width);
    SDL_RenderCopy(
        renderer,
        color_buffer_texture,
        NULL,
        NULL);
}

void draw_triangle(point_xy point_one, point_xy point_two, point_xy point_three, uint32_t color) {
    draw_line(point_one, point_two, color);
    draw_line(point_two, point_three, color);
    draw_line(point_three, point_one, color);
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 20, 60, 120, 255);
    SDL_RenderClear(renderer);

    render_color_buffer();
    clear_color_buffer(0xFFFFFFFF);
    draw_grid(0xFF000000, 50);

    point_xy point_one = {0, 0};
    point_xy point_two = {0, 100};
    point_xy point_three = {200, 100};

    draw_triangle(point_one, point_two, point_three, 0xFF000000);

    point_xy point_four = {100, 600};
    point_xy point_five = {200, 300};
    point_xy point_six = {900, 200};

    draw_triangle(point_four, point_five, point_six, 0xFF000000);

    SDL_RenderPresent(renderer);
}

void destroy_window(void) {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(void) {
    is_running = initialize_window();
    setup();
    while (is_running) {
        processInput();
        update();
        render();
    }

    destroy_window();

    return 0;
}