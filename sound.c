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
#define DEFAULT_PRIORITY -128

typedef struct {
  byte flags;			/**< флаги канала */
  char priority;		/**< приоритет канала, звучит только канал с максимальным приоритетом */
  char synth;			/**< если больше 0, то канал синтезатора */
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

void  add_sound(int pri, int obj)
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
  ch = sound_channels;
  for (int i = 0; i < NUM_CHANNELS; i++, ch++) {
  }
}

/** 
 * Проигрывание звука через синтезатор (заглушка)
 */
void play_sound_synth()
{
  extern int freq;
  extern int slen;
  new_get();
  int pri = (char)current_value;
  new_get();
  int vol = (char)current_value;
  new_get();
  freq = current_value;
  new_get();
  slen = current_value;
#ifdef DEBUG
  printf("play_sound_synth pri=%d vol=%d freq=%d len=%d\n", pri, vol, freq, slen);
#endif
  if (!slen)
    return;
  //  int s = (s2 << 8) % slen;
  add_sound(pri, object_num(run_object));
  exit(1);
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
  extern int freq;
  extern int slen;
  new_get();
  int s1 = current_value;
  new_get();
  int s2 = current_value;
  new_get();
  int s3 = freq = current_value;
  new_get();
  int s4 = slen = current_value;
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
