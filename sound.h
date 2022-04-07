#include "types.h"

typedef struct {
  byte flags;			/**< флаги канала */
  char priority;		/**< приоритет канала, звучит только канал с максимальным приоритетом */
  char synth;			/**< если больше 0, то канал синтезатора */
  word length;		/**< число кадров - длительность звучания ноты */
  short volume;			/**< громкость звучания ноты */
  short gain;			/**< смещение громкости (усиление и затухание звука во времени) ноты */
  short frequency;		/**< частота ноты */
  short frequency_gain;		/**< смещение частоты ноты во времени */
} sound_channel_t;

void sound_init();
void sound_update();
