// Original code from https://github.com/agra-uni-bremen/sifive-hifive1
#pragma once

#include "inttypes.h"

#define FONT_W 6
#define FONT_H 8
#define FONT_N 128
#define CHAR_W (FONT_W+1)

void printChar(uint8_t ch);
void printText(const char* text);
void setCursor(int new_row, int new_col);
