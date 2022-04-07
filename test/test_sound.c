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

void process_test(sound_channel_t input, sound_channel_t res)
{
  memcpy(sound_channels, &input, sizeof(sound_channel_t));
  sound_update();
  printf("testing flags\n");
  ASSERT(sound_channels->flags, res.flags);
  printf("testing priority\n");
  ASSERT(sound_channels->priority, res.priority);
  printf("testing synth\n");
  ASSERT(sound_channels->synth, res.synth);
  printf("testing length\n");
  ASSERT(sound_channels->length, res.length);
  printf("testing volume\n");
  ASSERT(sound_channels->volume, res.volume);
  printf("testing freq\n");
  ASSERT(sound_channels->frequency, res.frequency);
}

struct {
  sound_channel_t input;
  sound_channel_t res;
} process_table[] = {
  {
    {2, 0x7f, 1, 0x64, 0x7f00, -1, 0x1f4, 0},
    {2, 0x7f, 1, 0x63, 0x7eff, -1, 0x1f4, 0},
  },
  {
    {2, 0x7f, 1, 1, 0x7e9d, -1, 0x1f4, 0},
    {0, 0x80, 0, 0, 0, -1, 0, 0},
  },
  {
    {2, 0x64, 0x80, 0x64, 0x7f00, -1, 0x1f4, 0},
    {2, 0x64, 0x80, 0x64, 0x7f00, -1, 0x1f4, 0},
  },
  {
    {2, 0x7f, 1, 0x64, 0, -1, 0x1f4, 0},
    {0, 0x80, 0, 0x63, 0, -1, 0, 0},
  },
};

void process_channel_test()
{
  memset(sound_channels, 0, sizeof(sound_channel_t) * 4);
  for (int i = 0; i < sizeof(process_table) / sizeof(process_table[0]); i++) {
    printf("test: %d\n", i);
    process_test(process_table[i].input, process_table[i].res);
  }
}

void main()
{
  update_sound_test();
  process_channel_test();
}
