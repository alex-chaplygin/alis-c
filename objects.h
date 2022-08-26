#ifndef __OBJECTS__
#define __OBJECTS__

#include "types.h"
#include "memory.h"
#include "sprite.h"

#define OBJECT_NOMSG (1 << 0)	/**< поток не принимает сообщения */
#define OBJECT_NOHANDLEMSG (1 << 1)	/**< не запуск программы обработки сообщений каждый фрейм */
#define OBJECT_MSG (1 << 7) /**< есть сообщения для потока */

#pragma pack(1)

/// заголовок сценария
typedef struct {
  word id;			/**< номер класса */
  word entry;			/**< адрес начала программы */
  byte control;
  byte version;			/**< версия компилятора */
  word key_entry;			/**< обработка нажатий клавиш */
  byte u1;
  byte u2;
  word msg_handle_entry;			/**< обработка сообщений */
  byte u4;
  byte u5;
  dword resources;		/**< адрес таблицы ресурсов */
  word stack_size;		/**< размер стека вызовов */
  word data_size;		/**< размер сегмента данных */
  word msg_size;		/**< размер стека сообщений */
} program_t;

/// структура потока
typedef struct object_s {
  int id;			/**< номер класса */
  stack_t *call_stack;		/**< стек вызовов */
  int *saved_sp;		/**< сохраненный указатель стека */
  int *msg_queue;		/**< очередь сообщений */
  int msg_size;			/**< размер обчереди сообщений */
  int *msg_begin;			/**< начало очереди сообщений */
  int *msg_end;			/**< конец очереди сообщений */
  seg_t *data;			/**< данные объекта */
  byte *ip;			/**< текущий указатель команд */
  byte *class;			/**< образ класса (содержит программу и ресурсы) */
  byte *class2;
  int version;			/**< версия компилятора */
  int frames_to_skip;		/**< число кадров через сколько выполняется программа */
  int cur_frames_to_skip;		/**< текущий отсчет кадров для выполнения */
  int running;			/**< программа работает */
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
  int sprites_object;		/**< номер объекта, которому будут принадлежать новые спрайты */
  sprite_t *sprite_list;	/**< список спрайтов объекта */
  view_t *current_view;		/**< окно, куда добавляются новые спрайты */
  program_t *header;		/**< заголовок программы */
  struct object_s *parent;	/**< родительский объект */
} object_t;

/// запись таблицы объектов
typedef struct objects_table_s {
  object_t *object;		/**< объект */
  struct objects_table_s *run_next; /**< следующий объект на запуск */
  struct objects_table_s *next;	/**< следующий объект в списке в запуске */
} object_table_t;

void objects_init();
void objects_init_table();
void object_setup_main(byte *class, int size);
void object_setup(object_table_t *tb, byte *class, int size);
object_t *object_add(byte *class, int size, vec_t *translate);
void object_new();
void objects_run();

void object_receive_msg();
void object_ready_to_receive();
void object_send_message();
void op_object_kill_remove_all();
void object_get_message();
void object_pause();
void object_clear_messages();
void objects_kill_by_class(int id);
void object_stop();
void object_resume();
int object_num(object_t *t);
void objects_find_by_class();
void object_set_f25();
void object_set_origin();
void object_move_origin();
void objects_get_all();
void object_store_next();
void obj_set_form();
void set_find_all_objects();
void object_kill_no_remove();
void object_pause_by_ref();
void objects_dump();
void object_disable_msg();
void object_disable_handle_msg();
void object_set_flip0();

extern int max_objects;		/**< максимальное количество потоков */
extern int num_run_objects;		/**< число рабочих потоков */
extern object_t *main_object;		/**< главный поток */
extern object_table_t *objects_table; /**< таблица потоков */
extern int *saved_sp;			/**< сохраненный указатель стека */
extern word objects_list[512];
extern int find_all_objects;		/**< если 1, то ищутся все объекты по классу */
#endif
