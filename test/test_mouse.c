#include <stdio.h>
#include <SDL.h>

#include "types.h"
#include "mouse.h"

#define WINDOW_WIDTH 640	/**< размеры окна */
#define WINDOW_HEIGHT 400

SDL_Window *window;		/**< окно SDL */
SDL_Renderer *renderer;		/**< устройство вывода */
SDL_Surface *screen;		/**< поверхность экрана */

void main()
{
  SDL_Event e;
  int x, y, b;

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  window = SDL_CreateWindow("mouse test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  SDL_ShowCursor(SDL_DISABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_CreateRGBSurface(0, 320, 200, 8, 0, 0, 0, 0);
  // Тестовая палитра
  SDL_Color colors[256];
  for(int i = 0; i < 256; i++){
    colors[i].r = i;
    colors[i].g = i;
    colors[i].b = i;
  }
  SDL_SetPaletteColors(screen->format->palette, colors, 0, 256);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
  SDL_LockSurface(screen);
  while (1) {
    if (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
	break;
      else if (e.type == SDL_MOUSEMOTION) {
        b = SDL_GetMouseState(&x, &y);
	mouse_event(x >> 1, y >> 1, b);
      } else if (e.type == SDL_MOUSEBUTTONDOWN) {
	SDL_GetMouseState(&x, &y);
	mouse_event(x, y, e.button.button);
      } else if (e.type == SDL_MOUSEBUTTONUP) {
	SDL_GetMouseState(&x, &y);
	mouse_event(x, y, 0);
      }
    }
    mouse_get(&x, &y, &b);
    printf("mouse x = %d, y = %d, buttons = %d\n", x, y, b);
    byte *buf = (byte *)screen->pixels;
    buf[y * 320 + x] = 255;
    SDL_UnlockSurface(screen);  
    SDL_RenderClear(renderer);
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, screen);
    SDL_RenderCopy(renderer, t, NULL, NULL);
    SDL_DestroyTexture(t);
    SDL_RenderPresent(renderer);
    SDL_LockSurface(screen);
  }
  SDL_UnlockSurface(screen);  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
