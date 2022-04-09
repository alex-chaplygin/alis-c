#include "types.h"

#pragma pack(1)

typedef struct {
  word id;			/**< номер сценария */
  word main_entry;			/**< адрес начала сценария */
  byte control;
  byte version;			/**< версия сценария */
  word key_entry;			/**< обработка нажатий клавиш */
  byte u1;
  byte u2;
  word message_entry;			/**< обработка сообщений */
  byte u4;
  byte u5;
  dword resources;		/**< адрес таблицы ресурсов */
  word stack_size;		/**< размер стека вызовов */
  word data_size;		/**< размер сегмента данных */
  word msg_size;		/**< размер стека сообщений */
} io_header_t;

void io_unpack_sound(byte *io);
int io_num_free_files();
