/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
* main.c:
*
* This application configures UART 16550 to baud rate 9600.
* PS7 UART (Zynq) is not initialized by this application, since
* bootrom/bsp configures it to baud rate 115200
*
* ------------------------------------------------
* | UART TYPE   BAUD RATE                        |
* ------------------------------------------------
*   uartns550   9600
*   uartlite    Configurable only in HW design
*   ps7_uart    115200 (configured by bootrom/bsp)
*/

#include <stdio.h>		// Used for printf()
#include <stdlib.h>		// Used for rand()
#include "xparameters.h"	// Contains hardware addresses and bit masks
#include "xil_cache.h"		// Cache Drivers
#include "xintc.h"		// Interrupt Drivers
#include "xtmrctr.h"		// Timer Drivers
#include "xtmrctr_l.h" 		// Low-level timer drivers
#include "xil_printf.h" 	// Used for xil_printf()
#include "xgpio.h" 		// LED driver, used for General purpose I/i

#define RESET_VALUE 0x5F5E100 // 100*10^6 @ 100 MHz = 1s
#define ENCODER_CHANNEL 1
#define LED_CHANNEL 1

XTmrCtr TimerCounter;		// Timer Counter Instance
XIntc InterruptController;	// Interrupt Controller Instance
XGpio Encoder_Gpio;		// Encoder GPIO Instance
XGpio LED_Gpio;			// LED GPIO Instance
XGpio RGB_LED_Gpio;		// RGB LED GPIO Instance

static int encoder_count = 4;	// Encoder count
unsigned int toggle = 1;	// Toggle for LED
static u16 state = 0b11;	// State of encoder
volatile u16 led_16 = 1; 	// LED value for 16 LEDs

enum STATES {		// States of encoder
	S0 = 0b11,
	S1 = 0b01,
	S2 = 0b00,
	S3 = 0b10
};

void LED_Press(int toggle) {
	if (toggle == 0) {
		XGpio_DiscreteWrite(&LED_Gpio, LED_CHANNEL, 0x00);
	} else {
		XGpio_DiscreteWrite(&LED_Gpio, LED_CHANNEL, led_16);
	}
}

void left_overflow() {
	if (led_16 == 0b1000000000000000) {
		led_16 = 0b1;
	} else {
		led_16 = led_16 << 1;
	}
	XGpio_DiscreteWrite(&LED_Gpio, LED_CHANNEL, led_16);
}

void right_overflow() {
	if (led_16 == 0b1) {
		led_16 = 0b1000000000000000;
	} else {
		led_16 = led_16 >> 1;
	}
	XGpio_DiscreteWrite(&LED_Gpio, LED_CHANNEL, led_16);
}

void Timer_Handler() {
	u32 ControlStatusReg;
	ControlStatusReg = XTimerCtr_ReadReg(TimerCounter.BaseAddress, 0, XTC_TCSR_OFFSET);
	XTmrCtr_WriteReg(TimerCounter.BaseAddress, 0, XTC_TCSR_OFFSET, ControlStatusReg | XTC_CSR_INT_OCCURED_MASK);
}

void Encoder_Handler(void *CallbackRef) {
	XGpio *GpioPtr = (XGpio *)CallbackRef;
	u32 start = XTmrCtr_GetTimerCounterReg(TimerCounter.BaseAddress, 0);
	u32 end = start;

	while (end < (start + RESET_VALUE / 10000)) {
		end = XTmrCtr_GetTimerCounterReg(TimerCounter.BaseAddress, 0);
	}
	u32 encoderStatus = XGpio_DiscreteRead(&Encoder_Gpio, ENCODER_CHANNEL);

	if (encoderStatus == 7)  {//if the encoder button is pressed
		if (toggle == 0) {
			toggle = 1;
		} else {
			toggle = 0;
		}
		state = S0; //Reset state
		encoder_count = 4; //Reset encoder count
		LED_Press(toggle);
	}

	switch (state) {
		case S0: {
			if (toggle == 1) {
				if (encoder_count == 0 || encoder_count == 8) {
					encoder_count = 4; //Reset encoder count
				}
			}
			switch (encoderStatus) {
				case 0b01: {
					if (encoder_count == 4) {
						encoder_count = encoder_count + 1;
						state = S1;
					}
					break;
				}
				case 0b10: {
					if (encoder_count == 4) {
						encoder_count = encoder_count - 1;
						state = S3;
					}
					break;
				}
			}
			break;
		}
		case S1: {
			switch(encoderStatus) {
				case 0b11: {
					if (encoder_count == 1) {
						encoder_count = encoder_count - 1;
						state = S0;
						if (toggle == 1) {
							left_overflow();
						}
					}
					break;
				}
				case 0b00: {
					if (encoder_count == 5) {
						encoder_count = encoder_count + 1;
						state = S2;
					}
					break;
				}
			}
			break;
		}
		case S2: {
			switch(encoderStatus) {
				case 0b01: {
					if (encoder_count == 2) {
						encoder_count = encoder_count - 1;
						state = S1;
					}
					break;
				}
				case 0b10: {
					if (encoder_count == 6) {
						encoder_count = encoder_count + 1;
						state = S3;
					}
					break;
				}
			}
			break;
		}
		case S3: {
			switch(encoderStatus) {
				case 0b00: {
					if (encoder_count == 3) {
						encoder_count = encoder_count - 1;
						state = S2;
					}
					break;
				}
				case 0b11: {
					if (encoder_count == 7) {
						encoder_count = encoder_count + 1;
						state = S0;
						if (toggle == 1) {
							right_overflow();
						}
					}
					break;
				}
			}
			break;
		}
	}
	XGpio_InterruptClear(GpioPtr, ENCODER_CHANNEL); //clear the interrupt
}


int initialize()
{
	XStatus Status = XST_SUCCESS;

	// Initialize Interrupt Controller
	Status = XIntc_Initialize(&InterruptController, XPAR_INTC_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: Interrupt Controller Initialization failed!\n");
		return XST_FAILURE;
	}
	Status = XIntc_Connect(&InterruptController, XPAR_INTC_0_TMRCTR_0_VEC_ID,
			(XInterruptHandler)Timer_Handler, &TimerCounter);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: Timer Handler Connection failed!\n");
		return XST_FAILURE;
	}
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: Interrupt Controller Start failed!\n");
		return XST_FAILURE;
	}
	XIntc_Enable(&InterruptController, XPAR_INTC_0_TMRCTR_0_VEC_ID);

	// Initialize Timer Counter
	Status = XTmrCtr_Initialize(&TimerCounter, XPAR_INTC_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: Timer Counter Initialization failed!\n");
		return XST_FAILURE;
	}
	XTmrCtr_SetOptions(&TimerCounter, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetResetValue(&TimerCounter, 0, 0xFFFFFFFF - RESET_VALUE);
	XTmrCtr_Start(&TimerCounter, 0);

	// Initialize LED GPIO
	Status = XGpio_Initialize(&LED_Gpio, XPAR_AXI_GPIO_LED_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: LED GPIO Initialization failed!\n");
		return XST_FAILURE;
	}

	// Initialize RGB LED GPIO
	Status = XGpio_Initialize(&RGB_LED_Gpio, XPAR_AXI_GPIO_RGB_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: RGB LED GPIO Initialization failed!\n");
		return XST_FAILURE;
	}

	// Initialize Encoder GPIO
	Status = XGpio_Initialize(&Encoder_Gpio, XPAR_AXI_GPIO_ENCODER_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: Encoder GPIO Initialization failed!\n");
		return XST_FAILURE;
	}
	Status = XIntc_Connect(&InterruptController, XPAR_INTC_0_GPIO_2_VEC_ID,
			(XInterruptHandler)Encoder_Handler, &Encoder_Gpio);
	if (Status != XST_SUCCESS) {
		xil_printf("Error: Encoder Handler Connection failed!\n");
		return XST_FAILURE;
	}
	XIntc_Enable(&InterruptController, XPAR_INTC_0_GPIO_2_VEC_ID);

	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE); //running again because of encoder
	if (Status != XST_SUCCESS) {
		xil_printf("Error: Interrupt Controller Start failed!\n");
		return XST_FAILURE;
	}
	XGpio_InterruptEnable(&Encoder_Gpio, ENCODER_CHANNEL);
	XGpio_InterruptGlobalEnable(&Encoder_Gpio);

	// Initialize Microblaze
	microblaze_register_handler((XInterruptHandler)XIntc_DeviceInterruptHandler,
			(void*)XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);

	microblaze_enable_interrupts();

	return XST_SUCCESS;
}

void test_rbg_blink() {
	XGpio_DiscreteWrite(&RGB_LED_Gpio, LED_CHANNEL, 0x1);
	for (int i = 0; i < 5000000; i++);
	XGpio_DiscreteWrite(&RGB_LED_Gpio, LED_CHANNEL, 0x0);
	for (int i = 0; i < 5000000; i++);
}

int main()
{
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();

   print("In Main\n\r");

	initialize();

	XGpio_DiscreteWrite(&LED_Gpio, LED_CHANNEL, 1 << led_16);
	while(1) {
		test_rbg_blink();
	}

   return XST_SUCCESS;
}
