#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_opengles2.h>
#include <stdio.h>

#define TINYGP_IMPLEMENTATION
#define TGPGL_IMPLEMENTATION
#include "tinygp.h"
#include "tinygp_gl.h"

void draw(tgp_context* ctx, int width, int height) {
    tgp_begin(ctx, width, height);
    tgp_project(ctx, 0, (float)width, 0, (float)height);
    tgp_set_color(ctx, 0.2f, 0.2f, 0.2f, 1.0f);
    tgp_clear(ctx);
    // ctx->antialiasing = false;

    tgp_vec2 points[3] = {
        {128.0f, 256.0f},
        {256.0f, 128.0f},
        {512.0f, 350.0f},
    };
    tgp_vec2 points2[6] = {
        {128.0f * 4.0f, 256.0f * 4.0f},
        {256.0f * 4.0f, 128.0f * 4.0f},
        {512.0f * 4.0f, 350.0f * 4.0f},
        {128.0f * 2.0f, 256.0f * 2.0f},
        {256.0f * 2.0f, 128.0f * 2.0f},
        {512.0f * 2.0f, 350.0f * 2.0f},
    };
    tgp_set_color(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
    tgp_draw_convex_polygon(ctx, (const tgp_vec2*)&points, 3);
    tgp_draw_convex_polygon(ctx, (const tgp_vec2*)&points2, 6);
    // tgp_set_color(ctx, 1.0f, 1.0f, 0.5f, 1.0f);
    // tgp_draw_convex_polygon(ctx, (const tgp_vec2*)&points, 3);
}

int main(void) {
    // setup SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "failed to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    // enable native IME
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_Window* window = SDL_CreateWindow(
        "tinygp example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GLContext glc = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glc);
    SDL_GL_SetSwapInterval(1); // enable vsync

    // initialize tinygp context
    tgp_context ctx;
    tgp_options opts = tgp_default_options();
    tgp_init_context(&ctx, &opts);

    // initialize tinygp_gl context
    tgpgl_context tgpgl_ctx;
    tgpgl_init_context(&tgpgl_ctx, &ctx);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)) {
                running = false;
            }
        }

        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);

        // draw the frame
        draw(&ctx, width, height);

        // render
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        tgpgl_render(&tgpgl_ctx);
        SDL_GL_SwapWindow(window);
    }

    // cleanup
    // tgp_destroy_context(&ctx);
    // tgpgl_destroy_context(&tgpgl_ctx);
    SDL_GL_DeleteContext(glc);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
