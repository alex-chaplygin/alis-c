#include "sprites.h"

#define THREAD_NOMSG (1 << 0)	/**< поток не принимает сообщения */
#define THREAD_NOSTART3 (1 << 1)	/**< не запуск сценария 3 каждый фрейм */
#define THREAD_MSG (1 << 7) /**< есть сообщения для потока */

/// структура потока
typedef struct {
  byte flags;			/**< флаги потока */
  char running;		/**< если 0 - поток не работает */
  int num_frames;		/**< число кадров за которое поток работает 1 раз */
  int frame_num;		/**< счетчик кадров */
  byte *io_file;		/**< загруженный образ io файла */
  byte *memory;			/**< память потока */
  word ip;			/**< указатель команд */
  word call_sp;			/**< указатель стека вызовов */
  sprite_t *sprites;		/**< список спрайтов потока */
  word *call_stack;		/**< стек вызовов */
} thread_t;

/// запись таблицы потоков
typedef struct threads_table_s {
  thread_t *thread;		/**< указатель на поток */
  struct threads_table_s *next;	/**< следующий поток в списке в запущенных */
} threads_table_t;

void threads_init();
void threads_run();
int threads_free_num();
byte *threads_call(int step, byte *ip);
