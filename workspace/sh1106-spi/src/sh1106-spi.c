// See LICENSE for license details.

#include "encoding.h"
#include "platform.h"
#include "wrap.h"
#include "startup.h"
#include "display.h"
#include "font.h"
#include "framebuffer.h"

#include <string.h>

// wait for f_milliseconds
void delay(uint32_t f_milliseconds)
{
    volatile uint64_t *now = (volatile uint64_t*)(CLINT_CTRL_ADDR + CLINT_MTIME);
    volatile uint64_t then = *now + f_milliseconds*(RTC_FREQ / 2000);
    while (*now < then);
}

// example text
const char* text = "Oled example...";

int main (void)
{
	// init oled display including spi
	oled_init();
	// init frame buffer
	fb_init();

	// short break
	delay(2000);

	uint32_t textpos = 0;
	while (1)
	{
		// print single character
		printChar(text[textpos]);
		if(textpos % ((DISP_W / CHAR_W) * (DISP_H/8)) == ((DISP_W / CHAR_W) * (DISP_H/8))-1)
		{
			delay(200);
		}
		// restart at the beginning of the text if at the end
		if (textpos + 1 >= strlen(text))
		{
			textpos = 0;
			delay(1000);
		}
		// otherwise just go to the next character
		else
		{
			textpos++;
		}

		// show framebuffer on the display
		fb_flush();
	}
}

