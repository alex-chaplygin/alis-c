#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sound.h"
#include "test.h"

extern sound_channel_t  sound_channels[];	/**< каналы звука */
extern sound_channel_t *current_channel; /**< текущий канал */
extern int channel_num;		/**< номер текущего канала */

void update_test(int num)
{
  sound_update();
  ASSERT(current_channel - sound_channels, num);
  ASSERT(channel_num, num);
}

void update_sound_test()
{
  memset(sound_channels, 0, sizeof(sound_channel_t) * 4);
  sound_channels[0].flags = 2;
  update_test(0);
  sound_channels[1].flags = 2;
  update_test(1);
  sound_channels[2].flags = 2;
  update_test(2);
  sound_channels[2].flags = 0xff;
  update_test(1);
  sound_channels[1].flags = 0;
  update_test(0);
  sound_channels[1].flags = 0;
  update_test(0);
  sound_channels[2].flags = 0;
  update_test(0);
  sound_channels[0].flags = 2;
  update_test(0);
}

void main()
{
  update_sound_test();
}
