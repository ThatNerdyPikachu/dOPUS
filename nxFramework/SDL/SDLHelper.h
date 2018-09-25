#pragma once
#include "../common.h"

namespace NXFramework
{

namespace SDL
{

static inline SDL_Color MakeColour(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_Color colour = {r, g, b, a};
	return colour;
}

void ClearScreen(SDL_Renderer *renderer, SDL_Color colour);
void DrawRect(SDL_Renderer *renderer, int x, int y, int w, int h, SDL_Color colour);
void DrawCircle(SDL_Renderer *renderer, int x, int y, int r, SDL_Color colour);
void DrawText(SDL_Renderer *renderer, TTF_Font *font, int x, int y, SDL_Color colour, const char *text);
void DrawTextf(SDL_Renderer *renderer, TTF_Font *font, int x, int y, SDL_Color colour, const char* text, ...);
void LoadImage(SDL_Renderer *renderer, SDL_Texture **texture, char *path);
void LoadImageBuf(SDL_Renderer *renderer, SDL_Texture **texture, void *mem, int size);
void DrawImage(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y);
void DrawImageScale(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y, int w, int h);

}

#define WHITE                 SDL::MakeColour(255, 255, 255, 255)
#define BLACK_BG              SDL::MakeColour(48, 48, 48, 255)
#define STATUS_BAR_LIGHT      SDL::MakeColour(37, 79, 174, 255)
#define STATUS_BAR_DARK       SDL::MakeColour(38, 50, 56, 255)
#define MENU_BAR_LIGHT        SDL::MakeColour(51, 103, 214, 255)
#define MENU_BAR_DARK         SDL::MakeColour(55, 71, 79, 255)
#define BLACK                 SDL::MakeColour(0, 0, 0, 255)
#define SELECTOR_COLOUR_LIGHT SDL::MakeColour(241, 241, 241, 255)
#define SELECTOR_COLOUR_DARK  SDL::MakeColour(76, 76, 76, 255)
#define TITLE_COLOUR          SDL::MakeColour(30, 136, 229, 255)
#define TITLE_COLOUR_DARK     SDL::MakeColour(0, 150, 136, 255)
#define TEXT_MIN_COLOUR_LIGHT SDL::MakeColour(32, 32, 32, 255)
#define TEXT_MIN_COLOUR_DARK  SDL::MakeColour(185, 185, 185, 255)
#define BAR_COLOUR            SDL::MakeColour(200, 200, 200, 255)

}
