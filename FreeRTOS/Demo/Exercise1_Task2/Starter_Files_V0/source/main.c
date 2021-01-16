/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

	
	/* Notes : 
	*--------
	* 1- in this exercise I have created 3 tasks:
	*													- LED1_Task_100ms to toggle LED1 (PORT0, PIN0) every 100 ms , with Priority 3 (highest)
	*													- LED2_Task_500ms to toggle LED2 (PORT0, PIN1) every 500 ms , with Priority 2
	*													- LED3_Task_1000ms to toggle LED3 (PORT0, PIN2) every 1000 ms, with Priority 1
	*
	* 2- Preemption is disabled  by using the Macro (configUSE_PREEMPTION		0) in FreeRTOSConfig.g
	* 
	* 3- I added 3 extra global variables for debugging/demonestration purpose:
  *													- LED1_Task_100ms_ExecutionCounter will increament by 1 every execution of the task (max = 100). after 100 it will reset
	*													- LED2_Task_500ms_ExecutionCounter will increament by 1 every execution of the task (max = 20). after 20 it will reset
	*													- LED3_Task_1000ms_ExecutionCounter will increament by 1 every execution of the task (max = 10). after 10 it will reset
	*/



/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


TaskHandle_t LED1_Task_100ms_Handler;
TaskHandle_t LED2_Task_500ms_Handler;
TaskHandle_t LED3_Task_1000ms_Handler;

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

/*--------------- Global Variables ( used for Demonestration via debugger ) ----------*/

static uint8_t LED1_Task_100ms_ExecutionCounter = 0;
static uint8_t LED2_Task_500ms_ExecutionCounter = 0;
static uint8_t LED3_Task_1000ms_ExecutionCounter = 0;

/*--------------- END of Global Variables ( used for Demonestration via debugger ) ----------*/

void LED1_Task_100ms( void * pvParameters )
{
	/* Required state intialization  */
	pinState_t LED1_RequiredState = PIN_IS_LOW;
    for( ;; )
    {
			/* Toggle the required state */
			LED1_RequiredState ^= PIN_IS_HIGH;
			/* update PORT0_PIN0 with the required state */
			GPIO_write(PORT_0,PIN0,LED1_RequiredState);
			LED1_Task_100ms_ExecutionCounter++;
			if(LED1_Task_100ms_ExecutionCounter % 100 == 0) // LED1_Task executed 100 times & LED2_Task executed 20 times & LED3_Task executed 10 times
			{
				LED1_Task_100ms_ExecutionCounter = 0; // reset counter
				LED2_Task_500ms_ExecutionCounter = 0; // reset counter
				LED3_Task_1000ms_ExecutionCounter = 0; // reset counter
			}
			/* delay for 100 ms, so that the task go to blocked state and allow for a context switching */
			vTaskDelay( 100U);
		}
}

void LED2_Task_500ms( void * pvParameters )
{
	/* Required state intialization  */
	pinState_t LED2_RequiredState = PIN_IS_LOW;
    for( ;; )
    {
			/* Toggle the required state */
			LED2_RequiredState ^= PIN_IS_HIGH;
			/* update PORT0_PIN1 with the required state */
			GPIO_write(PORT_0,PIN1,LED2_RequiredState);
			LED2_Task_500ms_ExecutionCounter++;
			/* delay for 500 ms, so that the task go to blocked state and allow for a context switching */
			vTaskDelay( 500U);
		}
}


void LED3_Task_1000ms( void * pvParameters )
{
	/* Required state intialization  */
	pinState_t LED3_RequiredState = PIN_IS_LOW;
    for( ;; )
    {
			/* Toggle the required state */
			LED3_RequiredState ^= PIN_IS_HIGH;
			/* update PORT0_PIN2 with the required state */
			GPIO_write(PORT_0,PIN2,LED3_RequiredState);
			LED3_Task_1000ms_ExecutionCounter++;
			/* delay for 1000 ms, so that the task go to blocked state and allow for a context switching */
			vTaskDelay( 1000U);
		}
}



/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();


	
	
    /* Create Tasks here */
	 xTaskCreate(    LED1_Task_100ms,
                   "LED1_Task_100ms",
                   125,
                   NULL,
                   3,
                   &LED1_Task_100ms_Handler
                          );
	
		 xTaskCreate(    LED2_Task_500ms,
                   "LED2_Task_500ms",
                   125,
                   NULL,
                   2,
                   &LED2_Task_500ms_Handler
                          );
	
		 xTaskCreate(    LED3_Task_1000ms,
                   "LED3_Task_1000ms",
                   125,
                   NULL,
                   1,
                   &LED3_Task_1000ms_Handler
                          );
	



	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


