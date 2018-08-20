#pragma once
#include <iostream>

#include "../main.h"
#include "../sdl_helper.h"
#include "../html/utils.h"
#include "../console.h"

#include "../stack/stack.h"

#include "libs/imgui/imgui.h"

extern Console console;
extern device_aspect DEVICE;

//#define SCROLLBAR_WIDTH 25
#define SCROLLBAR_WIDTH 0 //TODO
#define ADDRESS_BAR_HEIGHT 35

struct browser_state {
    std::string action = "SELECTED_ADDRESSBAR";
};

namespace browser {
    class GUI {
        public:
            int x = 0, y = 0;
            browser_state State;
            SDL_Surface *_gui_surface = NULL;
            SDL_Surface *_browser_surface = NULL;
            SDL_Surface *_overlay_surface = NULL;

            SDL_Surface *_debugger_surface = NULL;

            GUI() {
                sdl_helper::init();
                console = Console();
            }
            ~GUI() {
                sdl_helper::exit();
            }

            void scroll (int x, int y, int accel = 15) {
                if (y > 0)
                    this->y += 1 * accel;
                else if (y < 0)
                    this->y -= 1 * accel;

                if (this->y >= 0) {
                    this->y = 0;
                };
            }

            void prepareTick() {
                // Get Window Size
                int w, h, window_w, window_h;
                SDL_GetRendererOutputSize(_renderer, &w, &h);
                SDL_GetWindowSize(_window, &window_w, &window_h);
                #ifndef __SWITCH__
                SDL_SetWindowSize(_debugger_window, window_w, window_h);
                #endif
                
                short scaling;
                if (w/window_w > 1) scaling = (short)w/window_w;
                else scaling = 1;

                DEVICE = {scaling, w, h};

                SDL_RenderClear(_renderer);

                // Clear surfaces
                if(this->_gui_surface != NULL)
                    SDL_FreeSurface(this->_gui_surface);
                this->_gui_surface = SDL_CreateRGBSurface(0, DEVICE.w, DEVICE.h, 32, 0, 0, 0, 255);
                SDL_FillRect(_gui_surface, NULL, SDL_MapRGBA(_gui_surface->format, 102, 51, 153, 255));
                SDL_SetColorKey(this->_gui_surface, SDL_TRUE, SDL_MapRGBA(this->_gui_surface->format, 102, 51, 153, 255));

                if(this->_overlay_surface != NULL)
                    SDL_FreeSurface(this->_overlay_surface);
                this->_overlay_surface = SDL_CreateRGBSurface(0, DEVICE.w, DEVICE.h, 32, 0, 0, 0, 0);
                SDL_SetColorKey(this->_overlay_surface, SDL_TRUE, SDL_MapRGBA(this->_overlay_surface->format, 0, 0, 0, 255));

                #ifndef __SWITCH__
                if(this->_debugger_surface != NULL)
                    SDL_FreeSurface(this->_debugger_surface);
                this->_debugger_surface = SDL_CreateRGBSurface(0, DEVICE.w, DEVICE.h, 32, 0, 0, 0, 0);
                #endif
            }

            bool doTick() {
                SDL_Rect screen_pos = {0, 0, DEVICE.w, DEVICE.h};
                SDL_Rect browser_pos_dst = {0, (ADDRESS_BAR_HEIGHT * DEVICE.scaling) + this->y, DEVICE.w - SCROLLBAR_WIDTH, DEVICE.h};

                SDL_Texture *browser = SDL_CreateTextureFromSurface(_renderer, this->_browser_surface);
                SDL_RenderCopy(_renderer, browser, &screen_pos, &browser_pos_dst);
                SDL_DestroyTexture(browser);

                SDL_Texture *gui = SDL_CreateTextureFromSurface(_renderer, this->_gui_surface);
                SDL_RenderCopy(_renderer, gui, &screen_pos, &screen_pos);
                SDL_DestroyTexture(gui);

                SDL_Texture *overlay = SDL_CreateTextureFromSurface(_renderer, this->_overlay_surface);
                SDL_RenderCopy(_renderer, overlay, &screen_pos, &screen_pos);
                SDL_DestroyTexture(overlay);

                #ifndef __SWITCH__
                SDL_Texture *debugger = SDL_CreateTextureFromSurface(_debugger_renderer, this->_debugger_surface);
                SDL_RenderCopy(_debugger_renderer, debugger, NULL, NULL);
                SDL_DestroyTexture(debugger);
                #endif

                SDL_RenderPresent(_renderer);

                #ifndef __SWITCH__
                SDL_RenderPresent(_debugger_renderer);
                #endif
            }
    };

    namespace UIElements {
        namespace AddressBar {
            void Render (browser::GUI *GUI, browser::STACK *STACK) {
                short height = ADDRESS_BAR_HEIGHT;
                stack Stack = STACK->getCurrentPage();

                #ifdef __SWITCH__
                    TTF_Font *font = browser::utils::get_font_from_cache("romfs:/fonts/NintendoStandard.ttf", (height/2-2) * DEVICE.scaling);
                #elif __MACOS__
                    TTF_Font *font = browser::utils::get_font_from_cache("/Library/Fonts/Microsoft Sans Serif.ttf", (height/2-2) * DEVICE.scaling);
                #else
                    TTF_Font *font = browser::utils::get_font_from_cache("../../resources/fonts/NintendoStandard.ttf", (height/2-2) * DEVICE.scaling);
                #endif

                sdl_helper::renderBackground (GUI->_gui_surface, {
                    0,
                    0,
                    DEVICE.w,
                    height * DEVICE.scaling
                }, {235, 235, 235, 255});
                sdl_helper::renderBackground (GUI->_gui_surface, {
                    (height + height/2) * DEVICE.scaling,
                    5 * DEVICE.scaling,
                    DEVICE.w - ((height*2) * DEVICE.scaling),
                    (height - 10) * DEVICE.scaling
                }, {255, 255, 255, 255});

                sdl_helper::renderText(Stack.path, GUI->_gui_surface, {(height + height/2 + 10) * DEVICE.scaling, ((height - height/2-2)/2) * DEVICE.scaling, DEVICE.w , 25 * DEVICE.scaling}, DEVICE.w, font, {0, 0, 0, 255});

                // Home Icon
                sdl_helper::renderBackground (GUI->_gui_surface, {
                    10 * DEVICE.scaling,
                    5 * DEVICE.scaling,
                    (height - 5) * DEVICE.scaling,
                    (height - 10) * DEVICE.scaling
                }, {255, 255, 255, 255});

                sdl_helper::renderBackground (GUI->_overlay_surface, {
                    0,
                    height * DEVICE.scaling,
                    (int)(DEVICE.w * (Stack.loaded/100)),
                    2 * DEVICE.scaling
                }, {0, 255, 0, 255});
            }
        }
        namespace Console {
            void Render (browser::GUI *GUI) {
                //if(console.hidden)
                //    return;

                #ifdef __SWITCH__
                    TTF_Font *font = browser::utils::get_font_from_cache("romfs:/fonts/NintendoStandard.ttf", 18 * DEVICE.scaling);
                #elif __MACOS__
                    TTF_Font *font = browser::utils::get_font_from_cache("/Library/Fonts/Arial.ttf", 18 * DEVICE.scaling);
                #else
                    TTF_Font *font = browser::utils::get_font_from_cache("../../resources/fonts/NintendoStandard.ttf", 18 * DEVICE.scaling);
                #endif

                sdl_helper::renderBackground (GUI->_debugger_surface, {
                    (DEVICE.w - (DEBUG_CONSOLE_WIDTH * DEVICE.scaling + 30)) + 15,
                    15,
                    DEBUG_CONSOLE_WIDTH * DEVICE.scaling,
                    DEVICE.h - 30
                }, {180, 180, 180, 255});

                sdl_helper::renderText(console.getFormattedOutput(), GUI->_debugger_surface, {
                    (DEVICE.w - (DEBUG_CONSOLE_WIDTH * DEVICE.scaling + 30)) + 30,
                    30,
                    DEVICE.h,
                    DEBUG_CONSOLE_WIDTH * DEVICE.scaling},
                    DEBUG_CONSOLE_WIDTH * DEVICE.scaling, font, {55, 55, 55, 255});
            }
            void RenderStat (browser::GUI *GUI, short pos, std::string text) {
                //if(console.hidden)
                //    return;

                #ifdef __SWITCH__
                    TTF_Font *font = browser::utils::get_font_from_cache("romfs:/fonts/NintendoStandard.ttf", 18 * DEVICE.scaling);
                #elif __MACOS__
                    TTF_Font *font = browser::utils::get_font_from_cache("/Library/Fonts/Arial.ttf", 18 * DEVICE.scaling);
                #else
                    TTF_Font *font = browser::utils::get_font_from_cache("../../resources/fonts/NintendoStandard.ttf", 18 * DEVICE.scaling);
                #endif

                sdl_helper::renderBackground (GUI->_debugger_surface, {
                    15,
                    DEVICE.h - (15 + ((28*DEVICE.scaling)*pos)),
                    500*DEVICE.scaling,
                    25*DEVICE.scaling
                }, {180, 180, 180, 55});

                sdl_helper::renderText(text, GUI->_debugger_surface,
                    {20, DEVICE.h - (15 + ((28*DEVICE.scaling)*pos)), DEVICE.w, 30}, DEVICE.w, font, {0, 255, 0, 255});
            }
        }
    }
}