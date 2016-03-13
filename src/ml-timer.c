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

gpt_sample_t gpt_sample;

#define  TIMER_32K_COUNT   (32*500)                     //32K unit,500ms
#define  SYSTICK_COUNT     (2000/portTICK_PERIOD_MS)    //2000ms

static uint32_t get_current_milisecond()
{
    hal_gpt_status_t ret;
    uint32_t count = 0;
    ret = hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    return (count * 1000) / 32768;
}

DELCARE_HANDLER(getTime) {
  uint32_t time_record = get_current_milisecond();
  ret_val_p->type = JERRY_API_DATA_TYPE_FLOAT64;
  ret_val_p->v_float64 = time_record;

  return true;
}

DELCARE_HANDLER(setTimeout) {
  if (args_cnt == 2 && args_p[0].type == JERRY_API_DATA_TYPE_OBJECT) {
    hal_gpt_delay_ms((int)args_p[1].v_float32);
    jerry_api_call_function(args_p[0].v_object, NULL, false, NULL, 0);
  }

  ret_val_p->type = JERRY_API_DATA_TYPE_BOOLEAN;
  ret_val_p->v_bool = true;

  return true;
}

DELCARE_HANDLER(timer) {
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
  REGISTER_HANDLER(timer);
  REGISTER_HANDLER(getTime);
  REGISTER_HANDLER(setTimeout);
}
