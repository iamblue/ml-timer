#include <string.h>

#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"

#include "microlattice.h"

#include "jerry.h"
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
  ret_val_p->type = JERRY_API_DATA_TYPE_FLOAT64;
  ret_val_p->v_float64 = time_record;

  return true;
}
static int serial_nubmer = 0;

void setTimeOutTimerCallback( TimerHandle_t pxTimer ) {
  jerry_api_value_t *arg_clone_timer = pvTimerGetTimerID(pxTimer);
  jerry_api_call_function(arg_clone_timer[0].v_object, NULL, false, NULL, 0);
  // release object and string, if any
  const args_cnt = 2;

  for(int i = 0; i < args_cnt; ++i)
  {
    switch(arg_clone_timer[i].type)
    {
      case JERRY_API_DATA_TYPE_OBJECT:
          jerry_api_release_object(arg_clone_timer[i].v_object);
          break;
      case JERRY_API_DATA_TYPE_STRING:
          jerry_api_release_string(arg_clone_timer[i].v_string);
          break;
    }
  }

  xTimerDelete(pxTimer, portMAX_DELAY);

  free(arg_clone_timer);
  arg_clone_timer = NULL;

}

DELCARE_HANDLER(setTimeout)
{
  serial_nubmer = serial_nubmer + 1;
  assert(args_cnt == 2);
  const size_t array_size = sizeof(jerry_api_value_t) * args_cnt;
  jerry_api_value_t *arg_clone = malloc(array_size);
  memcpy(arg_clone, args_p, array_size);

  // acquire object and string, if any
  for(int i = 0; i < args_cnt + 1; ++i)
  {
    switch(arg_clone[i].type)
    {
      case JERRY_API_DATA_TYPE_OBJECT:
          arg_clone[i].v_object = jerry_api_acquire_object(arg_clone[i].v_object);
          break;
      case JERRY_API_DATA_TYPE_STRING:
          arg_clone[i].v_string = jerry_api_acquire_string(arg_clone[i].v_string);
          break;
    }
  }
  TimerHandle_t setTimeout_timer;

  char task_name[25] = {0};

  snprintf(task_name, 25, "TimerMain%d", serial_nubmer);

  setTimeout_timer = xTimerCreate(task_name, ((int)arg_clone[1].v_float32 / portTICK_RATE_MS), pdFALSE, arg_clone, setTimeOutTimerCallback);
  xTimerStart( setTimeout_timer, 0 );

  ret_val_p->type = JERRY_API_DATA_TYPE_UINT32;
  ret_val_p->v_uint32 = (unsigned int) setTimeout_timer;

  return true;
}

DELCARE_HANDLER(__loop)
{
  if (args_cnt == 2 && args_p[0].type == JERRY_API_DATA_TYPE_OBJECT) {
    ret_val_p->type = JERRY_API_DATA_TYPE_BOOLEAN;
    ret_val_p->v_bool = true;
    for (;;) {
      hal_gpt_delay_ms((int)args_p[1].v_float32);
      jerry_api_call_function(args_p[0].v_object, NULL, false, NULL, 0);
    }
  }
  return true;
}

DELCARE_HANDLER(clearTimeout)
{
  TimerHandle_t xTimer = (TimerHandle_t) (int)args_p[0].v_float32;
  jerry_api_value_t *arg_clone_timer = pvTimerGetTimerID(xTimer);

  for(int i = 0; i < args_cnt; ++i)
  {
    switch(arg_clone_timer[i].type)
    {
      case JERRY_API_DATA_TYPE_OBJECT:
          jerry_api_release_object(arg_clone_timer[i].v_object);
          break;
      case JERRY_API_DATA_TYPE_STRING:
          jerry_api_release_string(arg_clone_timer[i].v_string);
          break;
    }
  }

  free(arg_clone_timer);
  arg_clone_timer = NULL;

  xTimerDelete(xTimer, portMAX_DELAY);
  return true;
}


void setIntervalTimerCallback( TimerHandle_t pxTimer ) {

  jerry_api_value_t *arg_clone_timer = pvTimerGetTimerID(pxTimer);
  jerry_api_call_function(arg_clone_timer[0].v_object, NULL, false, NULL, 0);

}

DELCARE_HANDLER(setInterval)
{
  serial_nubmer = serial_nubmer + 1;
  assert(args_cnt == 2);
  const size_t array_size = sizeof(jerry_api_value_t) * args_cnt;
  jerry_api_value_t *arg_clone = malloc(array_size);
  memcpy(arg_clone, args_p, array_size);

  // acquire object and string, if any
  for(int i = 0; i < args_cnt + 1; ++i)
  {
    switch(arg_clone[i].type)
    {
      case JERRY_API_DATA_TYPE_OBJECT:
          arg_clone[i].v_object = jerry_api_acquire_object(arg_clone[i].v_object);
          break;
      case JERRY_API_DATA_TYPE_STRING:
          arg_clone[i].v_string = jerry_api_acquire_string(arg_clone[i].v_string);
          break;
    }
  }
  TimerHandle_t setTimeout_timer;

  char task_name[25] = {0};

  snprintf(task_name, 25, "TimerMain%d", serial_nubmer);

  setTimeout_timer = xTimerCreate(task_name, ((int)arg_clone[1].v_float32 / portTICK_RATE_MS), pdTRUE, arg_clone, setIntervalTimerCallback);
  xTimerStart( setTimeout_timer, 0 );

  ret_val_p->type = JERRY_API_DATA_TYPE_UINT32;
  ret_val_p->v_uint32 = (unsigned int) setTimeout_timer;

  return true;
}

DELCARE_HANDLER(clearInterval)
{
  printf("%d\n", (int)args_p[0].v_float32);
  TimerHandle_t xTimer = (TimerHandle_t) (int)args_p[0].v_float32;
  jerry_api_value_t *arg_clone_timer = pvTimerGetTimerID(xTimer);

  const _args_cnt = 2;

  for(int i = 0; i < _args_cnt; ++i)
  {
    switch(arg_clone_timer[i].type)
    {
      case JERRY_API_DATA_TYPE_OBJECT:
          jerry_api_release_object(arg_clone_timer[i].v_object);
          break;
      case JERRY_API_DATA_TYPE_STRING:
          jerry_api_release_string(arg_clone_timer[i].v_string);
          break;
    }
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