
#ifndef __ORPHEUS_CONFIG_H__
#define __ORPHEUS_CONFIG_H__

#include "orpheus.h"
#define EMOT_BOX 45 // 45x45 emoji box
#define GRID_BOX 11 // 10x10 grid
#define FLW 2 // width of focus line

#define FLW2 FLW / 2

const short gap = 2;
const short tabs_row = GRID_BOX - 1;
const short gap_box = EMOT_BOX + gap;
const short width = (GRID_BOX * EMOT_BOX) + ((GRID_BOX - 1) * gap);
const short height = width;
const char *colors[] = {"#f2f2f2", "#040404"};
const int tab_active = 0xFFD600; // color for active tab
const short tab_line = 3;

// default active tab 0 to (GRID_BOX - 1)
short tab = 9;

// Noto Color Emoji
const char *fonts[] = { "emoji:size=25" };

#endif // __ORPHEUS_CONFIG_H__

