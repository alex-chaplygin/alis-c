#ifndef __OBJECTS__
#define __OBJECTS__

#include "types.h"
#include "memory.h"
#include "sprite.h"

#define OBJECT_NOMSG (1 << 0)	/**< поток не принимает сообщения */
#define OBJECT_NOSTART3 (1 << 1)	/**< не запуск сценария 3 каждый фрейм */
#define OBJECT_MSG (1 << 7) /**< есть сообщения для потока */

#pragma pack(1)

/// заголовок сценария
typedef struct {
  word id;			/**< номер сценария */
  word entry;			/**< адрес начала сценария */
  byte control;
  byte version;			/**< версия сценария */
  word entry2;			/**< обработка нажатий клавиш */
  byte u1;
  byte u2;
  word entry3;			/**< обработка сообщений */
  byte u4;
  byte u5;
  dword resources;		/**< адрес таблицы ресурсов */
  word stack_size;		/**< размер стека вызовов */
  word data_size;		/**< размер сегмента данных */
  word msg_size;		/**< размер стека сообщений */
} script_t;

/// структура потока
typedef struct object_s {
  int id;			/**< номер сценария */
  stack_t *call_stack;		/**< стек вызовов */
  int *saved_sp;		/**< сохраненный указатель стека */
  stack_t *msg_stack;		/**< стек сообщений потоку */
  seg_t *data;			/**< сегмент данных */
  byte *ip;			/**< текущий указатель команд */
  byte *script;			/**< образ сценария (содержит код и ресурсы) */
  byte *script2;
  int version;			/**< версия сценария */
  int frames_to_skip;		/**< число кадров через сколько выполняется сценарий */
  int cur_frames_to_skip;		/**< текущий отсчет кадров для выполнения */
  int running;			/**< основной сценарий потока запущен */
  int form;			/**< номер формы для столкновений */
  int flags;			/**< флаги потока */
  int x_flip;			/**< если 1, то все спрайты будут повернуты по горизонтали */
  int layer;			/**< слой отрисовки для всех новых спрайтов */
  int f20;
  int f22;
  int f25;
  int f26;
  int f2c;
  int f2d;
  int f2f;
  int f30;
  int f32;
  int f34;
  int sprites_object;		/**< номер потока, которому будут принадлежать новые спрайты */
  sprite_t *sprite_list;	/**< список спрайтов потока */
  window_t *current_window;		/**< текущая сцена */
  script_t *header;		/**< заголовок сценария */
  struct object_s *parent;	/**< родительский поток */
} object_t;

/// запись таблицы потоков
typedef struct objects_table_s {
  object_t *object;		/**< структура потока */
  struct objects_table_s *run_next; /**< следующий поток на запуск */
  struct objects_table_s *next;	/**< следующий поток в списке в запуске */
} object_table_t;

void object_init();
void object_init_table();
void object_setup_main(byte *script, int size);
void object_setup(object_table_t *tb, byte *script, int size);
object_t *object_add(byte *script, int size, vec_t *translate);
void objects_run();

void object_receive_msg();
void object_ready_to_receive();
void object_send_message();
void op_object_kill_remove_all();
void get_message();
void object_pause_yield_no_saved();
void object_clear_messages();
void kill_object_by_script(int id);
void object_stop();
void object_resume();
int object_num(object_t *t);
void script_num_to_object_num();
void set_object_f25();
void set_object_layer();
void set_sprites_object();
void get_objects_list();
void store_object_num();
void obj_set_form();
void object_find_all();
void op_kill_object();
void object_pause_by_ref();
void dump_objects();

extern int max_objects;		/**< максимальное количество потоков */
extern int num_run_objects;		/**< число рабочих потоков */
extern object_t *main_object;		/**< главный поток */
extern object_table_t *objects_table; /**< таблица потоков */
extern int *saved_sp;			/**< сохраненный указатель стека */

#endif
