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
#include <SDL2/SDL_ttf.h>
#include "types.h"
#include "key.h"
#include "palette.h"
#include "graphics.h"
#include "mouse.h"

#define WINDOW_WIDTH 640	/**< размеры окна */
#define WINDOW_HEIGHT 400
#define FPS 50			/**< число кадров в секунду */

byte *video_buffer;		/**< видео буфер экрана 320 на 200 */
SDL_Window *window;		/**< окно SDL */
SDL_Renderer *renderer;		/**< устройство вывода */
SDL_Surface *screen;		/**< поверхность экрана без курсора мыши */
SDL_Surface *screen2;		/**< поверхность экрана */
SDL_Surface *cursor_surface;	/**< курсор мыши */
TTF_Font *font;			/**< шрифт для вывода */
long current_time;		/**< текущее время */
long time_step = 1000 / FPS;	/**< длительность кадра */

/** 
 * Инициализация графического интерфейса
 */
void graphics_init()
{
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
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
  TTF_Init();
  font = TTF_OpenFont("/home/alex/src/jet/jet-c/img/sans.ttf", 10);
  if (!font) {
    printf("Cannot load font: %s\n", TTF_GetError());
    exit(1);
  }
  SDL_ShowCursor(SDL_DISABLE);
  cursor_surface = SDL_CreateRGBSurface(0, 16, 16, 8, 0, 0, 0, 0);
  SDL_SetColorKey(cursor_surface, SDL_TRUE, 0);
  current_time = SDL_GetTicks();
  screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, 0, 0, 0);
  screen2 = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, 0, 0, 0);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
  SDL_LockSurface(screen);
  video_buffer = screen->pixels;
}

/// Обработка событий клавиатуры и мыши
int graphics_get_events()
{
  SDL_Event e;
  if (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      return 0;
    else if (e.type == SDL_KEYDOWN)
      set_key(e.key.keysym.scancode, e.key.keysym.sym, e.key.keysym.mod);
    else if (e.type == SDL_KEYUP)
      release_key(e.key.keysym.scancode);
    else if (e.type == SDL_MOUSEMOTION) {
      mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
      mouse_x >>= 1;
      mouse_y >>= 1;
    }
    else if (e.type == SDL_MOUSEBUTTONDOWN)
      mouse_buttons = e.button.button;
    else if (e.type == SDL_MOUSEBUTTONUP)
      mouse_buttons = 0;
  }
  return 1;
}

/** 
 * Обновляет палитру в начале каждого кадра
 * Вызывается в цикле интерпретатора
 */
void graphics_palette_update()
{
  long now = SDL_GetTicks();
  if ((now -  current_time) > time_step) {
    palette_update();
    current_time = now;
  }
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
  if (!graphics_get_events())
    return 0;
  long now = SDL_GetTicks();
  if (now -  current_time < time_step)
    SDL_Delay(now - current_time);
  else
    current_time = now;
  SDL_UnlockSurface(screen);
  //SDL_RenderClear(renderer);
  SDL_BlitSurface(screen, NULL, screen2, NULL);
  if (show_cursor >= 0) {
    SDL_Rect curs = {mouse_x, mouse_y, 16, 16};
    SDL_BlitSurface(cursor_surface, NULL, screen2, &curs);
  }
  SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, screen2);
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
  SDL_CloseAudio();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

/// Ожидание одного кадра
void graphics_sleep()
{
    SDL_Delay(1000 / FPS);
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
    colors[i].r = *dst++ << 2;
    colors[i].g = *dst++ << 2;
    colors[i].b = *dst++ << 2;
  }
  if (SDL_SetPaletteColors(screen->format->palette, colors, 0, 256)) {
    printf("palette error\n");
    exit(1);
  }
  if (SDL_SetPaletteColors(screen2->format->palette, colors, 0, 256)) {
    printf("palette error\n");
    exit(1);
  }
  if (SDL_SetPaletteColors(cursor_surface->format->palette, colors, 0, 256)) {
    printf("palette error\n");
    exit(1);
  }
}

/** 
 * Устанавливает изображение курсора мыши
 * 
 * @param img изображение
 * @param w ширина
 * @param h высота
 */
void graphics_set_cursor(byte *img, int w, int h)
{
  SDL_LockSurface(cursor_surface);
  memcpy(cursor_surface->pixels, img, w * h);
  SDL_UnlockSurface(cursor_surface);
}

/** 
 * Печать отладочной строки
 * 
 * @param x координаты
 * @param y 
 * @param str строка
 */
void graphics_print(int x, int y, char *str)
{
  int w;
  int h;
  int color = 0xffffff;
  SDL_Color c = {color >> 16, (color >> 8) & 0xff, color & 0xff};
  SDL_Surface* s = TTF_RenderText_Solid(font, str, c);
  TTF_SizeText(font, str, &w, &h);
  SDL_Rect dest = { x, y, w, h};
  SDL_UnlockSurface(screen);
  SDL_BlitSurface(s, NULL, screen, &dest);
  SDL_LockSurface(screen);
  SDL_FreeSurface(s);
}
