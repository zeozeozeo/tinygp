#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_opengles2.h>
#include <stdio.h>

#define TINYGP_IMPLEMENTATION
#define TGPGL_IMPLEMENTATION
#include "tinygp.h"
#include "tinygp_gl.h"

// #define AUDIO
#ifdef AUDIO
uint64_t t = 0;

void audio_callback(void* userdata, uint8_t* stream, int len) {
    // https://www.pouet.net/topic.php?which=8357&page=8#c388898
    for (int i = 0; i < len; i++) {
        stream[i] = ((t >> 1) * (15 & 0x234568a0 >> (t >> 8 & 28)) |
                     t >> 1 >> (t >> 11) ^ t >> 12) +
                    (t >> 4 & t & 24);
        stream[i] /= 64;
        t++;
    }
}
#endif

void draw(tgp_context* ctx, int width, int height) {
    tgp_begin(ctx, width, height);
    tgp_project(ctx, 0, (float)width, 0, (float)height);
    tgp_set_color(ctx, 0.2, 0.2, 0.2, 1.0);
    tgp_clear(ctx);

    tgp_vec2 points[3] = {
        {256, 128},
        {128, 256},
        {512, 350}
    };
    tgp_set_color(ctx, 1.0, 1.0, 1.0, 1.0);
    // ctx->antialiasing = false;
    tgp_draw_convex_polygon(ctx, (const tgp_vec2*)&points, 3);
}

int main(void) {
    // setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "failed to initialize SDL: %s\n", SDL_GetError());
        exit(-1);
    }

#ifdef AUDIO
    // setup audio
    SDL_AudioSpec     want = {}, have;
    SDL_AudioDeviceID dev;
    want.freq = 44100;
    want.format = AUDIO_U8;
    want.channels = 1;
    want.samples = 4096;
    want.callback = audio_callback;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                              SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
#endif

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

#ifdef AUDIO
    // unpause the audio device
    SDL_PauseAudioDevice(dev, 0);
#endif

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
#ifdef AUDIO
    SDL_CloseAudioDevice(dev);
#endif
    SDL_GL_DeleteContext(glc);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
