enum resource_e {
  RES_PALETTE = 0xfe,		/**< палитра */
  RES_OBJECT = 0xff,		/**< составной объект из спрайтов */
};

/// заголовок изображения
typedef struct {
  byte type; 	/**< тип ресурса/изображения */
  union {
    byte num_subimages;		/**< для составного изображения
число компонент */
    byte fill_color;		/**< для типа 1 - цвет заливки */
  };
  word maxx;			/**< ширина - 1*/
  word maxy;			/**< высота - 1 */
  word palette_offset;		/**< смещение палитры для некоторых типов */
} image_t;

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

extern int load_main_res;	
extern int image_flag;
