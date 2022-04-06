/**
 * @file   video.c
 * @author alex <alex@localhost>
 * @date   Wed Apr  6 10:35:00 2022
 * 
 * @brief  Модуль для работы с графикой SDL: окно, обработка событий, вывод графики
 * 
 */
#include <stdio.h>
#include <SDL.h>

#include "types.h"

#define WINDOW_WIDTH 640	/**< размеры окна */
#define WINDOW_HEIGHT 400
#define SCREEN_WIDTH 320	/**< размеры буфера экрана */
#define SCREEN_HEIGHT 200

SDL_Window *window;		/**< окно SDL */
SDL_Renderer *renderer;		/**< устройство вывода */
SDL_Surface *screen;		/**< поверхность экрана */
long current_time;		/**< текущее время */
byte *video_buffer;		/**< видео буфер экрана 320 на 200 */

void video_init()
{
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow("alis", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("Cannot create window\n");
    exit(1);
  }
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    printf("Cannot create renderer\n");
    exit(1);
  }
  current_time = SDL_GetTicks();
  screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, 0, 0, 0);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
  SDL_LockSurface(screen);
  video_buffer = screen->pixels;
}
