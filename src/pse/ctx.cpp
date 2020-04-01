#include <cstdio>
#include <chrono>
#include <thread>

#include "ctx.hpp"
#include "colors.hpp"

namespace pse {

#define US_PER_S 1000000.0

context::context(const char* title, int w, int h, size_t fps)
: screen_width{w}, screen_height{h}
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    set_window(title, w, h, SDL_WINDOW_SHOWN);

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        fprintf(stderr, "Error: Failed to initialize SDL_image: %s\n", IMG_GetError());
        exit(-1);
    }

    set_frame_target(fps);
}

void context::set_window(const char *title, int w, int h, unsigned int flags)
{
    if (window) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

    screen_width = w;
    screen_height = h;

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screen_width, screen_height,
        flags
    );
    if (!window) {
        fprintf(stderr, "Error: Failed to initialize SDL Window\n");
        exit(-1);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error: Failed to initialize SDL Renderer\n");
        exit(-1);
    }
}

void context::run(void (*setup)(context& ctx), void (*update)(context& ctx), void (*cleanup)(context& ctx))
{
    srand(time(0));

    auto time_now = []() {
        return std::chrono::high_resolution_clock::now();
    };
    auto time_in_us = [](auto time) {
        return std::chrono::duration_cast<std::chrono::microseconds>(time).count();
    };
    auto sleep_us = [](auto time) {
        std::this_thread::sleep_for(std::chrono::microseconds((long long)(time)));
    };

    double frame_time = 0.0;
    auto frame_time_next = time_now();
    auto frame_time_diff = time_now() - frame_time_next;
    bool scroll_happened = false;

    setup(*this);

    while (!done) {
        scroll_happened = false;
        // keys
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                    mouse.lclick = true;
                    break;
                case SDL_BUTTON_MIDDLE:
                    mouse.mclick = true;
                    break;
                case SDL_BUTTON_RIGHT:
                    mouse.rclick = true;
                    break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                    mouse.lclick = false;
                    break;
                case SDL_BUTTON_MIDDLE:
                    mouse.mclick = false;
                    break;
                case SDL_BUTTON_RIGHT:
                    mouse.rclick = false;
                    break;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0) {
                    mouse.scrollup = true;
                    mouse.scrolldown = false;
                }
                else if (event.wheel.y < 0) {
                    mouse.scrolldown = true;
                    mouse.scrollup = false;
                }
                scroll_happened = true;
                break;
            default:
                break;
            }
        }
        if (!scroll_happened) {
            mouse.scrollup = false;
            mouse.scrolldown = false;
        }
        SDL_GetMouseState(&mouse.x, &mouse.y);
        SDL_PumpEvents();
        keystate = (unsigned char*)SDL_GetKeyboardState(NULL);

        // drawing
        SDL_SetRenderDrawColor(renderer, Black.r, Black.g, Black.b, Black.a);
        SDL_RenderClear(renderer);
        update(*this);
        SDL_RenderPresent(renderer);

        // frame management
        frame_time_diff = time_now() - frame_time_next;
        frame_time_next = time_now();
        frame_time = time_in_us(frame_time_diff);
        if (frame_time_target - frame_time > 0) {
            sleep_us(frame_time_target - frame_time);
        }

        frame_counter = (frame_counter + 1) % frame_target;
        delta_time = frame_time / US_PER_S;
    }

    if (cleanup) {
        cleanup(*this);
    }

    return;
}

context::~context()
{
    for (auto t: textures)
        SDL_DestroyTexture(t);
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool context::check_key(int sdl_scancode)
{
    return keystate[sdl_scancode];
}

bool context::check_key_invalidate(int sdl_scancode)
{
    bool pressed = keystate[sdl_scancode];
    keystate[sdl_scancode] = 0;
    return pressed;
}

void context::quit()
{
    done = true;
}

void context::set_frame_target(size_t target)
{
    frame_target = target;
    frame_counter = 0;
    frame_time_target = 1.0 / target * US_PER_S;
}

} // pse