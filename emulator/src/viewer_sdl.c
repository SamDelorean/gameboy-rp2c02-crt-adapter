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

static void request_next_palette(unsigned palette_mode, int *palette_pending)
{
    if (!palette_pending) return;
    *palette_pending =
        (int)((palette_mode + 1u) % (adapter_palette_count() + 1u));
}

static void copy_palette_codes(const rp2c02_ext_t *ppu, uint8_t codes[4])
{
    for (unsigned shade = 0; shade < 4u; ++shade) {
        codes[shade] = rp2c02_ext_palette_code(ppu, (uint8_t)shade);
    }
}

static void apply_editor_palette(rp2c02_ext_t *ppu,
                                 comparison_view_state_t *view,
                                 int *custom_palette_active,
                                 char *label,
                                 size_t label_size)
{
    adapter_palette_apply_codes(ppu, view->palette_editor_codes);
    if (custom_palette_active) *custom_palette_active = 1;
    snprintf(label, label_size,
             "CUSTOM %02X %02X %02X %02X",
             view->palette_editor_codes[0],
             view->palette_editor_codes[1],
             view->palette_editor_codes[2],
             view->palette_editor_codes[3]);
}

static void load_editor_preset(comparison_view_state_t *view,
                               unsigned preset_index,
                               uint8_t reset_codes[4])
{
    const adapter_palette_preset_t *preset = adapter_palette_get(preset_index);
    if (!preset) return;
    memcpy(view->palette_editor_codes, preset->shade_code,
           sizeof(view->palette_editor_codes));
    memcpy(reset_codes, preset->shade_code, 4u);
}

static int mouse_to_canvas(SDL_Window *window,
                           int mouse_x,
                           int mouse_y,
                           unsigned *canvas_x,
                           unsigned *canvas_y)
{
    int window_w = 0;
    int window_h = 0;
    SDL_GetWindowSize(window, &window_w, &window_h);
    if (window_w <= 0 || window_h <= 0 || mouse_x < 0 || mouse_y < 0) return 0;

    *canvas_x = (unsigned)((unsigned long long)mouse_x * COMPARISON_W /
                           (unsigned)window_w);
    *canvas_y = (unsigned)((unsigned long long)mouse_y * COMPARISON_H /
                           (unsigned)window_h);
    return 1;
}

/* palette_mode 0 = AUTO/SGB, 1..N = manual presets. */
static void apply_palette_mode(rp2c02_ext_t *ppu,
                               const gb_source_frame_t *frame,
                               unsigned palette_mode,
                               uint64_t *applied_sgb_sequence,
                               char *label,
                               size_t label_size)
{
    if (palette_mode == 0u) {
        if (frame->sgb_palette_valid) {
            adapter_palette_apply_sgb_rgb555(ppu, frame->sgb_palette_rgb555);
            *applied_sgb_sequence = frame->sgb_palette_sequence;
            snprintf(label, label_size, "AUTO/SGB");
        }
        else {
            const adapter_palette_preset_t *fallback = adapter_palette_get(0u);
            adapter_palette_apply(ppu, 0u);
            *applied_sgb_sequence = 0u;
            snprintf(label, label_size,
                     "AUTO/SGB FALLBACK %s",
                     fallback ? fallback->name : "PALETTE");
        }
        return;
    }

    const unsigned manual_index = palette_mode - 1u;
    const adapter_palette_preset_t *preset = adapter_palette_get(manual_index);
    adapter_palette_apply(ppu, manual_index);
    snprintf(label, label_size, "%s", preset ? preset->name : "MANUAL");
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

    rp2c02_ext_t ppu;
    rp2c02_ext_reset(&ppu);
    unsigned palette_mode = 0u; /* AUTO/SGB by default, matching hardware policy. */
    int palette_pending = -1;
    uint64_t applied_sgb_sequence = 0u;
    int custom_palette_active = 0;
    unsigned editor_preset_index = 0u;
    uint8_t editor_reset_codes[4] = {0};
    char palette_label[96];
    apply_palette_mode(&ppu,
                       &frame,
                       palette_mode,
                       &applied_sgb_sequence,
                       palette_label,
                       sizeof(palette_label));

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
        .palette_editor_open = 0,
        .palette_editor_selected_shade = 0u,
        .palette_editor_codes = {0u, 0u, 0u, 0u},
        .source_name = source.ops && source.ops->name ? source.ops->name : "source",
        .palette_name = palette_label,
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
            else if (event.type == SDL_MOUSEBUTTONDOWN &&
                     event.button.button == SDL_BUTTON_LEFT) {
                unsigned x = 0;
                unsigned y = 0;
                if (!mouse_to_canvas(window,
                                     event.button.x,
                                     event.button.y,
                                     &x,
                                     &y)) {
                    continue;
                }

                if (view.palette_editor_open) {
                    unsigned shade = 0u;
                    uint8_t code = 0u;
                    if (comparison_palette_editor_shade_at(x, y, &shade)) {
                        view.palette_editor_selected_shade = shade;
                    }
                    else if (comparison_palette_editor_code_at(x, y, &code)) {
                        view.palette_editor_codes[view.palette_editor_selected_shade & 3u] = code;
                        apply_editor_palette(&ppu, &view, &custom_palette_active,
                                             palette_label, sizeof(palette_label));
                    }
                    else {
                        const comparison_palette_editor_action_t action =
                            comparison_palette_editor_action_at(x, y);
                        if (action == COMPARISON_EDITOR_ACTION_REVERSE) {
                            uint8_t t = view.palette_editor_codes[0];
                            view.palette_editor_codes[0] = view.palette_editor_codes[3];
                            view.palette_editor_codes[3] = t;
                            t = view.palette_editor_codes[1];
                            view.palette_editor_codes[1] = view.palette_editor_codes[2];
                            view.palette_editor_codes[2] = t;
                            apply_editor_palette(&ppu, &view, &custom_palette_active,
                                                 palette_label, sizeof(palette_label));
                        }
                        else if (action == COMPARISON_EDITOR_ACTION_RESET) {
                            memcpy(view.palette_editor_codes, editor_reset_codes, 4u);
                            apply_editor_palette(&ppu, &view, &custom_palette_active,
                                                 palette_label, sizeof(palette_label));
                        }
                        else if (action == COMPARISON_EDITOR_ACTION_PREV ||
                                 action == COMPARISON_EDITOR_ACTION_NEXT) {
                            const unsigned count = adapter_palette_count();
                            if (count) {
                                if (action == COMPARISON_EDITOR_ACTION_PREV) {
                                    editor_preset_index =
                                        (editor_preset_index + count - 1u) % count;
                                }
                                else {
                                    editor_preset_index =
                                        (editor_preset_index + 1u) % count;
                                }
                                load_editor_preset(&view, editor_preset_index,
                                                   editor_reset_codes);
                                apply_editor_palette(&ppu, &view, &custom_palette_active,
                                                     palette_label, sizeof(palette_label));
                            }
                        }
                        else if (action == COMPARISON_EDITOR_ACTION_DONE) {
                            view.palette_editor_open = 0;
                        }
                    }
                }
                else if (comparison_palette_button_contains(x, y)) {
                    custom_palette_active = 0;
                    request_next_palette(palette_mode, &palette_pending);
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;

                if (event.key.repeat) continue;

                if (key == SDLK_q) {
                    running = 0;
                }
                else if (view.palette_editor_open) {
                    if (key == SDLK_ESCAPE || key == SDLK_e ||
                        key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                        view.palette_editor_open = 0;
                    }
                    else if (key == SDLK_r) {
                        uint8_t t = view.palette_editor_codes[0];
                        view.palette_editor_codes[0] = view.palette_editor_codes[3];
                        view.palette_editor_codes[3] = t;
                        t = view.palette_editor_codes[1];
                        view.palette_editor_codes[1] = view.palette_editor_codes[2];
                        view.palette_editor_codes[2] = t;
                        apply_editor_palette(&ppu, &view, &custom_palette_active,
                                             palette_label, sizeof(palette_label));
                    }
                    else if (key >= SDLK_1 && key <= SDLK_4) {
                        view.palette_editor_selected_shade = (unsigned)(key - SDLK_1);
                    }
                    else if (key == SDLK_LEFT) {
                        view.palette_editor_selected_shade =
                            (view.palette_editor_selected_shade + 3u) & 3u;
                    }
                    else if (key == SDLK_RIGHT) {
                        view.palette_editor_selected_shade =
                            (view.palette_editor_selected_shade + 1u) & 3u;
                    }
                }
                else if (key == SDLK_ESCAPE) {
                    if (view.menu_open) view.menu_open = 0;
                    else running = 0;
                }
                else if (key == SDLK_e) {
                    view.menu_open = 0;
                    view.palette_editor_open = 1;
                    view.palette_editor_selected_shade = 0u;
                    copy_palette_codes(&ppu, view.palette_editor_codes);
                    memcpy(editor_reset_codes, view.palette_editor_codes, 4u);
                    editor_preset_index = palette_mode ? palette_mode - 1u : 0u;
                    apply_editor_palette(&ppu, &view, &custom_palette_active,
                                         palette_label, sizeof(palette_label));
                    release_all_game_keys(&source);
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
                    custom_palette_active = 0;
                    request_next_palette(palette_mode, &palette_pending);
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
            else if (event.type == SDL_KEYUP &&
                     !view.palette_editor_open && !view.menu_open) {
                gb_source_key_t game_key;
                if (game_key_from_sdl(event.key.keysym.sym, &game_key)) {
                    (void)gb_source_set_key(&source, game_key, 0);
                }
            }
        }

        if (!running) break;

        if (!view.paused && !first_present) {
            const unsigned advances = gbcrt_clock_scheduler_step(&scheduler);
            if (advance_source(&source, &frame, advances) != 0) {
                fprintf(stderr, "source frame generation failed\n");
                break;
            }
        }
        first_present = 0;

        /*
         * Commit palette changes only at the comparison-frame boundary. Manual
         * mode ignores later SGB palette changes while source metadata keeps
         * caching them; returning to AUTO/SGB immediately applies the latest
         * palette. This is logical UI behavior only, not a microcontroller
         * simulation.
         */
        if (palette_pending >= 0) {
            palette_mode = (unsigned)palette_pending;
            palette_pending = -1;
            custom_palette_active = 0;
            apply_palette_mode(&ppu,
                               &frame,
                               palette_mode,
                               &applied_sgb_sequence,
                               palette_label,
                               sizeof(palette_label));
        }
        else if (!custom_palette_active && palette_mode == 0u &&
                 frame.sgb_palette_valid &&
                 frame.sgb_palette_sequence != applied_sgb_sequence) {
            apply_palette_mode(&ppu,
                               &frame,
                               palette_mode,
                               &applied_sgb_sequence,
                               palette_label,
                               sizeof(palette_label));
        }

        bridge_scale_frame(frame.shade, ext, BRIDGE_BORDER_EXT_INDEX);
        comparison_render(canvas, &frame, ext, &ppu, &view);

        char title[400];
        snprintf(title, sizeof(title),
                 "GB reference | RP2C02 EXT — %s/%s — %s — %s — GB frame %llu — output %llu — repeats %llu — skipped %llu",
                 view.source_name,
                 gbcrt_source_model_name(view.source_model),
                 gbcrt_clock_mode_name(view.clock_mode),
                 palette_label,
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
           palette_label,
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
