/**
 * @file   objects.c
 * @author alex <alex@localhost>
 * @date   Mon Jan 31 09:25:35 2022
 * 
 * @brief  Функции для работы с объектами
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "objects.h"
#include "class.h"
#include "interpret.h"
#include "render.h"
#include "get.h"
#include "store.h"
#include "res.h"
#include "intersection.h"

object_table_t *objects_table; /**< таблица объектов */
int max_objects;		/**< максимальное количество объектов */
object_table_t *free_object;	/**< голова списка свободных объектов */
object_table_t *current_object;	/**< текущий объект */
object_t *main_object;		/**< главный объект */
int *saved_sp;			/**< сохраненный указатель стека */
int num_run_objects;		/**< число не спящих объектов */
int main_run;		/**< 1 - во время главной программы, 0 - обработка сообщений */
int find_all_objects = 0;		/**< если 1, то ищутся все объекты по классу */
word objects_list[512];		/**< список 256 объектов + 256 масок для программы, формируется при поиске по классу */
word *objects_list_pos;		/**< указатель в списке объектов */
int kill_object_flag = 1;		/**< если равен 0, то объекты не удаляются при освобождении класса */

/** 
 * Загрузка blancpc
 * Загрузка главного сценария
 */
void objects_init()
{
  load_blancpc();
  class_load(0, "main.io");
}

/** 
 * Отладочная печать объектов
 */
void objects_dump()
{
  printf("objects: ");
  object_table_t *t = objects_table;
  object_t *th;
  while (t) {
    printf("table id: %x\n", (int)(t - objects_table) * 6);
    th = t->object;
    if (!th) break;
    printf("obj class: %x\n", th->id);
    //    printf("data:\n");
    //    dump_mem(th->data->data, th->data->size);
    //    printf("\n");
    t = t->next;
  }
  printf("free objects: ");
  t = free_object;
  while (t) {
    printf("table id: %x\n", (int)(t - objects_table) * 6);
    t = t->next;
  }
}

/** 
 * Инициализация таблицы объектов
 * Объекты организуются в список, в начале только один главный объект,
 * остальные - свободны
 * @param max - максимум объектов
 */
void objects_init_table(int max)
{
  max_objects = max;
  objects_table = xmalloc(max * sizeof(object_table_t));
  object_table_t *t = objects_table;
  for (int i = 0; i < max_objects - 1; i++) {
    t->next = t + 1;
    t->object = 0;
    t->run_next = 0;
    t++;
  }
  t->next = 0;
  objects_table->next = 0;
  free_object = objects_table + 1; // главный объект сразу запущен
  num_run_objects = 1;
#ifdef DEBUG
  objects_dump();
#endif
  get_string = get_string_buf;
  text_string = text_string_buf;
  store_string = store_string_buf;
  objects_list_pos = objects_list;
}

/** 
 * Настройка объекта для запуска
 * Создается стек вызовов, стек параметров, сегмент данных для 
 * переменных, объект запускается.
 * @param tb запись в таблице объектов
 * @param class загруженный образ класса
 * @param size размер образа
 */
void object_setup(object_table_t *tb, byte *class, int size)
{
  program_t *h = (program_t *)class;
  object_t *t = xmalloc(sizeof(object_t));
  memset(t, 0, sizeof(object_t));
  tb->object = t;
#ifdef DEBUG
  printf("Setup object: id = %x, size = %d, stack = %d, data = %d, msg = %d\n",
	 h->id, size, h->stack_size, h->data_size, h->msg_size);
#endif
  t->call_stack = stack_new(h->stack_size);
  t->msg_queue = xmalloc(h->msg_size * sizeof(int));
  t->msg_size = h->msg_size;
  t->msg_begin = t->msg_end = t->msg_queue;
  t->data = memory_alloc(h->data_size);
  t->ip = class + h->entry + 2;
  t->class = t->class2 = class;
  t->id = h->id;
  t->version = h->version;
  t->frames_to_skip = t->cur_frames_to_skip = 1;
  t->running = -1;
  t->form = -1;
  t->flags = OBJECT_NOHANDLEMSG; // bit 1
  t->header = h;
  t->f2c = 0;
  t->sprites_object = 0;
  t->f26 = -1;
}

/// запуск главного объекта
void object_setup_main(byte *class, int size)
{
  object_setup(objects_table, class, size);
  main_object = objects_table->object;
  main_object->parent = main_object;
}

/** 
 * Добавление нового объекта
 * Новый объект добавляется в конец списка объектов.
 * Текущий объект становится родителем нового.
 * Новый объект создается по координатам относительно родителя.
 * Новый объект начинает работу
 * @param class образ сценария
 * @param size размер сценария
 * @param origin координаты объекта
 * 
 * @return 
 */
object_t *object_add(byte *class, int size, vec_t *origin)
{
  object_table_t *next = current_object->next;
  object_table_t *new_object = free_object;
  current_object->next = new_object;
  free_object = new_object->next;
  new_object->next = next;
  num_run_objects++;
  if (num_run_objects == max_objects) {
    printf("Max objects reached %d\n", max_objects);
    exit(1);
  }
  object_setup(new_object, class, size);
  current_value = (int)(new_object - objects_table) * 6;
  object_t *t = new_object->object;
  memcpy(t->data->data, run_object->data->data, 6); /**< начало координат копируется из родительского новый объект */
  memcpy(t->data->data + 9, run_object->data->data + 9, 3); /**< вектор направления берется из текущего объекта */
  t->parent = run_object;
  // новый объект помещается относительно текущего
  short *coord = (short *)t->data->data;
  coord[0] += origin->x;
  coord[1] += origin->y;
  coord[2] += origin->z;
  t->current_view = run_object->current_view;
  t->f22 = run_object->f22;
  t->sprites_object = current_value;
#ifdef DEBUG
  word *o = (word *)t->data->data;
  byte *o2 = t->data->data + 9;
  printf("Add object %d origin (%d %d %d) data9 (%d %d %d)\n",
	 num_run_objects - 1, o[0], o[1], o[2], o2[0], o2[1], o2[2]);
#endif
}

/** 
 * Команда: создане нового объекта заданного класса
 */
void object_new()
{
  word id = fetch_word();
  int i = class_loaded(id);
  if (i == -1) {
    printf("Class %x (total %d) is not loaded\n", id, total_classes);
    current_value = -1;
    goto store;
  }
#ifdef DEBUG
  printf("new object class = %x\n", id);
#endif
  vec_t vec;
  vec.x = vec.y = vec.z = 0;
  object_t *t = object_add(class_get(i), class_size(i), &vec);
 store:
  switch_string_store();
}

/** 
 * Команда: создане нового объекта заданного класса
 * с координатами из формы
 */
void object_new_at_form()
{
  short origin[3];
  vec_t vec;
  new_get();
  int form = current_value;
  get_form_center(run_object->class, form, origin);
  vec.x = origin[0];
  vec.y = origin[1];
  vec.z = origin[2];
  word id = fetch_word();
  int i = class_loaded(id);
  if (i == -1) {
    printf("Class %x (total %d) is not loaded\n", id, total_classes);
    current_value = -1;
    goto store;
  }
#ifdef DEBUG
  printf("new object class = %x at form %x origin (%x %x %x)\n", id, form, vec.x, vec.y, vec.z);
#endif
  object_t *t = object_add(class_get(i), class_size(i), &vec);
 store:
  switch_string_store();
}

/** 
 * Возвращает начало координат объекта
 * 
 * @param obj объект
 * @param origin куда возвращаются координаты
 */
void object_get_origin(object_t *obj, vec_t *origin)
{
  short *data = (short *)obj->data->data;
  origin->x = *(short *)data++;
  origin->y = *(short *)data++;
  origin->z = *(short *)data;
}

/** 
 * Главный цикл объектов. Для всех объектов с состоянием запуска
 * запускается интерпретатор. Учитывается параметр пропуска
 * кадров перед тем как начать интерпретацию. Всего в объекте может
 * быть 3 сценария: 1-й основной, 2-й - обработка клавиш (запускается
 * всегда), 3-й - обработка сообщений, запускается если установлен соответствующий флаг.
 * После того как все объекты прошли цикл происходит отрисовка спрайтов.
 */
void objects_run()
{
  object_t *t;
  vec_t delta;
  for (current_object = objects_table; current_object; ) {
    t = current_object->object;
#ifdef DEBUG
    printf("Run object %x ip = %x frames_to_skip = %d cur_frames_to_skip = %d running = %x flags = %x\n", t->id, (int)(t->ip - t->class), t->frames_to_skip, t->cur_frames_to_skip, t->running, t->flags);
#endif
    find_all_objects = main_run = 0;
    if (t->flags & OBJECT_MSG) // bit 7
      if (!(t->flags & OBJECT_NOHANDLEMSG)) // bit 1
	if (t->header->msg_handle_entry) {
#ifdef DEBUG
	  printf("starting handle message: %x\n", t->header->msg_handle_entry + 0xa);
#endif
	  saved_sp = t->call_stack->sp;
	  object_get_origin(t, &current_origin);
	  interpret(t, t->class + t->header->msg_handle_entry + 0xa);
	  t->call_stack->sp = saved_sp;
	  object_get_origin(t, &delta);
	  vec_sub(&delta, &current_origin, &delta); 
	  sprites_translate(&delta);
	}
    if (t->running != 0) {
      if (t->running < 0)
	t->running = 1;
      t->cur_frames_to_skip--;
      if (!t->cur_frames_to_skip) {
	object_get_origin(t, &current_origin);
	main_run++;
	t->ip = interpret(t, t->ip);
#ifdef DEBUG
	printf("ip = %x\n", (int)(t->ip - t->class));
#endif
	if (interpreting == 2) {
	  current_object = current_object->next;
	  continue;
	}
	if (t->header->key_entry) {
#ifdef DEBUG
	  printf("starting key_entry: %x\n", t->header->key_entry);
#endif
	  main_run = 0;
	  saved_sp = t->call_stack->sp;
	  interpret(t, t->class + t->header->key_entry + 6);
	  t->call_stack->sp = saved_sp;
	}
	object_get_origin(t, &delta);
	vec_sub(&delta, &current_origin, &delta); 
	sprites_translate(&delta);
	t->cur_frames_to_skip = t->frames_to_skip;
      }
    }
    current_object = current_object->next;
  }
  render_all();
}

/// команда - разрешение обработки сообщений
void object_receive_msg()
{
  run_object->flags &= ~OBJECT_NOHANDLEMSG;
#ifdef DEBUG
  printf("object receive msg flags: %x\n", run_object->flags);
#endif
}

/** 
 * Запрещение обработки сообщений для текущего объекта
 */
void object_disable_handle_msg()
{
  run_object->flags |= OBJECT_NOHANDLEMSG;
#ifdef DEBUG
  printf("object diable handle msg flags: %x\n", run_object->flags);
#endif
}

/// команда - разрешение объекту принимать сообщения
void object_ready_to_receive()
{
  run_object->flags &= ~OBJECT_NOMSG;
#ifdef DEBUG
  printf("object ready to receive msg flags: %x\n", run_object->flags);
#endif
}

/** 
 * Запрещение обработки сообщений
 */
void object_disable_msg()
{
  run_object->flags |= OBJECT_NOMSG;
  object_clear_messages();
#ifdef DEBUG
  printf("object diable msg flags: %x\n", run_object->flags);
#endif
}

/** 
 * Передает сообщение объекту через стек сообщений.
 * Сообщение - это определенное количество данных,
 * которые заносятся в стек.
 */
void object_send_message()
{
  int count = fetch_byte() + 1;
  new_get();
  if (current_value == -1) {
    printf("object_state object == -1\n");
    exit(1);
  }
  object_t *t = objects_table[current_value / 6].object;
#ifdef DEBUG
  printf("send message object: %d class: %x count: %d\n", current_value, *t->class, count);
#endif
  if (!t) {
    printf("object is NULL\n");
    exit(1);
  }
  if (t->flags & OBJECT_NOMSG) {
    printf("object flag = no send\n");
    exit(1);
  }
  while (count--) {
    new_get();
#ifdef DEBUG
    printf("param = %x; %d\n", current_value, current_value);
    printf("msg_end pos = %d\n", (int)(t->msg_end - t->msg_queue));
#endif
    if (t->msg_end == t->msg_queue + t->msg_size)
      t->msg_end = t->msg_queue;
    *t->msg_end++ = current_value;
  }
  t->flags |= OBJECT_MSG;
#ifdef DEBUG
    printf("flags = %x\n", t->flags);
#endif
}

/** 
 * Удаление объекта
 * 
 * @param num номер объекта * 6
 * @param remove если 1, то полное удаление спрайтов
 */
void object_kill(int num, int remove)
{
  if (num < 0)
    return;
  object_t *t = objects_table[num / 6].object;
  if (!t)
    return;
  object_t *rt = run_object;
#ifdef DEBUG
  printf("kill object num %x class %x\n", num, *t->class);
#endif
  run_object = t;
  remove_all_sprites(t->sprite_list, remove);
  run_object = rt;
  // освобождение ресурсов
  stack_free(t->call_stack);
  free(t->msg_queue);
  memory_free(t->data);
  object_table_t *tab = objects_table->next;
  object_table_t *prev = objects_table;
  while (tab) {
    if (tab->object->parent == t)
      tab->object->parent = -1; // если был родителем для объектов, то очищаем
    tab = tab->next;
  }
  tab = objects_table->next;
  while (tab) {
    if (tab->object == t)
      break;
    prev = tab;
    tab = tab->next;
  }
  num_run_objects--;
  prev->next = tab->next;
  free(tab->object);
  tab->object = 0;
  tab->next = free_object;
  free_object = tab;
  if (run_object == t) {
    interpreting = 2;
    // должен запуститься следующий объект
    current_object = prev;
  }
}

/** 
 * Удаление объекта, номер - параметр
 * 
 * @param remove если 1, то полное удаление спрайтов
 */
void op_object_kill(int remove)
{
  save_get();
  if (current_value <= 0) {
    printf(" object <= 0\n");
    exit(1);
    return;
  }
  object_kill(current_value, remove);
}

/// удаление объекта с очисткой всех спрайтов
void op_object_kill_remove_all()
{
#ifdef DEBUG
  printf("object kill remove all\n");
#endif
  op_object_kill(1);
}

/** 
 * Читает сообщение для текущего объекта
 * Сбрасывает флаг, если больше нет сообщений
 */
void object_get_message()
{
#ifdef DEBUG
  printf("msg_begin pos = %d\n", (int)(run_object->msg_begin - run_object->msg_queue));
#endif
  if (run_object->msg_begin == run_object->msg_end) {
    printf("get message queue is empty\n");
    current_value = -1;
  } else {
    if (run_object->msg_begin == run_object->msg_queue + run_object->msg_size)
      run_object->msg_begin = run_object->msg_queue;
    current_value = *(short *)run_object->msg_begin++;
    if (run_object->msg_begin == run_object->msg_end)
      run_object->flags &= ~OBJECT_MSG;
  }
  #ifdef DEBUG
  printf("get_message: %x; %d\n", current_value, current_value);
  printf("flags = %x\n", run_object->flags);
  #endif
}

/** 
 * Пристановка выполнения программы объекта
 */
void object_pause()
{
#ifdef DEBUG
  printf("object pause no saved yield\n");
#endif
  run_object->running = 0;
  if (main_run)
    yield();
}

/// Очищает очередь сообщений объекта
void object_clear_messages()
{
  run_object->msg_begin = run_object->msg_end = run_object->msg_queue;
  run_object->flags &= ~OBJECT_MSG;
#ifdef DEBUG
  printf("clear messages flags = %x\n", run_object->flags);
#endif
}

/** 
 * Удаляет все объекты класса
 * 
 * @param id номер класса
 */
void objects_kill_by_class(int id)
{
  int i;
  for (object_table_t *t = objects_table->next; t;) {
#ifdef DEBUG
    printf("kill object check = %x id = %x\n", t->object->id, id);
#endif
    if (t->object->id == id && kill_object_flag)
      object_kill(object_num(t->object), 1);
    t = t->next;
  }
}

/** 
 * Останавливает текущий объект
 * Если текущий объект - главный, то - выход
 */
void object_stop()
{
#ifdef DEBUG
  printf("object_stop\n");
  printf("run object = %x\n", *run_object->class);
#endif
  if (run_object == objects_table->object)
    exit(0);
  object_kill(object_num(run_object), 0);
}

/** 
 * Возобновляет выполнение программы объекта
 * Параметр - номер объекта
 */
void object_resume()
{
  new_get();
  if (current_value < 0)
    return;
  int thr = current_value;
  if (thr % 6 != 0) {
    printf("object resume invalid object: %x\n", thr);
    exit(1);
  }
  object_t *t = objects_table[thr / 6].object;
#ifdef DEBUG
  printf("resume object %x num = %x\n", *t->class, current_value);
#endif
  t->running = 1;
}

/** 
 * Вычисляет номер объекта в таблице объектов * 6
 * 
 * @param t указатель объекта
 * 
 * @return номер в таблице * 6
 */
int object_num(object_t *t)
{
  object_table_t *tab = objects_table;
  for (int i = 0; tab; i++, tab++)
    if (tab->object == t)
      return i * 6;
  return -1;
}

/** 
 * Сохраняет очередной элемент списка объектов в переменную
 */
void object_store_next()
{
  current_value = *(short *)objects_list_pos;
  if (current_value >= 0)
    objects_list_pos++;
  #ifdef DEBUG
  printf("store next object num: %x\n", current_value);
  #endif
  exchange_strings_store();
}

/** 
 * Находит объекты заданного класса и помещает в список
 */
void objects_find_by_class()
{
  int num = fetch_word();
  object_table_t *tab = objects_table;
  object_t *t;
  word *pos = objects_list;
#ifdef DEBUG
  printf("objects find by class: %x\n", num);
#endif
  while (tab) {
    t = tab->object;
#ifdef DEBUG
    printf("check object num: %x run_object: %x\n", t->id, run_object->id);
#endif
    if (t->id == num && t != run_object) {
      *pos = (word)object_num(t);
      pos[256] = 0; // маска поиска
#ifdef DEBUG
      printf("object num: %x\n", *pos);
#endif
      pos++;
      if (!find_all_objects)
	break;
    }
    tab = tab->next;
  }
  *pos = (short)-1;
  find_all_objects = 0;
  objects_list_pos = objects_list;
  object_store_next();
}

void object_set_f25()
{
#ifdef DEBUG
  printf("object set f25 -1\n");
#endif
  run_object->f25 = -1;
}

/** 
 * Установка начала координат для текущего объекта
 */
void object_set_origin()
{
  new_get();
  seg_write_word(run_object->data, 0, current_value);
  new_get();
  seg_write_word(run_object->data, 2, current_value);
  new_get();
  seg_write_word(run_object->data, 4, current_value);
#ifdef DEBUG
  short *w = (short *)run_object->data->data;
  printf("object set origin: (%d %d %d)\n", w[0], w[1], w[2]);
#endif
}

/** 
 * Перемещение начала координат для текущего объекта
 */
void object_move_origin()
{
  short d;
  new_get();
  d = *(short *)seg_read(run_object->data, 0);
  seg_write_word(run_object->data, 0, current_value + d);
  new_get();
  d = *(short *)seg_read(run_object->data, 2);
  seg_write_word(run_object->data, 2, current_value + d);
  new_get();
  d = *(short *)seg_read(run_object->data, 4);
  seg_write_word(run_object->data, 4, current_value + d);
#ifdef DEBUG
  short *w = (short *)run_object->data->data;
  printf("move coord origin: (%d %d %d)\n", w[0], w[1], w[2]);
#endif
}

/** 
 * Загружает список номеров всех объектов кроме главного
 * Первый элемент списка сохраняет в переменную
 */
void objects_get_all()
{
  object_table_t *tab = objects_table->next;
  object_t *t;
  word *pos = objects_list;
  int num;
#ifdef DEBUG
  printf("get objects list:\n");
#endif
  while (tab) {
    t = tab->object;
    num = object_num(t);
    *pos = num;
    pos[256] = 0;
    pos++;
#ifdef DEBUG
    printf("%x ", num);
#endif
    tab = tab->next;
  }
#ifdef DEBUG
    printf("\n");
#endif
  *pos = -1;
  find_all_objects = 0;
  objects_list_pos = objects_list;
  object_store_next();
}

/** 
 * Устанавливает признак поиска по классу: поиск всех объектов
 */
void set_find_all_objects()
{
  find_all_objects = 1;
#ifdef DEBUG
  printf("find all objects = 1\n");
#endif
}

/** 
 * Удаление объекта без удаления спрайтов из окна
 */
void object_kill_no_remove()
{
#ifdef DEBUG
  printf("kill object\n");
#endif
  op_object_kill(0);
}

/** 
 * Остановка выполнения объекта, вход - указатель на объект
 */
void object_pause_by_ref()
{
  new_get();
#ifdef DEBUG
  printf("object pause ref = %x\n", current_value);
#endif
  if (!current_value)
    return;
  object_t *t = objects_table[current_value / 6].object;
#ifdef DEBUG
  printf("object pause class = %x\n", *t->class);
#endif
  t->running = 0;
}

/** 
 * Сброс отражения для новых спрайтов текущего объекта
 */
void object_set_flip0()
{
#ifdef DEBUG
  printf("object set flip = 0\n");
#endif
  run_object->x_flip = 0;
}

/** 
 * Новые спрайты текущего объекта будут отражены
 */
void object_set_flip1()
{
#ifdef DEBUG
  printf("object set flip = 1\n");
#endif
  run_object->x_flip = 1;
}

/** 
 * Меняет отражение объекта на противоположное
 */
void object_switch_flip()
{
#ifdef DEBUG
  printf("object switch flip\n");
#endif
  run_object->x_flip ^= 1;
}

/** 
 * Движение объекта по вектору скорости
 */
void object_move()
{
#ifdef DEBUG
  printf("apply speed\n");
#endif
  short *org = (short *)run_object->data->data;
  char *speed = (char *)&run_object->data->data[9];
  org[0] += speed[0];
  org[1] += speed[1];
  org[2] += speed[2];
}

/** 
 * Устанавливает таблицу маршрутов у текущего объекта
 */
void object_set_path_table()
{
  int pt = fetch_word();
#ifdef DEBUG
  printf("object set path table: %x\n", pt);
#endif
  run_object->path_table= pt;
}
