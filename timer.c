/**
 * @file   timer.c
 * @author alex <alex@localhost>
 * @date   Fri Mar 18 08:09:58 2022
 * 
 * @brief  Системный таймер - вызывается каждый кадр асинхронно
 * 
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "types.h"
#include "palette.h"
#include "mouse.h"

/** 
 * Обработчик таймера.
 * Обновляет палитру.
 * @param i 
 */
void timer_handler(int i)
{
  palette_update();
  draw_mouse_cursor();
}

/** 
 * Инициализация таймера
 * 
 * @param time_step интервал таймера
 */
void init_timer(long time_step)
{
  struct sigaction sa;
  struct itimerval timer;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &timer_handler;
  sigaction(SIGVTALRM, &sa, NULL);
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = time_step * 1000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = time_step * 1000;
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
}
