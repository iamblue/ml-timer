#include <string.h>

#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"

#include "microlattice.h"

#include "jerry-api.h"
#include "hal_gpt.h"

#define  TIMER_32K_COUNT   (32*500)                     //32K unit,500ms
#define  SYSTICK_COUNT     (2000/portTICK_PERIOD_MS)    //2000ms

static uint32_t get_current_milisecond()
{
    hal_gpt_status_t ret;
    uint32_t count = 0;
    ret = hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    return (count * 1000) / 32768;
}

DELCARE_HANDLER(__getTime)
{
  uint32_t time_record = get_current_milisecond();
  printf("time_record : %d\n", time_record);
  return jerry_create_number(time_record);
}

static int serial_nubmer = 0;

void setTimeOutTimerCallback( TimerHandle_t pxTimer ) {
  jerry_value_t *arg_clone_timer = pvTimerGetTimerID(pxTimer);

  jerry_value_t this_val = jerry_create_undefined();
  jerry_value_t ret_val = jerry_call_function (arg_clone_timer[0], this_val, NULL, 0);

  const args_cnt = 2;

  for(int i = 0; i < args_cnt; ++i)
  {
    jerry_release_value(arg_clone_timer[i]);
  }

  xTimerDelete(pxTimer, portMAX_DELAY);

  free(arg_clone_timer);
  arg_clone_timer = NULL;
}

DELCARE_HANDLER(setTimeout)
{
  serial_nubmer = serial_nubmer + 1;
  assert(args_cnt == 2);
  const size_t array_size = sizeof(jerry_value_t) * args_cnt;
  jerry_value_t *arg_clone = malloc(array_size);
  memcpy(arg_clone, args_p, array_size);

  TimerHandle_t setTimeout_timer;

  char task_name[25] = {0};

  snprintf(task_name, 25, "TimerMain%d", serial_nubmer);

  setTimeout_timer = xTimerCreate(task_name, (jerry_get_number_value((int)arg_clone[1]) / portTICK_RATE_MS), pdFALSE, arg_clone, setTimeOutTimerCallback);
  xTimerStart( setTimeout_timer, 0 );

  return jerry_create_number((unsigned int) setTimeout_timer);;
}

DELCARE_HANDLER(__loop)
{
  if (args_cnt == 2 && jerry_value_is_object(args_p[0])) {
    for (;;) {
      hal_gpt_delay_ms(jerry_get_number_value(args_p[1]));
      jerry_value_t this_val = jerry_create_undefined();
      jerry_value_t ret_val = jerry_call_function (args_p[0], this_val, NULL, 0);
    }
  }
  return jerry_create_boolean(true);
}

DELCARE_HANDLER(clearTimeout)
{
  TimerHandle_t xTimer = (TimerHandle_t) (int) jerry_get_number_value(args_p[0]);
  jerry_value_t *arg_clone_timer = pvTimerGetTimerID(xTimer);

  for(int i = 0; i < args_cnt; ++i)
  {
    jerry_release_value(arg_clone_timer[i]);
  }

  free(arg_clone_timer);
  arg_clone_timer = NULL;

  xTimerDelete(xTimer, portMAX_DELAY);
  return true;
}


void setIntervalTimerCallback( TimerHandle_t pxTimer )
{
  jerry_value_t *arg_clone_timer = pvTimerGetTimerID(pxTimer);

  jerry_value_t this_val = jerry_create_undefined();
  jerry_value_t ret_val = jerry_call_function (arg_clone_timer[0], this_val, NULL, 0);
}

DELCARE_HANDLER(setInterval)
{
  serial_nubmer = serial_nubmer + 1;
  assert(args_cnt == 2);
  const size_t array_size = sizeof(jerry_value_t) * args_cnt;
  jerry_value_t *arg_clone = malloc(array_size);
  memcpy(arg_clone, args_p, array_size);

  TimerHandle_t setTimeout_timer;

  char task_name[25] = {0};

  snprintf(task_name, 25, "TimerMain%d", serial_nubmer);

  setTimeout_timer = xTimerCreate(task_name, (jerry_get_number_value((int)arg_clone[1]) / portTICK_RATE_MS), pdTRUE, arg_clone, setIntervalTimerCallback);
  xTimerStart( setTimeout_timer, 0 );

  return jerry_create_number((unsigned int) setTimeout_timer);
}

DELCARE_HANDLER(clearInterval)
{
  TimerHandle_t xTimer = (TimerHandle_t) (int) jerry_get_number_value(args_p[0]);
  jerry_value_t *arg_clone_timer = pvTimerGetTimerID(xTimer);

  const _args_cnt = 2;

  for(int i = 0; i < _args_cnt; ++i)
  {
    jerry_release_value(arg_clone_timer[i]);
  }

  free(arg_clone_timer);
  arg_clone_timer = NULL;

  xTimerDelete(xTimer, portMAX_DELAY);
  return true;
}

void ml_timer_init(void)
{
  REGISTER_HANDLER(__loop);
  REGISTER_HANDLER(__getTime);
  REGISTER_HANDLER(setTimeout);
  REGISTER_HANDLER(clearTimeout);
  REGISTER_HANDLER(setInterval);
  REGISTER_HANDLER(clearInterval);
}