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


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

/*------------------- user defined Types ---------------------------------*/

typedef enum  {BUTTON_OFF = 0, BUTTON_ON_400, BUTTON_ON_100} ButtonMode_t;

/*------------------- END user defined Types ---------------------------------*/

TaskHandle_t Button_Task_Handler;
TaskHandle_t LED_Task_100ms_Handler;
TaskHandle_t LED_Task_400ms_Handler;

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

/*------------------------ Global variables --------------------------------*/

static volatile ButtonMode_t ButtonMode  = BUTTON_OFF;
static	uint8_t PressCount = 0; // I declared it global to be able to see it in debugging mode
static	uint8_t PressCount_LastValue_BeforeReset = 0; // this variable is required only for Debuging/Demonestration purposes
static pinState_t LEDCurentState = PIN_IS_LOW;

/*------------------------ END Global variables --------------------------------*/

/*
* ButtonTask to get the button status and choose one of 3 modes
* BUTTON_OFF, BUTTON_ON_400, BUTTON_ON_100
* based on the duration of pressing the buttom
*/
void Button_Task( void * pvParameters )
{
	pinState_t ButtonCurrentState = PIN_IS_LOW;
	pinState_t ButtonPreviousState = PIN_IS_LOW;
    for( ;; )
    {
			ButtonCurrentState = GPIO_read(PORT_0,PIN0);
			// 
			if( (ButtonCurrentState == PIN_IS_HIGH) && (ButtonPreviousState == PIN_IS_HIGH) )
				{
						PressCount++;
				}
				// on the Rising edge of the button signal 
			else if( (ButtonCurrentState == PIN_IS_HIGH) && (ButtonPreviousState == PIN_IS_LOW) )
				{
					PressCount++;
					ButtonPreviousState = ButtonCurrentState;
				}
				//
			else if( (ButtonCurrentState == PIN_IS_LOW) && (ButtonPreviousState == PIN_IS_LOW) )
				{
				// do nothing
				}
				// on the Failing edge of the button signal
			else if( (ButtonCurrentState == PIN_IS_LOW) && (ButtonPreviousState == PIN_IS_HIGH) )
				{
					if(PressCount < 40) 
						{
							ButtonMode  = BUTTON_OFF; // 40 * 50ms = 2000ms , t < 2000ms
						}
					
					else if ( (PressCount >= 40) && (PressCount < 80) ) 
							{
								ButtonMode  = BUTTON_ON_400; // 2000ms < t < 4000ms
							}
					
					else if(PressCount >= 80) 
							{
								ButtonMode  = BUTTON_ON_100; // t > 4000ms
							}
							
					PressCount_LastValue_BeforeReset = PressCount;
					PressCount = 0;
					ButtonPreviousState = ButtonCurrentState;
				}
			
			//ButtonMode = BUTTON_OFF;
			vTaskDelay(50U);
    }
}


void LED_Task_100ms( void * pvParameters )
{
    for( ;; )
    {
			if(ButtonMode == BUTTON_ON_100)
				{
						/* Toggle the LED state variable */
					LEDCurentState^= PIN_IS_HIGH;
					/* update the LED pin with the required state */
					GPIO_write(PORT_0,PIN1,LEDCurentState);
				}
			else if( (ButtonMode == BUTTON_OFF) && (LEDCurentState == PIN_IS_HIGH) )
				{
					LEDCurentState = PIN_IS_LOW;
					/* update the LED pin with the required state */
					GPIO_write(PORT_0,PIN1,LEDCurentState);
				}
			/* delay for 100 ms, so that the task go to blocked state and allow for a context switching */
			vTaskDelay( 100U);
		}
}

void LED_Task_400ms( void * pvParameters )
{
    for( ;; )
    {
			if(ButtonMode == BUTTON_ON_400)
				{
						/* Toggle the LED state variable */
					LEDCurentState^= PIN_IS_HIGH;
					/* update LED pin with the required state */
					GPIO_write(PORT_0,PIN1,LEDCurentState);
				}
			/* delay for 100 ms, so that the task go to blocked state and allow for a context switching */
			vTaskDelay( 400U);
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
	 xTaskCreate(    Button_Task,
                   "Button_Task",
                   125,
                   NULL,
                   2,
                   &Button_Task_Handler
                          );
	    
	/* Create Tasks here */
	 xTaskCreate(    LED_Task_100ms,
                   "LED_Task_100ms",
                   125,
                   NULL,
                   2,
                   &LED_Task_100ms_Handler
                          );
	
    /* Create Tasks here */
	 xTaskCreate(    LED_Task_400ms,
                   "LED_Task_400ms",
                   125,
                   NULL,
                   1,
                   &LED_Task_400ms_Handler
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


