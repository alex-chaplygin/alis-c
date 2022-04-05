#include "types.h"
#include "sound.h"

#define NUM_CHANNELS 4		/**< число каналов звука */

#define CHANNEL_OFF (1 << 7)	/**< флаг - канал отключен */

sound_channel_t  sound_channels[NUM_CHANNELS];	/**< каналы звука */
sound_channel_t *current_channel; /**< текущий канал */
int channel_num;		/**< номер текущего канала */

void sound_check()
{
}

void sound_process(sound_channel_t *ch)
{
  current_channel = ch;
  channel_num = ch - sound_channels;
}

/** 
 * Обновление звуков.
 * Проверка 3-х каналов, что там появились звуки
 */
void sound_update()
{
  sound_channel_t *ch = sound_channels;
  sound_check();
  for (int i = 0; i < NUM_CHANNELS - 1; i++, ch++)
    if (ch->flags > 0)
      if (!(ch->flags & CHANNEL_OFF))
	sound_process(ch);
}
