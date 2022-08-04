
/// координатное окно
typedef struct {
  int min_x;			/**< левый верхний угол */
  int min_y;
  int max_x;			/**< правый нижний угол */
  int max_y;
} rectangle_t;

void show_object();
void show_object_flipped();
byte *get_resource(int num);
void set_tag();
void clear_all_objects();
void show_object_0();
void add_sprite(int num, vec_t *origin, int x_flip, int is_object, int tag);
void clear_all_objects2();

extern int image_flag;
