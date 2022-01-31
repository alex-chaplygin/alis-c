void set_key(int scan, int mod);
void release_key(int scan);
void get_key(); //6a
void get_joy(); //6e
void get_control_key();
void wait_key();

void init_keyboard();

extern int key_mod;		/**< нажатые модификаторы */
