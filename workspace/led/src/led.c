#include "wrap.h"
#include "startup.h"

typedef unsigned int uint32_t;

// simplify register access
#define REG(P) (*(volatile uint32_t *) (P))

// gpio addresses and offsets
#define GPIO_BASE 0x10012000
#define GPIO_INPUT_EN 0x4
#define GPIO_OUTPUT_EN 0x8
#define GPIO_OUTPUT_VAL 0xc
#define GPIO_IOF_EN 0x38

// internal led pin
#define BLUE_LED 5

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

	volatile uint32_t i = 0;
	while(1)
	{
		// set led pin to low
		REG(GPIO_BASE + GPIO_OUTPUT_VAL) &= ~(1 << BLUE_LED);
		// wait..
	 	for (i = 0; i < 100000; i++){}

		// set led pin to high
		REG(GPIO_BASE + GPIO_OUTPUT_VAL) |= (1 << BLUE_LED);
		// wait..
		for (i = 0; i < 100000; i++){}
	}

	return 0;
}

