/**
 * @file   sound.c
 * @author alex <alex@localhost>
 * @date   Tue Apr  5 07:29:09 2022
 * 
 * @brief  Модуль обработки звуков (синтезатор + оцифрованный звук)
 *
 * Первые 3 канала могут использоваться как синтезатор
 * Все 4 канала могут использоваться под оцифрованный звук
 */

#include "types.h"
#include "sound.h"

#define NUM_CHANNELS 4		/**< число каналов звука */
#define MAX_VOLUME 32767	/**< порог громкости */

#define CHANNEL_OFF (1 << 7)	/**< флаг - канал отключен */

sound_channel_t  sound_channels[NUM_CHANNELS];	/**< каналы звука */
sound_channel_t *current_channel; /**< текущий канал */
int channel_num;		/**< номер текущего канала */
int channel_num_max_priority;

/** 
 * Вычисляется номер канала, у которого максимальный приоритет
 */
void sound_find_channel_max_priority()
{
}

/** 
 * Проиграть ноту на синтезаторе
 * 
 * @param ch в канале - параметры ноты
 */
void synth_play(sound_channel_t *ch)
{
}

/** 
 * Выключение канала звука
 * 
 * @param ch канал
 */
void sound_channel_off(sound_channel_t *ch)
{
  ch->volume = 0;
  ch->frequency = 0;
  ch->priority = -128;
  ch->synth = 0;
  ch->flags = 0;
  synth_play(ch);
}

/** 
 * Обработка канала звука
 * 
 * Звук звучит с определенной длительностью
 * Громкость может изменяться (усиление/затухание) звука
 * @param ch текущий канал звука
 */
void sound_process_channel(sound_channel_t *ch)
{
  short vol;
  current_channel = ch;
  channel_num = ch - sound_channels;
  if (ch->synth < 0)
    return;
  --ch->length;
  if (!ch->length) {
    sound_channel_off(ch);
    return;
  }
  vol = ch->volume + ch->gain;
  if (vol < 0) {
    sound_channel_off(ch);
    return;
  }
  if (vol > MAX_VOLUME)
    vol = MAX_VOLUME;
  ch->volume = vol;
  vol = ch->frequency + ch->frequency_gain;
  if (vol < 0)
    sound_channel_off(ch);
  else {
    ch->frequency = vol;
    synth_play(ch);
  }
}

/** 
 * Обновление звуков.
 * Проверка 3-х каналов, что там появились звуки
 */
void sound_update()
{
  sound_channel_t *ch = sound_channels;
  sound_find_channel_max_priority();
  for (int i = 0; i < NUM_CHANNELS - 1; i++, ch++)
    if (ch->flags > 0)
      if (!(ch->flags & CHANNEL_OFF))
	sound_process_channel(ch);
}

/** 
 * Инициализация звуковой системы
 * Инициализация каналов
 */
void sound_init()
{
}
