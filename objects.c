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

object_table_t *objects_table; /**< таблица объектов */
int max_objects;		/**< максимальное количество объектов */
object_table_t *free_object;	/**< голова списка свободных объектов */
object_table_t *current_object;	/**< текущий объект */
object_t *main_object;		/**< главный объект */
int *saved_sp;			/**< сохраненный указатель стека */
int num_run_objects;		/**< число не спящих объектов */
int main_run;		/**< 1 - во время главной программы, 0 - обработка сообщений */
int find_all_objects = 0;		/**< если 1, то ищутся все объекты по классу */
word objects_list[256];		/**< список объектов для программы, формируется при поиске по классу */
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
    th = t->object;
    if (!th) break;
    printf("table id: %x ", (int)(t - objects_table) * 6);
    printf("id: %x\n", th->id);
    printf("data:\n");
    dump_mem(th->data->data, th->data->size);
    printf("\n");
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
  t->msg_stack = stack_new(h->msg_size);
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
}

/** 
 * Добавление нового объекта
 * Новый объект добавляется в конец списка объектов.
 * Текущий объект становится родителем нового.
 * Новый объект наследует трансформацию перемещения.
 * Новый объект начинает работу
 * @param class образ сценария
 * @param size размер сценария
 * @param translate вектор перемещения
 * 
 * @return 
 */
object_t *object_add(byte *class, int size, vec_t *translate)
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
  memcpy(t->data->data, run_object->data->data, 6); /**< трансформация из текущего объекта копируется в новый объект */
  memcpy(t->data->data + 9, run_object->data->data + 9, 3); /**< копируются 3 байта начиная с 9-го */
  t->parent = run_object;
  // добавления вектора перемещения к началу координат нового объекта
  short *coord = (short *)t->data->data;
  coord[0] += translate->x;
  coord[1] += translate->y;
  coord[2] += translate->z;
  t->current_window = run_object->current_window;
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
	  set_translate((word *)t->data->data);
	  interpret(t, t->class + t->header->msg_handle_entry + 0xa);
	  t->call_stack->sp = saved_sp;
	  sprites_translate((word *)t->data->data);
	}
    if (t->running != 0) {
      if (t->running < 0)
	t->running = 1;
      t->cur_frames_to_skip--;
      if (!t->cur_frames_to_skip) {
	set_translate((word *)t->data->data);
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
	sprites_translate((word *)t->data->data);
	t->cur_frames_to_skip = t->frames_to_skip;
      }
    }
    current_object = current_object->next;
  }
  views_update();
}

/// команда - разрешение обработки сообщений
void object_receive_msg()
{
  run_object->flags &= ~OBJECT_NOHANDLEMSG;
#ifdef DEBUG
  printf("object receive msg flags: %x\n", run_object->flags);
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
#ifdef DEBUG
  printf("send message object: %d count: %d\n", current_value, count);
#endif
  object_t *t = objects_table[current_value / 6].object;
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
#endif
    stack_push(t->msg_stack, current_value);
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
  printf("kill object %x\n", *t->class);
  objects_dump();
#endif
  run_object = t;
  remove_all_sprites(t->sprite_list, remove);
  run_object = rt;
  // освобождение ресурсов
  stack_free(t->call_stack);
  stack_free(t->msg_stack);
  memory_free(t->data);
  object_table_t *tab = objects_table->next;
  object_table_t *prev = objects_table;
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
  stack_t *s = run_object->msg_stack;
  if (stack_empty(run_object->msg_stack)) {
    printf("get message stack is empty\n");
    exit(1); 
    current_value = -1;
  } else {
    current_value = stack_pop(run_object->msg_stack);
    if (stack_empty(run_object->msg_stack))
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

/// Очищает стек сообщений объекта
void object_clear_messages()
{
  stack_clear(run_object->msg_stack);
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
    printf("kill object check = %x\n", t->object->id);
#endif
    exit(1);
    if (t->object->id == id && kill_object_flag)
      object_kill(object_num(t->object), 0);
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
      *pos++ = (word)object_num(t);
#ifdef DEBUG
  printf("object num: %x\n", *pos);
#endif
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
 * Установка номера объекта, которому будут принадлежать новые спрайты
 */
void set_sprites_object()
{
  new_get();
#ifdef DEBUG
  printf("set sprites object: %x prev: %x\n", current_value, run_object->sprites_object);
#endif
  run_object->sprites_object = current_value;
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
    *pos++ = num;
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
 * Устанавливает номер формы для объекта
 */
void obj_set_form()
{
  new_get();
  run_object->form = current_value;
#ifdef DEBUG
  printf("obj set form = %x\n", current_value);
#endif
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
 * Команда - установка слоя для новых объектов.
 * Объекты при отрисовке будут сортироваться по убыванию слоев.
 */
void object_set_layer()
{
  new_get();
#ifdef DEBUG
  printf("set object layer = %x; %d\n", (char)current_value, (char)current_value);
#endif
  run_object->layer = (char)current_value;
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
