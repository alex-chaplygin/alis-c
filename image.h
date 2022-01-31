enum resource_e {
  RES_PALETTE = 0xfe,		/**< палитра */
  RES_IMAGE = 0xff,		/**< составное изображение */
};

/// заголовок изображения
typedef struct {
  byte type; 	/**< тип ресурса/изображения */
  byte num_subimages;		/**< для составного изображения
число компонент */
  word maxx;			/**< ширина - 1*/
  word maxy;			/**< высота - 1 */
  word brightness;
} image_t;

void show_sprite();
void show_sprite_flipped();
byte *get_resource(int num);

extern int load_main_image;	
