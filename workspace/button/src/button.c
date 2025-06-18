#include "wrap.h"
#include "startup.h"

typedef unsigned int uint32_t;

// simplify register access
#define REG(P) (*(volatile uint32_t *) (P))

// gpio addresses and offsets
#define GPIO_BASE 0x10012000
#define GPIO_INPUT_EN 0x4
#define GPIO_INPUT_VAL 0x0
#define GPIO_PUE 0x10
#define GPIO_OUTPUT_EN 0x8
#define GPIO_OUTPUT_VAL 0xc
#define GPIO_IOF_EN 0x38

// internal pins
#define BLUE_LED 5
#define BUTTON 18

int main (void)
{
	// setup LED as output
	// disable pin-specific functions
	REG(GPIO_BASE + GPIO_IOF_EN) &= ~(1 << BLUE_LED);
	// disable input function of the pin
	REG(GPIO_BASE + GPIO_INPUT_EN) &= ~(1 << BLUE_LED);
	// enable output function of the pin
	REG(GPIO_BASE + GPIO_OUTPUT_EN) |= 1 << BLUE_LED;
	// set led pin to high
	REG(GPIO_BASE + GPIO_OUTPUT_VAL) |= (1 << BLUE_LED);

	// setup BUTTON as input
	// disable pin-specific functions
	REG(GPIO_BASE + GPIO_IOF_EN) &= ~(1 << BUTTON);
	// enable pull-up resistor
	REG(GPIO_BASE + GPIO_PUE) |= 1 << BUTTON;
	// enable input function of the pin
	REG(GPIO_BASE + GPIO_INPUT_EN) |= 1 << BUTTON;
	// disable output function of the pin
	REG(GPIO_BASE + GPIO_OUTPUT_EN) &= ~(1 << BUTTON);
	// set button pin to low
	REG(GPIO_BASE + GPIO_OUTPUT_VAL) &= ~(1 << BUTTON);


	volatile uint32_t i = 0;
	// polling
	while(1)
	{
		// a little delay
		for (i = 0; i < 184210; i++){}

		// check if button pressed and if turn led off else on
		// attention: no debouncing.
		// attention: pull-up enabled.
		if (REG(GPIO_BASE + GPIO_INPUT_VAL) & (1 << BUTTON))
		{
			// set led pin to high
			REG(GPIO_BASE + GPIO_OUTPUT_VAL) |= (1 << BLUE_LED);
		}
		else
		{
			// set led pin to low
			REG(GPIO_BASE + GPIO_OUTPUT_VAL) &= ~(1 << BLUE_LED);
		}
	}
}
