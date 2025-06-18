// FreeRTOS.org includes
#include "FreeRTOS.h"
#include "task.h"
#include "startup.h"
#include "wrap.h"

#define REG(P) (*(volatile uint32_t *) (P))

#define PLIC_BASE 0x0C000000
#define PLIC_CLAIM 0x200004

// task functions
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );
void vTask3( void *pvParameters );

// interrupt handler for handling all unclaimed interrupts
void irq_handler(void)
{
	// just claim it and do nothing else with it
    uint32_t claim = REG(PLIC_BASE + PLIC_CLAIM);
    REG(PLIC_BASE + PLIC_CLAIM) = claim;
}

int main( void )
{
	// create three tasks with different priorities
	xTaskCreate( vTask1, "Task 1", 1000, NULL, 2, NULL );
	xTaskCreate( vTask2, "Task 2", 1000, NULL, 1, NULL );
	xTaskCreate( vTask3, "Task 3", 1000, NULL, 1, NULL );

	// start scheduler
	vTaskStartScheduler();

	// infinite loop, code should never get here
	for( ;; );
	return 0;
}

// complex task 1 with delay
void vTask1( void *pvParameters )
{
	TickType_t xLastWakeTime;
	// delay between two loops ( 1 sec )
	const TickType_t xDelay = pdMS_TO_TICKS( 1000 );

	// initialize current tick count
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		// do something
		asm volatile ("nop");
		// periodic, ensures that the loop is executed exactly once per second
		vTaskDelayUntil( &xLastWakeTime, xDelay );
	}
}

// simple task 2
void vTask2( void *pvParameters )
{
	// every task requires its own loop
	for( ;; )
	{
		// do something
		asm volatile ("nop");
	}
}

// simple task 3
void vTask3( void *pvParameters )
{
	// every task requires its own loop
	for( ;; )
	{
		// do something
		asm volatile ("nop");
	}
}
