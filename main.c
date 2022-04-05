#include "blancpc.h"
#include "sound.h"

/** 
 * Инициализация всех подсистем
 */
void global_init()
{
  load_blancpc();
  sound_init();
  graphics_init();
  mouse_init();
}
