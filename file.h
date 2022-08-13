#define FILE_RW 2		/**< открытие на чтение и запись */

void file_open(char *file_name, int mode);
void file_read(void *buf, int size);
void file_close();
int file_exists(char *s);
void op_open_file();
void op_read_file();
void op_write_file();
void op_close_file();
void file_read_word();
void file_write_word();
