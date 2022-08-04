enum resource_e {
  RES_PALETTE = 0xfe,		/**< палитра */
  RES_COMPOSITE_SPRITE = 0xff,		/**< составной объект из спрайтов */
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

/// спрайт в составе объекта
typedef struct {
  word num;			/**< номер спрайта */
  short ofs_x;			/**< смещение спрайта по x*/
  short ofs_y;			/**< y */
  short ofs_z;			/**< z */
} subimage_t;

/// форма, каркас объекта
typedef struct {
  word form_type;		/**< тип формы 0, 1 или -1 */
  union {
    char rect0[6];		/**< тип 0 */
    short rect1[6];		/**< тип 1 */
  };
} form_t;

byte *res_get_image(int num);
byte *res_get_form(byte *script, int num);

extern int load_main_res;
