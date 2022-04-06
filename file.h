#define FILE_CREATE 0x200		/**< создание файла */
#define FILE_APPEND 0x800		/**< открытие на добавление файла */

void file_open(char *file_name, int mode);
void file_read(void *buf, int size);
void file_close();
int file_exists(char *s);
