/**
 * @file   sound.c
 * @author alex <alex@localhost>
 * @date   Thu Aug  4 17:55:25 2022
 * 
 * @brief  Команды - работа со звуком
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpret.h"
#include "get.h"
#include "res.h"
#include "objects.h"

#define NUM_CHANNELS 4		/**< число каналов звука */
#define MAX_VOLUME 32767	/**< порог громкости */
#define DEFAULT_PRIORITY -128
#define CHANNEL_OFF (1 << 7)	/**< флаг - канал выключен */
#define CHANNEL_2 (1 << 1)

typedef struct {
  byte flags;			/**< флаги канала */
  char priority;		/**< приоритет канала, звучит только канал с максимальным приоритетом */
  char ready;			/**< если больше 0, то канал синтезатора */
  word length;		/**< число кадров - длительность звучания ноты */
  short volume;			/**< громкость звучания ноты */
  short gain;			/**< смещение громкости (усиление и затухание звука во времени) ноты */
  short frequency;		/**< частота ноты */
  short frequency_gain;		/**< смещение частоты ноты во времени */
  int object;			/**< номер объекта, который послал звук */
} sound_channel_t;

sound_channel_t  sound_channels[NUM_CHANNELS];	/**< каналы звука */

/** 
 * Инициализация каналов звука
 */
void init_sound_channels()
{
  sound_channel_t *ch;
  ch = sound_channels;
  for (int i = 0; i < NUM_CHANNELS; i++, ch++) {
    memset(ch, 0, sizeof(sound_channel_t));
    ch->priority = DEFAULT_PRIORITY;
  }
}

/** 
 * Найти канал с минимальным приоритетом
 * 
 * @return канал
 */
sound_channel_t * find_minpri_channel()
{
  sound_channel_t *ch;
  ch = &sound_channels[0];
  if (ch->priority > sound_channels[1].priority)
    ch = &sound_channels[1];
  if (ch->priority > sound_channels[2].priority)
    ch = &sound_channels[2];
  if (ch->priority > sound_channels[3].priority)
    ch = &sound_channels[3];
  return ch;
}

/** 
 * Добавление звука в систему
 * 
 * @param pri приоритет
 * @param obj номер объекта
 * @param ready готовность
 * @param freq частота
 * @param fgain смещение частоты
 * @param vol громкость
 * @param gain смещение амплитуды
 * @param len длительность
 */
void  add_sound(int pri, int obj, int ready, int freq, int fgain, int vol, int gain, int len)
{
  sound_channel_t *ch;
#ifdef DEBUG
  printf("add sound: pri=%x obj=%x\n", pri, obj);
#endif
  if (pri < 0) {
    printf("add sound: pri < 0 %d\n", pri);
    exit(1);
  }
  ch = sound_channels;
  for (int i = 0; i < NUM_CHANNELS; i++, ch++) {
#ifdef DEBUG
    printf("Check channel %d obj = %x pri = %x\n", i, ch->object, ch->priority);
#endif
    if (ch->object != obj)
      continue;
    else if (ch->priority != pri)
      continue;
    else {
      printf("add sound: obj == pri ==\n");
      exit(1);
    }
  }
  ch = find_minpri_channel();
  if (pri < 0 || !ch->priority || ch->priority == DEFAULT_PRIORITY) {
    if (ch->priority > pri)
      return;
    ch->flags = CHANNEL_OFF;
    ch->priority = pri;
    ch->ready = ready;
    ch->frequency = freq;
    ch->frequency_gain = fgain;
    ch->volume = vol << 8;
    ch->gain = gain;
    ch->length = len;
    ch->object = obj;
    ch->flags = CHANNEL_2;
    if (ready & 0x80) {
      printf("ready & 0x80\n");
      exit(1);
    }
#ifdef DEBUG
    printf("adding sound channel num = %d\n", (int)(ch - sound_channels));
    printf("flags=%x pri=%x ready=%x freq=%x fgain=%x, vol=%x, gain=%x len=%x obj=%x\n", ch->flags, ch->priority, ch->ready, ch->frequency, ch->frequency_gain, ch->volume, ch->gain, ch->length, ch->object);
#endif
  }
}

/** 
 * Проиграть ноту на синтезаторе
 * 
 * @param ch в канале - параметры ноты
 */
void synth_play(sound_channel_t *ch)
{
  extern int vol;
  vol = (ch->volume >> 8);
  extern int freq;
  freq = 32 * 55000 / ((ch->frequency >> 3) + 320);
  extern int ready;
  ready = ch->ready;
  printf("vol = %d freq = %d ready = %d\n", vol, freq, ready);
  // если ready == 0, то нота выключается
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
  ch->priority = DEFAULT_PRIORITY;
  ch->ready = 0;
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
void sound_channel_update(sound_channel_t *ch)
{
  short vol;
  int channel_num = ch - sound_channels;
  if (ch->ready < 0)
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
void sound_channels_update()
{
  sound_channel_t *ch = sound_channels;
  for (int i = 0; i < NUM_CHANNELS - 1; i++, ch++)
    if (ch->flags > 0)
      if (!(ch->flags & CHANNEL_OFF))
	sound_channel_update(ch);
}

/** 
 * Проигрывание звука через синтезатор (заглушка)
 */
void play_sound_synth()
{
  new_get();
  int pri = (char)current_value;
  new_get();
  int vol = (char)current_value;
  new_get();
  int freq = current_value;
  new_get();
  int slen = current_value;
#ifdef DEBUG
  printf("play_sound_synth pri=%d vol=%d freq=%d len=%d\n", pri, vol, freq, slen);
#endif
  if (!slen)
    return;
  int gain = (vol << 8) / slen;
  if (gain)
    gain = 1;
  add_sound(pri, object_num(run_object), 1, freq, 0, vol, -gain, slen);
}

/** 
 * Проигрывание звука из ресурса
 * 
 */
void play_sound()
{
  load_main_res = 0;
  new_get();
  int num = current_value;
  new_get();
  int s1 = current_value;
  new_get();
  int s2 = current_value;
  new_get();
  int s3 = current_value;
  new_get();
  int s4 = current_value;
#ifdef DEBUG
  printf("play_sound_res %d %d %d %d %d\n", num, s1, s2, s3, s4);
#endif
  exit(1);
}

void play_music()
{
#ifdef DEBUG
  printf("play music\n");
#endif
  exit(1);
}

void play_sound1()
{
  load_main_res = 0;
  new_get();
  int s1 = current_value;
#ifdef DEBUG
  printf("play_sound1 %d\n", s1);
#endif
  exit(1);
}

void play_sound3()
{
  load_main_res = 0;
  new_get();
  int num = current_value;
  new_get();
  int s1 = current_value;
  new_get();
  int s2 = current_value;
#ifdef DEBUG
  printf("play_sound3 %d %d %d\n", num, s1, s2);
#endif
  exit(1);
}

void play_sound6()
{
  load_main_res = 0;
  new_get();
  int num = current_value;
  new_get();
  int s1 = current_value;
  new_get();
  int s2 = current_value;
  new_get();
  int s3 = current_value;
  new_get();
  int s4 = current_value;
  new_get();
  int s5 = current_value;
#ifdef DEBUG
  printf("play_sound6 %d %d %d %d %d %d\n", num, s1, s2, s3, s4, s5);
#endif
  exit(1);
}

void play_sound_blanpc()
{
  new_get();
  int s1 = current_value;
  new_get();
  int s2 = current_value;
  new_get();
  int s3 = current_value;
  new_get();
  int s4 = current_value;
#ifdef DEBUG
  printf("play_sound blanpc %d %d %d %d\n", s1, s2, s3, s4);
#endif
  exit(1);
}

void play_synth_gain()
{
  new_get();
  int s1 = current_value;
  new_get();
  int s2 = current_value;
  new_get();
  int s3 = current_value;
  new_get();
  int s4 = current_value;
#ifdef DEBUG
  printf("play synth gain %d %d %d %d ", s1, s2, s3, s4);
#endif
  if (!s4)
    return;
  new_get();
  int s5 = current_value;  
#ifdef DEBUG
  printf("%d\n", s5);
#endif
  exit(1);
}
