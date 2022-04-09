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
#include "timer.h"
#include "key.h"
#include "mouse.h"

#define WINDOW_WIDTH 640	/**< размеры окна */
#define WINDOW_HEIGHT 400
#define SCREEN_WIDTH 320	/**< размеры буфера экрана */
#define SCREEN_HEIGHT 200
#define FPS 40			/**< число кадров в секунду */

SDL_Window *window;		/**< окно SDL */
SDL_Renderer *renderer;		/**< устройство вывода */
SDL_Surface *screen;		/**< поверхность экрана */
long current_time;		/**< текущее время */
byte *video_buffer;		/**< видео буфер экрана 320 на 200 */
long time_step = 1000 / FPS;	/**< длительность кадра */

/** 
 * Инициализация видео подсистемы.
 * Создается окно и буфер экрана
 */
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

/** 
 * Обновление экрана.
 * Делается задержка, чтобы соблюдать частоту кадров
 * обновляется таймер, видео буфер выводится на экран.
 */
void video_update()
{
  long now = SDL_GetTicks();
  if (now -  current_time < time_step)
    SDL_Delay(now - current_time);
  else
    current_time = now;
  timer_update();
  SDL_UnlockSurface(screen);  
  SDL_RenderClear(renderer);
  SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, screen);
  SDL_RenderCopy(renderer, t, NULL, NULL);
  SDL_DestroyTexture(t);
  SDL_RenderPresent(renderer);
  SDL_LockSurface(screen);
  video_buffer = screen->pixels;
}

/** 
 * Завершение работы видео подсистемы
 */
void video_close()
{
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

/** 
 * Обработка событий клавиатуры и мыши
 */
void video_update_events()
{
  SDL_Event e;
  int x, y, b;
  
  if (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      video_close();
      exit(0);
    } else if (e.type == SDL_KEYDOWN)
      set_key(e.key.keysym.scancode, e.key.keysym.sym, e.key.keysym.mod);
    else if (e.type == SDL_KEYUP)
      release_key(e.key.keysym.scancode);
    else if (e.type == SDL_MOUSEMOTION) {
      b = SDL_GetMouseState(&x, &y);
      mouse_event(x >> 1, y >> 1, b);
    } else if (e.type == SDL_MOUSEBUTTONDOWN) {
      SDL_GetMouseState(&x, &y);
      mouse_event(x >> 1, y >> 1, e.button.button);
    } else if (e.type == SDL_MOUSEBUTTONUP) {
      SDL_GetMouseState(&x, &y);
      mouse_event(x >> 1, y >> 1, 0);
    }
  }
}
