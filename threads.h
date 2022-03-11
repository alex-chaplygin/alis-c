#ifndef __THREADS__
#define __THREADS__

#include "types.h"
#include "memory.h"
#include "sprite.h"

#define THREAD_NOMSG (1 << 0)	/**< поток не принимает сообщения */
#define THREAD_NOSTART3 (1 << 1)	/**< не запуск сценария 3 каждый фрейм */
#define THREAD_MSG (1 << 7) /**< есть сообщения для потока */

#pragma pack(1)

/// заголовок сценария
typedef struct {
  word id;			/**< номер сценария */
  word entry;			/**< адрес начала сценария */
  byte control;
  byte version;			/**< версия сценария */
  word entry2;
  byte u1;
  byte u2;
  word entry3;
  byte u4;
  byte u5;
  dword resources;		/**< адрес таблицы ресурсов */
  word stack_size;		/**< размер стека вызовов */
  word data_size;		/**< размер сегмента данных */
  word msg_size;		/**< размер стека сообщений */
} script_t;

/// структура потока
typedef struct thread_s {
  int id;			/**< номер сценария */
  stack_t *call_stack;		/**< стек вызовов */
  int *saved_sp;		/**< сохраненный указатель стека */
  stack_t *msg_stack;		/**< стек сообщений потоку */
  seg_t *data;			/**< сегмент данных */
  byte *ip;			/**< текущий указатель команд */
  byte *script;			/**< образ сценария (содержит код и ресурсы) */
  int version;			/**< версия сценария */
  byte frames_to_skip;		/**< число кадров через сколько выполняется сценарий */
  byte cur_frames_to_skip;		/**< текущий отсчет кадров для выполнения */
  byte running;			/**< основной сценарий потока запущен */
  word flags2;			/**< доп. флаги потока */
  int flags;			/**< флаги потока */
  int x_flip;			/**< если 1, то все спрайты будут повернуты по горизонтали */
  int layer;			/**< слой отрисовки для всех новых спрайтов */
  int f2c;
  int f25;
  int sprites_thread;
  sprite_t *sprite_list;	/**< список спрайтов потока */
  scene_t *current_scene;		/**< текущая сцена */
  script_t *header;		/**< заголовок сценария */
  struct thread_s *parent;	/**< родительский поток */
} thread_t;

/// запись таблицы потоков
typedef struct threads_table_s {
  thread_t *thread;		/**< структура потока */
  struct threads_table_s *run_next; /**< следующий поток на запуск */
  struct threads_table_s *next;	/**< следующий поток в списке в запуске */
} thread_table_t;

void thread_init();
void thread_init_table();
void thread_setup_main(byte *script, int size);
void thread_setup(thread_table_t *tb, byte *script, int size);
thread_t *thread_add(byte *script, int size);
void threads_run();

void thread_receive_msg();
void thread_ready_to_receive();
void thread_send_message();
void op_thread_kill_remove_all();
void get_message();
void thread_pause_yield_no_saved();
void thread_clear_messages();
void kill_thread_by_script(int id);
void thread_stop();
void thread_resume();
int thread_num(thread_t *t);
void script_num_to_thread_num();
void set_thread_f25();
void set_sprites_thread();

extern int max_threads;		/**< максимальное количество потоков */
extern int num_run_threads;		/**< число рабочих потоков */
extern thread_t *main_thread;		/**< главный поток */
extern thread_table_t *threads_table; /**< таблица потоков */

#endif
