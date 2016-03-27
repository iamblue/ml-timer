#include <string.h>

#include "jerry.h"
#include "hal_gpt.h"

#include "microlattice.h"

typedef struct
{
    uint32_t time;
    uint32_t flag;
    uint32_t count;
} gpt_sample_t;

jerry_api_value_t callback_j0;

gpt_sample_t gpt_sample0;


#define  TIMER_32K_COUNT   (32*500)                     //32K unit,500ms
#define  SYSTICK_COUNT     (2000/portTICK_PERIOD_MS)    //2000ms

static uint32_t get_current_milisecond()
{
    hal_gpt_status_t ret;
    uint32_t count = 0;
    ret = hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    return (count * 1000) / 32768;
}

DELCARE_HANDLER(__getTime) {
  uint32_t time_record = get_current_milisecond();
  ret_val_p->type = JERRY_API_DATA_TYPE_FLOAT64;
  ret_val_p->v_float64 = time_record;

  return true;
}

static void user_gpt_callback0 (gpt_sample_t *parameter) {
  char script [] = "global.eventStatus.emit('timeout', true);";
  jerry_api_value_t eval_ret;
  jerry_api_eval (script, strlen (script), false, false, &eval_ret);
  jerry_api_release_value (&eval_ret);
}

hal_gpt_running_status_t running_status0;
hal_gpt_status_t         ret_status0;

DELCARE_HANDLER(__setTimeout) {

  hal_gpt_port_t gpt_port;
  gpt_port = 0;

  hal_gpt_get_running_status(gpt_port, &running_status0);        //get running status to check if this port is used or idle.
  // if ( running_status0 != HAL_GPT_STOPPED ) {                         //if timer is running, exception handle
  //    //exception handler
  // }
  ret_status0 = hal_gpt_init(gpt_port);                               //set the GPT base environment.
  // if(HAL_GPT_STATUS_OK != ret_status0) {
  //    //error handler
  // }
  callback_j0 = args_p[0];
  hal_gpt_register_callback(gpt_port, user_gpt_callback0, (void *) &gpt_sample0); //register a user callback.
  hal_gpt_start_timer_ms(gpt_port, (int)args_p[0].v_float32, HAL_GPT_TIMER_TYPE_REPEAT);

  ret_val_p->type = JERRY_API_DATA_TYPE_BOOLEAN;
  ret_val_p->v_bool = true;

  return true;
}

DELCARE_HANDLER(__loop) {
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

void ml_timer_init(void) {
  REGISTER_HANDLER(__loop);
  REGISTER_HANDLER(__getTime);
  REGISTER_HANDLER(__setTimeout);
}
