
void load_blancpc();
void class_load(int id, char *name);
void load_main_class();
void op_class_load();
int class_loaded(word id);
byte *class_get(int i);
int class_size(int i);
void class_free();

extern int total_classes;
extern int num_classes;
