/**
 * @file   interpret.c
 * @author alex <alex@localhost>
 * @date   Sun Jan 30 16:45:07 2022
 * 
 * @brief  Интерпретатор - главные функции и цикл
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#include "threads.h"
#include "call.h"
#include "get.h"
#include "sprite.h"
#include "array.h"
#include "render.h"
#include "math.h"
#include "script.h"
#include "interpret.h"
#include "palette.h"
#include "append.h"
#include "image.h"
#include "sound.h"

byte *current_ip;		/**< указатель команд */
thread_t *run_thread;		/**< текущий выполняемый поток */
int interpreting;		/**< 1 - выполняется */
short current_value;		/**< аккумулятор */
short prev_value;		/**< второй регистр */
stack_t stack;			/**< стек для выражений */
int stack_data[MAX_STACK];	/**< буфер стека выражений */
char get_string_buf[MAX_STR];	/**< буфер строки загрузки */
char string_buf2[MAX_STR];
char store_string_buf[MAX_STR];	/**< буфер строки сохранения */
char string_buf4[MAX_STR];
char *get_string;		/**< указатель строки для загрузки */
char *string2;
char *store_string;		/**< указатель строки сохранения */
word *thread_array_pos;		/**< указатель на массив номеров потоков */

/// таблица инструкций
func vm_op[] = {
  null_op,//0
  null_op,//1
  null_op,//2
  null_op,//3
  null_op,//4
  call_byte,//5
  call_word,//6
  call_skip_word,//7
  jump_byte,//8
  jump_word,//9
  jump_skip_word,//a
  null_op,
  null_op,
  null_op,
  null_op,
  null_op,
  null_op,
  saved_return,//11
  jump_byte_z,//12
  jump_word_z,//13
  jump_word_skip_z,//14
  jump_byte_nz,//15
  jump_word_nz,//16
  jump_word_skip_nz,//17
  compare_jump_byte_z,//18
  compare_jump_word_z,//19
  compare_jump_word_skip_z,//1a
  compare_jump_byte_nz,//1b
  compare_jump_word_nz,//1c
  compare_jump_word_skip_nz,//1d
  assign, //1e
  new_get, //1f
  append, //20
  subract, // 21
  null_op, // 22
  null_op, // 23
  nimp, // 24
  nimp, // 25
  nimp, // 26
  nimp, // 27
  nimp, // 28
  array_new, // 29
  set_random_seed, // 2a
  loop_byte, // 2b
  nimp, // 2c
  nimp, // 2d
  nimp, // 2e
  op_jump_table, // 2f
  nimp, // 30
  nimp, // 31
  nimp, // 32
  nimp, // 33
  set_tag, // 34
  nimp, // 35
  null_op, // 36
  null_op, // 37
  nimp, // 38
  nimp, // 39
  nimp, // 3a
  null_op, // 3b
  nimp, // 3c
  free_script, // 3d
  thread_resume, // 3e
  thread_stop_yield_no_saved, // 3f
  run_script, // 40
  nimp, // 41
  yield, // 42
  nimp, // 43
  thread_stop, // 44
  op_script_load, // 45
  scene_new, // 46
  scene_set, // 47
  nimp, // 48
  show_object, // 49
  nimp, // 4a
  clear_object, // 4b
  nimp, // 4c
  nimp, // 4d
  scene_show, // 4e
  nimp, // 4f
  nimp, // 50
  nimp, // 51
  nimp, // 52
  nimp, // 53
  nimp, // 54
  nimp, // 55
  nimp, // 56
  nimp, // 57
  nimp, // 58
  nimp, // 59
  nimp, // 5a
  nimp, // 5b
  nimp, // 5c
  nimp, // 5d
  nimp, // 5e
  nimp, // 5f
  nimp, // 60  
  thread_send_message, // 61
  thread_ready_to_receive, // 62
  nimp, // 63
  thread_receive_msg, // 64
  nimp, // 65
  thread_clear_messages, // 66
  nimp, // 67
  set_palette_from_res, // 68
  nimp, // 69
  set_frames_to_skip, // 6a
  nimp, // 6b
  nimp, // 6c
  play_sound_synth, // 6d
  nimp, // 6e
  nimp, // 6f
  nimp, // 70
  nimp, // 71
  nimp, // 72
  nimp, // 73
  nimp, // 74
  nimp, // 75
  nimp, // 76
  nimp, // 77
  nimp, // 78
  nimp, // 79
  nimp, // 7a
  nimp, // 7b
  nimp, // 7c
  nimp, // 7d
  nimp, // 7e
  nimp, // 7f
  nimp, // 80
  nimp, // 81
  nimp, // 82
  nimp, // 83
  nimp, // 84
  nimp, // 85
  mouse_read, // 86
  nimp, // 87
  nimp, // 88
  nimp, // 89
  nimp, // 8a
  null_op, // 8b
  nimp, // 8c
  nimp, // 8d
  nimp, // 8e
  nimp, // 8f
  nimp, // 90
  nimp, // 91
  nimp, // 92
  nimp, // 93
  script_num_to_thread_num, // 94
  nimp, // 95
  nimp, // 96
  nimp, // 97
  nimp, // 98
  nimp, // 99
  nimp, // 9a
  nimp, // 9b
  nimp, // 9c
  play_sound, // 9d
  nimp, // 9e
  nimp, // 9f
  nimp, // a0
  play_music, // a1
  nimp, // a2
  nimp, // a3
  nimp, // a4
  nimp, // a5
  nimp, // a6
  show_object_flipped, // a7
  nimp, // a8
  nimp, // a9
  nimp, // aa
  nimp, // ab
  nimp, // ac
  nimp, // ad
  nimp, // ae
  nimp, // af
  nimp, // b0
  nimp, // b1
  nimp, // b2
  nimp, // b3
  nimp, // b4
  nimp, // b5
  nimp, // b6
  nimp, // b7
  nimp, // b8
  nimp, // b9
  nimp, // ba
  nimp, // bb
  palette_clear_fade, // bc
  nimp, // bd
  palette_set_fade, //be
  nimp, // bf
  nimp, // c0
  nimp, // c1
  nimp, // c2
  nimp, // c3
  nimp, // c4
  nimp, // c5
  nimp, // c6
  nimp, // c7
  null_op, // c8
  null_op, // c9
  nimp, // ca
  nimp, // cb
  nimp, // cc
  nimp, // cd
  nimp, // ce
  nimp, // cf
  nimp, // d0
  nimp, // d1
  nimp, // d2
  null_op, // d3
  null_op, // d4
  nimp, // d5
  nimp, // d6
  set_skip_palette, // d7
  nimp, // d8
  nimp, // d9
  nimp, // da
  nimp, // db
  clear_all_objects, // dc
  nimp, // dd
  op_thread_kill_remove_all, // de
};

/// Пустая команда
void null_op()
{
}

/// не реализованные команды
void nimp()
{
  printf("Not implemented: %x\n", *(current_ip - 1));
  exit(1);
}

/// Читает очередной байт из кода
byte fetch_byte()
{
  return *current_ip++;
}

/// Читает слово из кода
word fetch_word()
{
  word w= *((word *)current_ip);
  current_ip += 2;
  return w;
}

/** 
 * Выполнение программы сценария
 * 
 * @param ip точка входа
 * 
 * @return указатель команд после выхода из интепретации
 */
byte *interpret(thread_t *t, byte *ip)
{
  byte op;
  current_ip = ip;
  interpreting = 1;
  run_thread = t;
  while (interpreting) {
#ifdef DEBUG
    printf("%04x:\t\t", (int)(current_ip - t->script));
#endif
    op = fetch_byte();
    if (op < sizeof(vm_op) / sizeof(vm_op[0]))
      vm_op[op]();
    else {
      printf("Unknown vm op: %x\n", op);
      exit(1);
    }
    if (interpreting == 2)
      break;
  }
  return current_ip;
}

/** 
 * Прерывание интерпретации
 * передача управления к другому потоку
 */
void yield()
{
  interpreting = 0;
#ifdef DEBUG
  printf("yield\n");
#endif
}

/// Чтение состояния мыши в 3 переменные
void mouse_read()
{
#ifdef DEBUG
  printf("set mouse_x mouse_y buttons:\n");
#endif
  // get mouse x, y, buttons
  current_value = 0;
  switch_string_store();
  current_value = 0;
  switch_string_store();
  current_value = 0;
  switch_string_store();
}
