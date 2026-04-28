#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>

#define PERFECT_WINDOW 200
#define GOOD_WINDOW    500

void GL_SetTarget(char dir, uint32_t timestamp);
char GL_EvaluateSwing(uint32_t swing_time, char detected_dir);

#endif
