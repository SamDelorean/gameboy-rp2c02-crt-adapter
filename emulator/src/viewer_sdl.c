#include "bridge.h"
#include "comparison_render.h"
#include "gb_source.h"
#include "rp2c02_ext.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *argv0)
{
    fprintf(stderr,
            "usage: %s"
#ifdef GBCRT_ENABLE_SAMEBOY
            " [--rom game.gb --boot dmg_boot.bin]"
#endif
            "\n",
            argv0);
}

static int create_source(gb_source_t *source,
                         const char *rom,
                         const char *boot)
{
    if (rom) {
#ifdef GBCRT_ENABLE_SAMEBOY
        if (!boot) {
            fprintf(stderr, "--rom currently requires --boot for the SameBoy DMG source\n");
            return -1;
        }
        return gb_source_sameboy_create(source, rom, boot);
#else
        (void)boot;
        fprintf(stderr,
                "this build has no SameBoy support; enable GBCRT_ENABLE_SAMEBOY\n");
        return -1;
#endif
    }

    return gb_source_pattern_create(source);
}

static void init_demo_palette(rp2c02_ext_t *ppu)
{
    rp2c02_ext_reset(ppu);
    rp2c02_ext_write_palette(ppu, 0, 0x0f);
    rp2c02_ext_write_palette(ppu, 1, 0x09);
    rp2c02_ext_write_palette(ppu, 2, 0x19);
    rp2c02_ext_write_palette(ppu, 3, 0x29);
}

int main(int argc, char **argv)
{
    const char *rom = NULL;
    const char *boot = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--rom") == 0 && i + 1 < argc) {
            rom = argv[++i];
        }
        else if (strcmp(argv[i], "--boot") == 0 && i + 1 < argc) {
            boot = argv[++i];
        }
        else {
            usage(argv[0]);
            return 1;
        }
    }

    gb_source_t source = {0};
    if (create_source(&source, rom, boot) != 0) {
        fprintf(stderr, "failed to initialize Game Boy source\n");
        return 2;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        gb_source_destroy(&source);
        return 3;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Game Boy RP2C02 CRT Adapter — hybrid emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        COMPARISON_W,
        COMPARISON_H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    SDL_Renderer *renderer = window ? SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) : NULL;

    SDL_Texture *texture = renderer ? SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        COMPARISON_W,
        COMPARISON_H) : NULL;

    rgb8_t *canvas = calloc((size_t)COMPARISON_W * COMPARISON_H, sizeof(*canvas));

    if (!window || !renderer || !texture || !canvas) {
        fprintf(stderr, "SDL viewer initialization failed: %s\n", SDL_GetError());
        free(canvas);
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
        gb_source_destroy(&source);
        return 4;
    }

    rp2c02_ext_t ppu;
    init_demo_palette(&ppu);

    gb_source_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W];

    int running = 1;
    int paused = 0;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE ||
                    event.key.keysym.sym == SDLK_q) {
                    running = 0;
                }
                else if (event.key.keysym.sym == SDLK_SPACE) {
                    paused = !paused;
                }
            }
        }

        if (!paused) {
            if (gb_source_next_frame(&source, &frame) != 0) {
                fprintf(stderr, "source frame generation failed\n");
                break;
            }
            bridge_scale_frame(frame.shade, ext, 0u);
            comparison_render(canvas, &frame, ext, &ppu);

            char title[192];
            snprintf(title, sizeof(title),
                     "Game Boy reference | RP2C02 EXT path — %s — frame %llu",
                     source.ops && source.ops->name ? source.ops->name : "source",
                     (unsigned long long)frame.frame_number);
            SDL_SetWindowTitle(window, title);
        }

        SDL_UpdateTexture(texture, NULL, canvas,
                          COMPARISON_W * (int)sizeof(rgb8_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    free(canvas);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    gb_source_destroy(&source);
    return 0;
}
