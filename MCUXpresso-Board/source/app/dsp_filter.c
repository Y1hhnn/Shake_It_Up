#include <stdio.h>
#include <stdlib.h>
#include "drivers/uart_comm.h"
#include "dsp_filter.h"

#define SWING_THRESHOLD_SQ 2000000
#define DEBOUNCE_COOLDOWN 600
#define EMA_ALPHA 3

static int cooldown_counter = 0;
static int32_t ema_x = 0, ema_y = 0, ema_z = 0;
static int32_t prev_dynamic_sq = 0;
static int is_rising = 0;

static int32_t peak_dyn_x = 0;
static int32_t peak_dyn_y = 0;

char DSP_DetectSwing(SRAWDATA *data)
{
    if (cooldown_counter > 0)
    {
        cooldown_counter--;
    }

    if (ema_x == 0 && ema_y == 0 && ema_z == 0)
    {
        ema_x = data->x;
        ema_y = data->y;
        ema_z = data->z;
        return 0;
    }
    ema_x = ema_x + ((EMA_ALPHA * (data->x - ema_x)) >> 10);
    ema_y = ema_y + ((EMA_ALPHA * (data->y - ema_y)) >> 10);
    ema_z = ema_z + ((EMA_ALPHA * (data->z - ema_z)) >> 10);

    int32_t dyn_x = data->x - ema_x;
    int32_t dyn_y = data->y - ema_y;
    int32_t dyn_z = data->z - ema_z;

    dyn_x >>= 2;
    dyn_y >>= 2;
    dyn_z >>= 2;

    int32_t dynamic_sq = (dyn_x * dyn_x) + (dyn_y * dyn_y) + (dyn_z * dyn_z);

    //    char debug_msg[64];
    //    snprintf(debug_msg, sizeof(debug_msg), "DEBUG_SQ:%d\n", dynamic_sq);
    //    uart_puts(debug_msg);

    char hit_direction = 0;

    if (dynamic_sq > SWING_THRESHOLD_SQ)
    {
        if (dynamic_sq > prev_dynamic_sq)
        {
            is_rising = 1;
            peak_dyn_x = dyn_x;
            peak_dyn_y = dyn_y;
        }
        else if (is_rising && dynamic_sq < prev_dynamic_sq)
        {
            is_rising = 0;

            if (cooldown_counter <= 0)
            {
                if (abs(peak_dyn_x) > abs(peak_dyn_y))
                {
                    hit_direction = (peak_dyn_x > 0) ? 'R' : 'L';
                }
                else
                {
                    hit_direction = (peak_dyn_y > 0) ? 'U' : 'D';
                }

                cooldown_counter = 60;

                peak_dyn_x = 0;
                peak_dyn_y = 0;
                prev_dynamic_sq = 0;
            }
        }
    }
    else
    {
        is_rising = 0;
    }

    prev_dynamic_sq = dynamic_sq;
    return hit_direction;
}
