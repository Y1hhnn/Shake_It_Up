#include "game_logic.h"
#include <stdlib.h>

static char current_target_dir = 0;
static uint32_t target_timestamp = 0;

void GL_SetTarget(char dir, uint32_t timestamp) {
    current_target_dir = dir;
    target_timestamp = timestamp;
}

char GL_EvaluateSwing(uint32_t swing_time, char detected_dir) {
    if (detected_dir != current_target_dir) {
        return 'M';
    }

    int32_t error = abs((int32_t)swing_time - (int32_t)target_timestamp);

    if (error <= PERFECT_WINDOW) {
        return 'P';
    }
    else if (error <= GOOD_WINDOW) {
        return 'G';
    }
    else {
        return 'M';
    }
}
