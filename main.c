#include "blancpc.h"
#include "sound.h"
#include "video.h"
#include "mouse.h"
#include "vm.h"
#include "threads.h"

/** 
 * Инициализация всех подсистем
 */
void global_init()
{
  load_blancpc();
  video_init();
  sound_init();
  mouse_init();
}

void main()
{
  global_init();
  threads_init();
  vm_init();
  while (1) {
    threads_run();
    video_update();
  }
}
