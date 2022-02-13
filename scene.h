#ifndef __SCENE__
#define __SCENE__
#pragma pack(1)

#define SCENE_HIDDEN (1 << 6)	/**< сцена не рисуется */
#define SCENE_NOTTRANSLATED (1 << 7) /**< сцена не была перемещена */

#define SCENE2_FLAG1 (1 << 1)
#define SCENE2_NOBLIT (1 << 5)	/**< сцена не отправляется в видеопамять после отрисовки */
#define SCENE2_MOUSE (1 << 6)	/**< в сцене присутствует курсор мыши */
#define SCENE2_3D (1 << 7)		/**< сцена 3d */

/// сцена графики
typedef struct scene_s {
  byte flags;			/**< флаги сцены */
  byte flags2;			
  word scene_sprite;		/**< спрайт сцены */
  word next;		/**< адрес следующей сцены в списке */
  byte f1;
  byte f2;
  byte f3;
  byte f4;
  byte f5;			/**< проекция: перемещение x */
  byte f6;
  byte f7;			/**< проекция: перемещение y */
  byte f8;
  word min_x;			/**< экранные прямоугольника сцены */
  word min_y;
  word width;			/**< ширина прямоугольника сцены */
  word height;			/**< высота прямоугольника сцены */
  byte origin_x;		/**< начало коодинат сцены в мировых координатах */
  byte f18;
  byte origin_y;
  byte f20;
  byte origin_z;
  byte f22;
  byte f23;
  char f24;
  byte f25;
  byte f26;
  char bx;			/**< система координат*/
  char by;			/**< вектор b (x) */
  char bz;
  char ax;			/**< вектор a (y)*/
  char ay;
  char az;
  char cx;			/**< вектор c (z)*/
  char cy;
  char cz;
  byte f36;
  word delta_x;			/**< вектор перемещения */
  word delta_y;
  word delta_z;
} scene_t;

void scene_new();
void scene_show();
void scene_set();

extern scene_t *scene_list_head;	/**< список сцен */
#endif
