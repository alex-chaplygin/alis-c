#include "palette.h"
#include "sound.h"
#include "mouse.h"

char frame_num = 0;		/**< номер кадра */

/** 
 * Вызывается каждый кадр.
 * Обновляется палитра, звук, перерисовка курсора мыши.
 */
void timer_update()
{
  ++frame_num;
  if (!frame_num)
    --frame_num;
  palette_update();
  sound_update();
  mouse_draw_cursor();
}
