FILE *file_open(char *file_name, char *mode);
void file_read(FILE *f, void *buf, int size);
int file_exists(char *s);
void op_open_file();
void op_read_file();
void op_close_file();
