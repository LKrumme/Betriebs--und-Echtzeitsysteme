// Original code from https://github.com/agra-uni-bremen/sifive-hifive1
/*
 * framebuffer.c
 *
 *  Created on: Feb 3, 2020
 *      Author: dwd
 */
#include "framebuffer.h"
#include <string.h>

uint8_t framebuffer[DISP_W][DISP_H/8];

// init framebuffer
// basically does the same as fb_clear()
void fb_init()
{
	fb_clear();
}

// clear framebuffer
void fb_clear()
{
	memset(framebuffer, 0, DISP_W*(DISP_H/8));
}

// send framebuffer to display
void fb_flush()
{
	for(uint8_t y = 0; y < DISP_H/8; y++)
	{
		set_xrow(0, y);
		for(uint8_t x = 0; x < DISP_W; x++)
		{
			spi(framebuffer[x][y]);
		}
	}
}

// set pixel to p at position (x,y)
void fb_set_pixel(uint8_t x, uint8_t y, uint8_t p)
{
	if(p)
		framebuffer[x][y/8] |= 1 << y%8;
	else
		framebuffer[x][y/8] &= ~(1 << y%8);
}

// set a 8-bit vertical bar of values
void fb_set_bar(uint8_t x, uint8_t y, char val)
{
	framebuffer[x][y] = val;
}

// set pixel on framebuffer and send update directly
// to the display
void fb_set_pixel_direct(uint8_t x, uint8_t y, uint8_t p)
{
	fb_set_pixel(x, y, p);
	set_xrow(x, y/8);
	spi(framebuffer[x][y/8]);
}

// get pixel value at position (x,y)
uint8_t fb_get_pixel(uint8_t x, uint8_t y)
{
	return framebuffer[x][y/8] & (1 << y%8);
}
