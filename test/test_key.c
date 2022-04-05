#include <stdio.h>
#include <SDL.h>

#include "types.h"
#include "key.h"

#define WINDOW_WIDTH 640	/**< размеры окна */
#define WINDOW_HEIGHT 400

SDL_Window *window;		/**< окно SDL */

void main()
{
  SDL_Event e;
  byte b;
  
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  window = SDL_CreateWindow("mouse test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  while (1) {
    if (SDL_PollEvent(&e))
      if (e.type == SDL_QUIT)
	break;
    b = wait_for_key();
    printf("key = %d\n", b);
    if (b == 27)
      break;
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
}
