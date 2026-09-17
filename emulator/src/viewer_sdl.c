#include "adapter_palette.h"
#include "bridge.h"
#include "clock_mode.h"
#include "comparison_render.h"
#include "gb_source.h"
#include "rp2c02_ext.h"
#include "source_model.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *argv0)
{
    fprintf(stderr,
            "usage: %s [--clock stock|sync] [--source-model dmg|sgb]"
#ifdef GBCRT_ENABLE_SAMEBOY
            " [--rom game.gb --boot boot.bin]"
#endif
            "\n",
            argv0);
}

static int parse_clock_mode(const char *text, gbcrt_clock_mode_t *mode)
{
    if (strcmp(text, "stock") == 0) {
        *mode = GBCRT_CLOCK_STOCK;
        return 0;
    }
    if (strcmp(text, "sync") == 0) {
        *mode = GBCRT_CLOCK_SYNC;
        return 0;
    }
    return -1;
}

static int create_source(gb_source_t *source,
                         const char *rom,
                         const char *boot,
                         gbcrt_source_model_t source_model)
{
    if (rom) {
#ifdef GBCRT_ENABLE_SAMEBOY
        if (!boot) {
            fprintf(stderr, "--rom currently requires --boot for the SameBoy source\n");
            return -1;
        }
        return gb_source_sameboy_create_model(source, rom, boot, source_model);
#else
        (void)boot;
        (void)source_model;
        fprintf(stderr,
                "this build has no SameBoy support; enable GBCRT_ENABLE_SAMEBOY\n");
        return -1;
#endif
    }

    return gb_source_pattern_create(source);
}

static int advance_source(gb_source_t *source,
                          gb_source_frame_t *frame,
                          unsigned count)
{
    for (unsigned i = 0; i < count; ++i) {
        if (gb_source_next_frame(source, frame) != 0) return -1;
    }
    return 0;
}

static void apply_clock_mode(gbcrt_clock_scheduler_t *scheduler,
                             comparison_view_state_t *view,
                             gbcrt_clock_mode_t mode)
{
    gbcrt_clock_scheduler_set_mode(scheduler, mode);
    view->clock_mode = mode;
    view->menu_selection = (int)mode;
}

static int game_key_from_sdl(SDL_Keycode key, gb_source_key_t *out)
{
    if (!out) return 0;

    switch (key) {
    case SDLK_RIGHT:     *out = GB_SOURCE_KEY_RIGHT;  return 1;
    case SDLK_LEFT:      *out = GB_SOURCE_KEY_LEFT;   return 1;
    case SDLK_UP:        *out = GB_SOURCE_KEY_UP;     return 1;
    case SDLK_DOWN:      *out = GB_SOURCE_KEY_DOWN;   return 1;
    case SDLK_z:         *out = GB_SOURCE_KEY_A;      return 1;
    case SDLK_x:         *out = GB_SOURCE_KEY_B;      return 1;
    case SDLK_BACKSPACE: *out = GB_SOURCE_KEY_SELECT; return 1;
    case SDLK_RETURN:    *out = GB_SOURCE_KEY_START;  return 1;
    default: return 0;
    }
}

static void release_all_game_keys(gb_source_t *source)
{
    for (int key = 0; key < GB_SOURCE_KEY_COUNT; ++key) {
        (void)gb_source_set_key(source, (gb_source_key_t)key, 0);
    }
}

int main(int argc, char **argv)
{
    const char *rom = NULL;
    const char *boot = NULL;
    gbcrt_clock_mode_t clock_mode = GBCRT_CLOCK_SYNC;
    gbcrt_source_model_t source_model = GBCRT_SOURCE_MODEL_DMG;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--rom") == 0 && i + 1 < argc) {
            rom = argv[++i];
        }
        else if (strcmp(argv[i], "--boot") == 0 && i + 1 < argc) {
            boot = argv[++i];
        }
        else if (strcmp(argv[i], "--clock") == 0 && i + 1 < argc) {
            if (parse_clock_mode(argv[++i], &clock_mode) != 0) {
                usage(argv[0]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "--source-model") == 0 && i + 1 < argc) {
            if (gbcrt_source_model_parse(argv[++i], &source_model) != 0) {
                usage(argv[0]);
                return 1;
            }
        }
        else {
            usage(argv[0]);
            return 1;
        }
    }

    gb_source_t source = {0};
    if (create_source(&source, rom, boot, source_model) != 0) {
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
    if (window && !renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }

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
    rp2c02_ext_reset(&ppu);
    unsigned palette_index = 0u;
    int palette_pending = -1;
    adapter_palette_apply(&ppu, palette_index);
    const adapter_palette_preset_t *palette = adapter_palette_get(palette_index);

    gb_source_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    if (gb_source_next_frame(&source, &frame) != 0) {
        fprintf(stderr, "initial source frame generation failed\n");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        gb_source_destroy(&source);
        free(canvas);
        return 5;
    }

    uint8_t ext[PPU_ACTIVE_H][PPU_ACTIVE_W];

    gbcrt_clock_scheduler_t scheduler;
    gbcrt_clock_scheduler_init(&scheduler, clock_mode, source_model);
    /* The preloaded source frame is output frame/source frame #1. */
    scheduler.output_frames = 1;
    scheduler.source_frames = 1;

    comparison_view_state_t view = {
        .clock_mode = clock_mode,
        .source_model = source_model,
        .menu_open = 0,
        .menu_selection = (int)clock_mode,
        .paused = 0,
        .source_name = source.ops && source.ops->name ? source.ops->name : "source",
        .palette_name = palette ? palette->name : "unknown",
    };

    int running = 1;
    int first_present = 1;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            else if (event.type == SDL_WINDOWEVENT &&
                     event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                release_all_game_keys(&source);
            }
            else if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;

                if (event.key.repeat) continue;

                if (key == SDLK_q) {
                    running = 0;
                }
                else if (key == SDLK_ESCAPE) {
                    if (view.menu_open) view.menu_open = 0;
                    else running = 0;
                }
                else if (key == SDLK_SPACE) {
                    view.paused = !view.paused;
                }
                else if (key == SDLK_m) {
                    view.menu_open = !view.menu_open;
                    view.menu_selection = (int)view.clock_mode;
                    if (view.menu_open) release_all_game_keys(&source);
                }
                else if (key == SDLK_c) {
                    const gbcrt_clock_mode_t next =
                        view.clock_mode == GBCRT_CLOCK_STOCK ?
                        GBCRT_CLOCK_SYNC : GBCRT_CLOCK_STOCK;
                    apply_clock_mode(&scheduler, &view, next);
                }
                else if (key == SDLK_p) {
                    palette_pending = (int)((palette_index + 1u) % adapter_palette_count());
                }
                else if (view.menu_open &&
                         (key == SDLK_UP || key == SDLK_DOWN ||
                          key == SDLK_1 || key == SDLK_2)) {
                    if (key == SDLK_1) view.menu_selection = GBCRT_CLOCK_STOCK;
                    else if (key == SDLK_2) view.menu_selection = GBCRT_CLOCK_SYNC;
                    else view.menu_selection = !view.menu_selection;
                }
                else if (view.menu_open &&
                         (key == SDLK_RETURN || key == SDLK_KP_ENTER)) {
                    apply_clock_mode(&scheduler,
                                     &view,
                                     view.menu_selection ?
                                     GBCRT_CLOCK_SYNC : GBCRT_CLOCK_STOCK);
                    view.menu_open = 0;
                }
                else if (!view.menu_open) {
                    gb_source_key_t game_key;
                    if (game_key_from_sdl(key, &game_key)) {
                        (void)gb_source_set_key(&source, game_key, 1);
                    }
                }
            }
            else if (event.type == SDL_KEYUP) {
                gb_source_key_t game_key;
                if (game_key_from_sdl(event.key.keysym.sym, &game_key)) {
                    (void)gb_source_set_key(&source, game_key, 0);
                }
            }
        }

        if (!running) break;

        /*
         * Apply requested palette changes once at the comparison-frame
         * boundary. This is the virtual-bench counterpart of committing PPU
         * palette writes during the hardware VBlank-safe interval.
         */
        if (palette_pending >= 0) {
            palette_index = (unsigned)palette_pending;
            palette_pending = -1;
            adapter_palette_apply(&ppu, palette_index);
            palette = adapter_palette_get(palette_index);
            view.palette_name = palette ? palette->name : "unknown";
        }

        if (!view.paused && !first_present) {
            const unsigned advances = gbcrt_clock_scheduler_step(&scheduler);
            if (advance_source(&source, &frame, advances) != 0) {
                fprintf(stderr, "source frame generation failed\n");
                break;
            }
        }
        first_present = 0;

        bridge_scale_frame(frame.shade, ext, BRIDGE_BORDER_EXT_INDEX);
        comparison_render(canvas, &frame, ext, &ppu, &view);

        char title[360];
        snprintf(title, sizeof(title),
                 "GB reference | RP2C02 EXT — %s/%s — %s — %s — GB frame %llu — output %llu — repeats %llu — skipped %llu",
                 view.source_name,
                 gbcrt_source_model_name(view.source_model),
                 gbcrt_clock_mode_name(view.clock_mode),
                 view.palette_name ? view.palette_name : "palette",
                 (unsigned long long)frame.frame_number,
                 scheduler.output_frames,
                 scheduler.repeated_output_frames,
                 scheduler.skipped_source_frames);
        SDL_SetWindowTitle(window, title);

        SDL_UpdateTexture(texture, NULL, canvas,
                          COMPARISON_W * (int)sizeof(rgb8_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    release_all_game_keys(&source);

    printf("viewer stopped: model=%s mode=%s palette=%s output=%llu source=%llu repeats=%llu skipped=%llu\n",
           gbcrt_source_model_name(view.source_model),
           gbcrt_clock_mode_name(view.clock_mode),
           view.palette_name ? view.palette_name : "unknown",
           scheduler.output_frames,
           scheduler.source_frames,
           scheduler.repeated_output_frames,
           scheduler.skipped_source_frames);

    free(canvas);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    gb_source_destroy(&source);
    return 0;
}
