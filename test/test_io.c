#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "io.h"
#include "test.h"

struct {
  char name[10];
  dword size;
  struct {
    dword adr;
    byte data;
  } assert[20];
} table[] = {
  {"sound0", 0x4ece, {
      {0x4eae, 0x78},
      {0x4ebf, 0},
      {0x185e, 0x6e},
    }
  },
  {"sound1", 0x4ece, {
      {0xb6, 1},
      {0xc6, 0xff},
      {0xc7, 0x95},
      {0x3950, 0x1},
      {0x3960, 0xf9},
      {0x3961, 0xf9},
    }
  },
  {"sound2", 0x2ffff, {
      {0x1bbb4, 0},
      {0x1bbb5, 0xff},
      {0x1bbb6, 0},
      {0x1bbd0, 1},
      {0x1bc6c, 0xe2},
      {0x1bcdc, 0xfa},
      {0x1d685, 0},
      {0x1d686, 0},
      {0x1d688, 2},
      {0x1bba6, 0xe2},
      {0x1bba7, 0x1a},
    }
  },
};

void main()
{
  FILE *f;
  byte *buf;
  for (int i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
    printf("testing %s size=%d\n", table[i].name, table[i].size);
    buf = malloc(table[i].size);
    f = fopen(table[i].name, "rb");
    fread(buf, table[i].size, 1, f);
    printf("unpacking...");
    io_unpack_sound(buf);
    byte *b = buf;
    /*    for (int y = 0; y < table[i].size / 16; y++) {
      printf("%04X: ", (int)(b - buf));
      for (int x = 0; x < 16; x++)
	printf("%02X ", *b++);
      printf("\n");
      }*/
    printf("done\n");
    for (int j = 0; j < 10; j++)
      if (table[i].assert[j].adr) {
	printf("asserting %x %x\n", table[i].assert[j].adr, table[i].assert[j].data);
	ASSERT(buf[table[i].assert[j].adr], table[i].assert[j].data);
      }
    fclose(f);
    free(buf);
  }
}
