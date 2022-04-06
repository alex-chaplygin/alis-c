#include "blancpc.h"
#include "sound.h"
#include "video.h"
#include "mouse.h"

/** 
 * Инициализация всех подсистем
 */
void global_init()
{
  load_blancpc();
  sound_init();
  video_init();
  mouse_init();
}
