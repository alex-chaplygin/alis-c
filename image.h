enum resource_e {
  RES_PALETTE = 0xfe,		/**< палитра */
  RES_IMAGE = 0xff,		/**< составное изображение */
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
  word brightness;
} image_t;

/// координатное окно
typedef struct {
  int min_x;			/**< левый верхний угол */
  int min_y;
  int max_x;			/**< правый нижний угол */
  int max_y;
} rectangle_t;

void show_sprite();
void show_sprite_flipped();
byte *get_resource(int num);

extern int load_main_image;	
