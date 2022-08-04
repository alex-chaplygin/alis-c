#ifndef __WINDOW__
#define __WINDOW__
#pragma pack(1)

#define WINDOW_HIDDEN (1 << 6)	/**< окно не рисуется */
#define WINDOW_NOTTRANSLATED (1 << 7) /**< окно не была перемещена */

#define WINDOW2_FLAG1 (1 << 1)
#define WINDOW2_NOBLIT (1 << 5)	/**< окно не отправляется в видеопамять после отрисовки */
#define WINDOW2_MOUSE (1 << 6)	/**< в окне присутствует курсор мыши */
#define WINDOW2_3D (1 << 7)		/**< окно 3d */

/// окно графики
typedef struct window_s {
  byte flags;			/**< флаги окна */
  byte flags2;			
  word window_sprite;		/**< спрайт окна */
  word next;		/**< адрес следующей окна в списке */
  byte f1;
  byte f2;
  byte f3;
  byte f4;
  byte f5;			/**< проекция: перемещение x */
  byte f6;
  byte f7;			/**< проекция: перемещение y */
  byte f8;
  short min_x;			/**< экранные прямоугольника окна */
  short min_y;
  short width;			/**< ширина прямоугольника окна */
  short height;			/**< высота прямоугольника окна */
  char origin_x;		/**< начало коодинат окна в мировых координатах */
  char f18;
  char origin_y;
  char f20;
  char origin_z;
  char f22;
  char f23;
  char f24;
  char f25;
  char f26;
  char bx;			/**< система координат*/
  char by;			/**< вектор b (x) */
  char bz;
  char ax;			/**< вектор a (y)*/
  char ay;
  char az;
  char cx;			/**< вектор c (z)*/
  char cy;
  char cz;
  char f36;
  short delta_x;			/**< вектор перемещения */
  short delta_y;
  short delta_z;
} window_t;

void window_new();
void window_show();
void window_set();
void window_free_sprites();
void window_hide();

extern window_t *window_list_head;	/**< список окон */
#endif
