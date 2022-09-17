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
#include <math.h>
#include <SDL.h>
#include "interpret.h"
#include "get.h"
#include "res.h"
#include "objects.h"

#define NUM_CHANNELS 4		/**< число каналов звука */
#define MAX_VOLUME 32767	/**< порог громкости */
#define DEFAULT_PRIORITY -128
#define CHANNEL_OFF (1 << 7)	/**< флаг - канал выключен */
#define CHANNEL_2 (1 << 1)
#define SOUND_SYNTH 1		/**< тип звука - синтезируемый */
#define SOUND_3 3
#define SOUND_DIGITAL 0x80	/**< тип звука - оцифрованный */

typedef struct {
  byte flags;			/**< флаги канала */
  char priority;		/**< приоритет канала, звучит только канал с максимальным приоритетом */
  char type;			/**< если больше 0, то канал синтезатора */
  word length;		/**< число кадров - длительность звучания ноты */
  short volume;			/**< громкость звучания ноты */
  short gain;			/**< смещение громкости (усиление и затухание звука во времени) ноты */
  short frequency;		/**< частота ноты */
  short frequency_gain;		/**< смещение частоты ноты во времени */
  int object;			/**< номер объекта, который послал звук */
} sound_channel_t;

sound_channel_t  sound_channels[NUM_CHANNELS];	/**< каналы звука */
int volume;			/**< текущая громкость */
int frequency;			/**< текущая частота */
int frequency_gain;		/**< изменение частоты */
int sound_length;		/**< длина в кадрах текущего звука */
int gain;			/**< изменение громкости звука */
int sound_type;			/**< тип текущего звука */
int sound_pos = 0;		/**< позиция текущего звука */
byte *sound_data;		/**< цифровые данные звука */

void audio_update(void*  userdata, Uint8* stream, int len)
{
  if (sound_type == 0) {
    SDL_memset(stream, 0, len);
    sound_pos = 0;
  } else {
    SDL_memset(stream, 0, len);
    printf("SOUND DATA type=%x freq = %d sound_length = %d vol = %x\n", sound_type, frequency, sound_length, volume);
    for (int i = 0; i < len; i++) {
      if (sound_type == SOUND_SYNTH)
	stream[i] = (Uint8)((128 + volume * sin((i + sound_pos) * frequency / 8000.0)));
      else if (sound_type == SOUND_DIGITAL) {
	if (i + sound_pos < sound_length)
	  stream[i] = (Uint8)(sound_data[i + sound_pos]);
      }
      printf("%d ", stream[i]);
    }
    sound_pos += len;
    printf("\n");
  }
}

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
 * Инициализация звука
 */
void audio_init()
{
  SDL_AudioSpec want, have;

  SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
  want.freq = 8000;
  want.format = AUDIO_U8;
  want.channels = 1;
  want.samples = 4096;
  want.callback = audio_update; /* you wrote this function elsewhere -- see SDL_AudioSpec for details */
  if (SDL_OpenAudio(&want, &have) < 0) {
    SDL_Log("Failed to open audio: %s", SDL_GetError());
    exit(1);
  }
  if (have.format != want.format) {
    SDL_Log("We didn't get audio format. freq = %d format = %d\n", have.freq, have.format);
    exit(1);
  }
  SDL_PauseAudio(0); /* start audio playing. */
  init_sound_channels();
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
 * @param type готовность
 * @param vol громкость
 * @param len длительность
 */
void  add_sound(int pri, int vol, int obj, int type, int len)
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
      goto add;
    }
  }
  ch = find_minpri_channel();
  if (pri < 0 || !ch->priority || ch->priority == DEFAULT_PRIORITY) {
    if (ch->priority > pri)
      return;
  add:    
    ch->flags = CHANNEL_OFF;
    ch->priority = pri;
    ch->type = type;
    ch->frequency = frequency;
    ch->frequency_gain = frequency_gain;
    ch->volume = vol;
    ch->gain = gain;
    ch->length = sound_length;
    ch->object = obj;
    ch->flags = CHANNEL_2;
    /*    if (type & SOUND_DIGIT) {
      printf("type & 0x80\n");
      exit(1);
      }*/
#ifdef DEBUG
    printf("adding sound channel num = %d\n", (int)(ch - sound_channels));
    printf("flags=%x pri=%x type=%x freq=%x fgain=%x, vol=%x, gain=%x len=%x obj=%x\n", ch->flags, ch->priority, ch->type, ch->frequency, ch->frequency_gain, ch->volume, ch->gain, ch->length, ch->object);
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
  volume = ch->volume;
  frequency = 32 * 55000 / ((ch->frequency >> 3) + 320);
  sound_type = ch->type;
  sound_pos = 0;
  printf("vol = %d freq = %d type = %d\n", volume, frequency, sound_type);
  // если type == 0, то нота выключается
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
  ch->type = 0;
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
  int vol;
  int channel_num = ch - sound_channels;
  if (ch->type & SOUND_DIGITAL)
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
 * Проигрывание звука через синтезатор
 */
void play_sound_synth()
{
  new_get();
  int pri = (char)current_value;
  new_get();
  volume = (char)current_value;
  new_get();
  frequency = current_value;
  new_get();
  sound_length = current_value;
#ifdef DEBUG
  printf("play_sound_synth pri=%d vol=%d freq=%d len=%d\n", pri, volume, frequency, sound_length);
#endif
  if (!sound_length)
    return;
  gain = (volume << 8) / sound_length;
  if (gain)
    gain = 1;
  gain = -gain;
  frequency_gain = 0;
  sound_type = SOUND_SYNTH;
  add_sound(pri, volume, object_num(run_object), sound_type, sound_length);
}

/** 
 * Проигрывание звука из ресурса
 */
void play_sound()
{
  load_main_res = 0;
  new_get();
  int num = current_value;
  new_get();
  int pri = current_value;
  new_get();
  int vol = current_value;
  new_get();
  int s1 = current_value;
  new_get();
  int s2 = current_value;
#ifdef DEBUG
  printf("play_sound_res num=%d pri=%d vol=%d %d %d\n", num, pri, vol, s1, s2);
#endif
  sound_t *s = (sound_t *)res_get_sound(num);
  if (!s2)
    s2 = s->b2;
  sound_length = s->length - 16;//  длина данных, начало = заголовок + 16
  sound_pos = 0;
  sound_data = (byte *)s + 16;
  sound_type = SOUND_DIGITAL;
  s2 * 1000;
  add_sound(pri, vol, object_num(run_object), sound_type, sound_length);
}

void sound_flush()
{
#ifdef DEBUG
  printf("sound flush\n");
#endif
}

void play_sound1()
{
  load_main_res = 0;
  new_get();
  int s1 = current_value;
#ifdef DEBUG
  printf("play_sound1 %d\n", s1);
#endif
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

/** 
 * Проигрывание звука из blancpc
 */
void play_sound_blanpc()
{
  new_get();
  int pri = current_value;
  new_get();
  int vol = volume = current_value;
  new_get();
  frequency = current_value;
  new_get();
  sound_length = current_value;
#ifdef DEBUG
  printf("play_sound blanpc %d %d %d %d\n", pri, vol, frequency, sound_length);
#endif
  if (!sound_length)
    return;
  gain = (volume << 8) / sound_length;
  if (gain)
    gain = 1;
  gain = -gain;
  frequency_gain = 0;
  sound_type = SOUND_3;
  int s2 = (32767 - frequency) / 1638 + 1;
  if (s2 > 20)
    s2 = 20;
  int s1 = 1;
  extern byte *blancpc_data;
  sound_data = blancpc_data + 16;
  sound_type = SOUND_DIGITAL;
  s2 * 1000;
  add_sound(pri, vol, object_num(run_object), sound_type, sound_length);
}

/** 
 * Проигрывание синтезированного звука с изменением частоты
 */
void play_synth_gain()
{
  new_get();
  int pri = current_value;
  new_get();
  int vol = current_value;
  new_get();
  frequency = current_value;
  new_get();
  sound_length = current_value;
#ifdef DEBUG
  printf("play synth gain pri=%d vol=%d freq=%d len=%d ", pri, vol, frequency, sound_length);
#endif
  if (!sound_length)
    return;
  new_get();
  frequency_gain = current_value;
  gain = 0;
#ifdef DEBUG
  printf(" fgain=%d\n", frequency_gain);
#endif
  sound_type = SOUND_SYNTH;
  add_sound(pri, vol, object_num(run_object), sound_type, sound_length);
}
