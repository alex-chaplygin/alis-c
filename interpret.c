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
#include "objects.h"
#include "call.h"
#include "get.h"
#include "sprite.h"
#include "array.h"
#include "render.h"
#include "math.h"
#include "class.h"
#include "palette.h"
#include "append.h"
#include "sound.h"
#include "text.h"
#include "file.h"
#include "mouse.h"
#include "graphics.h"
#include "intersection.h"
#include "res.h"
#include "path.h"
#include "misc.h"

byte *current_ip;		/**< указатель команд */
object_t *run_object;		/**< текущий выполняемый поток */
int interpreting;		/**< 1 - выполняется */
short current_value;		/**< аккумулятор */
short prev_value;		/**< второй регистр */
stack_t stack;			/**< стек для выражений */
int stack_data[MAX_STACK];	/**< буфер стека выражений */
char get_string_buf[MAX_STR];	/**< буфер строки загрузки */
char text_string_buf[MAX_STR];
char store_string_buf[MAX_STR];	/**< буфер строки сохранения */
char *get_string;		/**< указатель строки для загрузки */
char *text_string;		/**< указатель на строку для вывода */
char *store_string;		/**< указатель строки сохранения */

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
  loop_word, // 2c
  nimp, // 2d
  op_switch_case, // 2e
  op_jump_table, // 2f
  nimp, // 30
  call_word_resume, // 31
  call_skip_word_resume, // 32
  nimp, // 33
  set_tag, // 34
  object_pause_by_ref, // 35
  null_op, // 36
  null_op, // 37
  nimp, // 38
  nimp, // 39
  nimp, // 3a
  null_op, // 3b
  object_new_at_form, // 3c
  class_free, // 3d
  object_resume, // 3e
  object_pause, // 3f
  object_new, // 40
  object_kill_no_remove, // 41
  yield, // 42
  nimp, // 43
  object_stop, // 44
  op_class_load, // 45
  view_new, // 46
  view_set, // 47
  sprite_show_0, // 48
  sprite_show, // 49
  nimp, // 4a
  sprites_clear_with_tag, // 4b
  object_set_origin, // 4c
  object_move_origin, // 4d
  view_show, // 4e
  view_hide, // 4f
  sprites_clear_all, // 50
  set_form, // 51
  clear_form, // 52
  find_intersection_list_vector_cur_form, // 53
  nimp, // 54
  find_intersection_list_vector, // 55
  find_intersection_list_point, // 56
  object_store_next, // 57
  nimp, // 58
  nimp, // 59
  nimp, // 5a
  nimp, // 5b
  nimp, // 5c
  nimp, // 5d
  nimp, // 5e
  path_set, // 5f
  nimp, // 60  
  object_send_message, // 61
  object_ready_to_receive, // 62
  object_disable_msg, // 63
  object_receive_msg, // 64
  object_disable_handle_msg, // 65
  object_clear_messages, // 66
  set_find_all_objects, // 67
  set_palette_from_res, // 68
  nimp, // 69
  set_frames_to_skip, // 6a
  play_synth_gain, // 6b
  play_sound_blanpc, // 6c
  play_sound_synth, // 6d
  skip4, // 6e
  array_set, // 6f
  op_open_file, // 70
  op_close_file, // 71
  nimp, // 72
  nimp, // 73
  file_read_word, // 74
  file_write_word, // 75
  nimp, // 76
  op_read_file, // 77
  op_write_file, // 78
  nimp, // 79
  nimp, // 7a
  nimp, // 7b
  nimp, // 7c
  set_color, // 7d
  nimp, // 7e
  nimp, // 7f
  nimp, // 80
  nimp, // 81
  nimp, // 82
  set_sprites_object, // 83
  show_mouse_cursor, // 84
  hide_mouse_cursor, // 85
  mouse_read, // 86
  set_mouse_cursor, // 87
  nimp, // 88
  jump3, // 89
  object_set_path_table, // 8a
  null_op, // 8b
  path_find_shortest, // 8c
  nimp, // 8d
  find_intersection_list_path_cur_form, // 8e
  find_intersection_list_path, // 8f
  object_move, // 90
  nimp, // 91
  nimp, // 92
  nimp, // 93
  objects_find_by_class, // 94
  play_sound6, // 95
  play_sound1, // 96
  nimp, // 97
  nimp, // 98
  object_switch_flip, // 99
  object_set_flip1, // 9a
  object_set_flip0, // 9b
  objects_get_all, // 9c
  play_sound, // 9d
  nimp, // 9e
  nimp, // 9f
  object_set_f25, // a0
  play_music, // a1
  nimp, // a2
  nimp, // a3
  nimp, // a4
  find_intersection_list_cur_obj, // a5
  nimp, // a6
  sprite_show_flipped, // a7
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
  print_number, // b8
  print_get_string, // b9
  set_text_params, // ba
  skip1, // bb
  palette_clear_fade, // bc
  nimp, // bd
  palette_set_fade, //be
  set_text_pos, // bf
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
  play_sound3, // cd
  nimp, // ce
  set_sprites_layer, // cf
  nimp, // d0
  nimp, // d1
  nimp, // d2
  null_op, // d3
  null_op, // d4
  new_get, // d5
  nimp, // d6
  set_skip_palette, // d7
  get2switch, // d8
  nimp, // d9
  nimp, // da
  nimp, // db
  sprites_clear_all_view, // dc
  sprites_clear_with_tag_view, // dd
  op_object_kill_remove_all, // de
  view_set_param, //df
  nimp, //e0
  get_last_object_mask, //e1
  delete_image, //e2
};

/// Пустая команда
void null_op()
{
}

/// не реализованные команды
void nimp()
{
  printf("Not implemented: %x\n", *(current_ip - 1));
  objects_dump();
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
byte *interpret(object_t *t, byte *ip)
{
  byte op;
  current_ip = ip;
  interpreting = 1;
  run_object = t;
  while (interpreting) {
    graphics_get_events();
    graphics_palette_update();
#ifdef DEBUG
    printf("%02x:%04x:\t%d\t", *t->class, (int)(current_ip - t->class), main_run);
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
