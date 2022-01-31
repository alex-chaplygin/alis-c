/**
 * @file   graphics.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 15:29:03 2022
 * 
 * @brief  Вывод графики через библиотеку SDL
 * 
 */

#include <stdlib.h>
#include <SDL.h>
#include "types.h"
#include "key.h"
#include "palette.h"

#define WINDOW_WIDTH 640	/**< размеры окна */
#define WINDOW_HEIGHT 400
#define SCREEN_WIDTH 320	/**< размеры буфера экрана */
#define SCREEN_HEIGHT 200
#define FPS 30			/**< число кадров в секунду */

int mouse_x;
int mouse_y;
int mouse_button;
byte frame_num = 0;
byte timer_num = 0;
byte *video_buffer;		/**< видео буфер экрана 320 на 200 */
SDL_Window *window;		/**< окно SDL */
SDL_Renderer *renderer;		/**< устройство вывода */
SDL_Surface *screen;		/**< поверхность экрана */
long current_time;		/**< текущее время */
long time_step = 1000 / FPS;	/**< длительность кадра */

/** 
 * Инициализация графического интерфейса
 */
void graphics_init()
{
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
  //atexit(graphics_close);
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
  video_buffer = screen->pixels;
  //mouse_init
  //sound_init
}

/** 
 * Обновление графики.
 * Обработка событий клавиатуры, мыши.
 * Обновление таймера, задержка кадра для постоянного fps.
 * Отрисовка экранного буфера.
 * 
 * @return 0 - если окно было закрыто, иначе 1
 */
int graphics_update()
{
  SDL_Event e;
  if (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      return 0;
    else if (e.type == SDL_KEYDOWN)
      // key func
      set_key(e.key.keysym.scancode, e.key.keysym.mod);
    else if (e.type == SDL_KEYUP)
      release_key(e.key.keysym.scancode);
    else if (e.type == SDL_MOUSEMOTION) {
      SDL_GetGlobalMouseState(&mouse_x, &mouse_y);
      mouse_x >>= 1;
      mouse_y >>= 1;
      // mouse_func
    }
    else if (e.type == SDL_MOUSEBUTTONDOWN)
      mouse_button = e.button.button;
    else if (e.type == SDL_MOUSEBUTTONUP)
      mouse_button = 0;
  }
  long now = SDL_GetTicks();
  if (now -  current_time < time_step)
    SDL_Delay(now - current_time);
  else
    current_time = now;
  // sound update
  //palette_update();
  //printf("tick %i\n", now);
  SDL_UnlockSurface(screen);  
  SDL_RenderClear(renderer);
  SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, screen);
  SDL_RenderCopy(renderer, t, NULL, NULL);
  SDL_DestroyTexture(t);
  SDL_RenderPresent(renderer);
  SDL_LockSurface(screen);
  video_buffer = screen->pixels;
  return 1;
}

/** 
 * Завершение графики, закрытие окна
 */
void graphics_close()
{
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

/** 
 * Установка новой палитры
 * 
 * @param palette данные палитры: 3 * 256 = 768 байт
 */
void graphics_set_palette(byte *palette)
{
  SDL_Color colors[256];
  byte *dst = palette;
  for(int i = 0; i < 256; i++) {
      colors[i].r = *dst++;
      colors[i].g = *dst++;
      colors[i].b = *dst++;
  }
  SDL_SetPaletteColors(screen->format->palette, colors, 0, 256);
}