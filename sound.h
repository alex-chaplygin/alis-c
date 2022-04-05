#include "types.h"

typedef struct {
  byte flags;			/**< флаги канала */
} sound_channel_t;

void sound_init();
void sound_update();
