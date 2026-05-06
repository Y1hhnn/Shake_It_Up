/**
 * @file dsp_filter.c
 * @brief Accelerometer DSP logic for swing detection and direction classification.
 */

#include <stdio.h>
#include <stdlib.h>
#include "drivers/uart_comm.h"
#include "dsp_filter.h"

#define SWING_START_THRESHOLD_SQ 1500000
#define SWING_RELEASE_THRESHOLD_SQ 500000
#define DEBOUNCE_COOLDOWN 250
#define EMA_ALPHA 3

#define MIN_CAPTURE_SAMPLES 4
#define MAX_CAPTURE_SAMPLES 25

#define DIRECTION_RATIO_NUM 10
#define DIRECTION_RATIO_DEN 7

typedef enum
{
    SWING_IDLE,
    SWING_CAPTURING,
    SWING_COOLDOWN
} SwingState;

static SwingState swing_state = SWING_IDLE;
static int cooldown_counter = 0;

static int32_t ema_x = 0;
static int32_t ema_y = 0;
static int32_t ema_z = 0;

static int capture_count = 0;
static int32_t peak_dynamic_sq = 0;
static int32_t peak_xy_sq = 0;
static int32_t peak_xy_x = 0;
static int32_t peak_xy_y = 0;

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

    int32_t dynamic_sq = dyn_x * dyn_x + dyn_y * dyn_y + dyn_z * dyn_z;
    int32_t xy_sq = dyn_x * dyn_x + dyn_y * dyn_y;

    char hit_direction = 0;

    switch (swing_state)
    {
    case SWING_IDLE:
        if (dynamic_sq > SWING_START_THRESHOLD_SQ)
        {
            capture_count = 0;
            peak_dynamic_sq = dynamic_sq;

            peak_xy_sq = xy_sq;
            peak_xy_x = dyn_x;
            peak_xy_y = dyn_y;

            swing_state = SWING_CAPTURING;
        }
        break;

    case SWING_CAPTURING:
        capture_count++;

        if (dynamic_sq > peak_dynamic_sq)
        {
            peak_dynamic_sq = dynamic_sq;
        }

        if (xy_sq > peak_xy_sq)
        {
            peak_xy_sq = xy_sq;
            peak_xy_x = dyn_x;
            peak_xy_y = dyn_y;
        }

        if ((dynamic_sq < SWING_RELEASE_THRESHOLD_SQ && capture_count >= MIN_CAPTURE_SAMPLES) ||
            capture_count >= MAX_CAPTURE_SAMPLES)
        {
            if (peak_dynamic_sq > SWING_START_THRESHOLD_SQ &&
                capture_count >= MIN_CAPTURE_SAMPLES &&
                cooldown_counter <= 0)
            {
                int32_t abs_x = abs(peak_xy_x);
                int32_t abs_y = abs(peak_xy_y);

                if (abs_x * DIRECTION_RATIO_DEN > abs_y * DIRECTION_RATIO_NUM)
                {
                    // X axis flipped for your board orientation
                    hit_direction = (peak_xy_x > 0) ? 'R' : 'L';
                }
                else if (abs_y * DIRECTION_RATIO_DEN > abs_x * DIRECTION_RATIO_NUM)
                {
                    // Y axis kept same
                    hit_direction = (peak_xy_y > 0) ? 'U' : 'D';
                }
                else
                {
                    hit_direction = 0;
                }
            }

            cooldown_counter = DEBOUNCE_COOLDOWN;
            swing_state = SWING_COOLDOWN;

            capture_count = 0;
            peak_dynamic_sq = 0;
            peak_xy_sq = 0;
            peak_xy_x = 0;
            peak_xy_y = 0;
        }
        break;

    case SWING_COOLDOWN:
        if (cooldown_counter <= 0 && dynamic_sq < SWING_RELEASE_THRESHOLD_SQ)
        {
            swing_state = SWING_IDLE;
        }
        break;

    default:
        swing_state = SWING_IDLE;
        break;
    }

    return hit_direction;
}
