/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "wrap.h"
#include "startup.h"
#include "task.h"
#include "semphr.h"
#include "framebuffer.h"
#include "font.h"
#include "platform.h"

#define REG(P) (*(volatile uint32_t *) (P))

#define GPIO_BASE 0x10012000

#define PLIC_BASE 0x0C000000
#define PLIC_ENABLE 0x2000

// interrupt handler for handling all unclaimed interrupts
void irq_handler(void)
{
	// do nothing
	asm volatile ("nop");
}

int main( void )
{
	// deactivate PLIC
    REG(PLIC_BASE + PLIC_ENABLE) = 0;
    REG(PLIC_BASE + PLIC_ENABLE + 4) = 0;

	// dummy loop
	for (;;) {
		// does nothing
		asm volatile ("nop");
	}

	return 0;
}
