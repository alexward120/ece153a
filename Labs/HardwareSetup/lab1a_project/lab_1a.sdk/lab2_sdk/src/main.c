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
 * helloworld.c: simple test application
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
#include "sevenSeg_new.h"	//Seven segment display digit driver
#include "xgpio.h" 		// LED driver, used for General purpose I/i

#define RESET_VALUE 0xF4240 //1*10^6 at 100MHZ = 10 Ms

XTmrCtr tmrctr;
XGpio btnGpio;
XIntc intctr;
unsigned int count = 0;

static volatile u32 Start = 0;
static volatile u32 Direction = 1;
volatile int num[8];


void reset_num() {
	for (int i=0; i < 9; i++) {
		num[i] = 0;
	}
}


void update_count() {
	if (num[7] == 0 && num[6] == 0 && num[5] == 0 && num[4] == 0 && num[3] == 0 && num[2] == -1) {
		Start = 0;
		Direction = 1;
		reset_num();
	}

	for (int i=0; i < 7; i++) {
		if (num[i] > 9) {
			num[i+1]++;
			num[i] = 0;
		}
		if (num[i] < 0) {
			num[i+1]--;
			num[i] = 9;
		}
	}

	if (num[7] > 9) {
		Start = 0;
		reset_num();
	}
}


void update_display() {
	sevenseg_draw_digit(0, num[0]);
	sevenseg_draw_digit(1, num[1]);
	sevenseg_draw_digit(2, num[2]);
	sevenseg_draw_digit(3, num[3]);
	sevenseg_draw_digit(4, num[4]);
	sevenseg_draw_digit(5, num[5]);
	sevenseg_draw_digit(6, num[6]);
	sevenseg_draw_digit(7, num[7]);
}

void button_handler(void *CallbackRef) {

	XGpio *GpioPtr = (XGpio*)CallbackRef;

	unsigned int btnVal = XGpio_DiscreteRead(&btnGpio, 1);
	if (btnVal == 0x04) {
		//right button = stop
		Start = 0;
	}
	else if (btnVal == 0x02) {
		//left button = start
		Start = 1;
	}
	else if (btnVal == 0x10) {
		//center button = reset
		reset_num();
	}
	else if (btnVal == 0x01) {
		//up button = up count
		Direction = 1;
	}
	else if (btnVal == 0x08) {
		//down button = down count
		Direction = 0;
	}

	XGpio_InterruptClear(GpioPtr, 1);
}


void timer_handler() {

	unsigned int ControlStatusReg;

	ControlStatusReg = XTimerCtr_ReadReg(tmrctr.BaseAddress, 0, XTC_TCSR_OFFSET);

	int min = 2;
	if (Start == 1) {
		if (Direction == 1) {
			num[min]++;
			if (num[min] > 9) {
				update_count();
			}
		}
		else if (Direction == 0) {
			num[min]--;
			if (num[min] < 0) {
				update_count();
			}
		}
	}

	XTmrCtr_WriteReg(tmrctr.BaseAddress, 0, XTC_TCSR_OFFSET,
			ControlStatusReg | XTC_CSR_INT_OCCURED_MASK);
}


int initialize() {
	XStatus Status;
	Status = XST_SUCCESS;

	//Setting up the timer
	Status = XTmrCtr_Initialize(&tmrctr, XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
			xil_printf("Timer initialization failed \r\n");
			return XST_FAILURE;
		}
	XTmrCtr_SetOptions(&tmrctr, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetResetValue(&tmrctr, 0, 0xFFFFFFFF - RESET_VALUE);
	XTmrCtr_Start(&tmrctr, 0);

	//Setting up GPIO buttons
	Status = XGpio_Initialize(&btnGpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	if (Status != XST_SUCCESS) {
			xil_printf("Button GPIO initialization failed...\r\n");
			return XST_FAILURE;
		}
	XGpio_InterruptEnable(&btnGpio, 1);
	XGpio_InterruptGlobalEnable(&btnGpio);


	// Setting up the interrupt controller
	Status = XIntc_Initialize(&intctr, XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Timer interrupt initialization failed \r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(&intctr, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR,
				(XInterruptHandler)timer_handler, &tmrctr);
	if (Status != XST_SUCCESS) {
			xil_printf("Failed to connect timer interrupt to interrupt controller \r\n");
			return XST_FAILURE;
		}

	Status = XIntc_Connect(&intctr, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR,
				(XInterruptHandler)button_handler, &btnGpio);
	if (Status != XST_SUCCESS) {
				xil_printf("Failed to connect button interrupt to interrupt controller \r\n");
				return XST_FAILURE;
			}

	XIntc_Enable(&intctr, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR);
	XIntc_Enable(&intctr, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR);


	XIntc_Start(&intctr, XIN_REAL_MODE);

	//Connect interrupt controller to the CPU
	microblaze_register_handler((XInterruptHandler)XIntc_DeviceInterruptHandler,
			(void*)XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);

	microblaze_enable_interrupts();

	return XST_SUCCESS;
}



int main()
{
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();

	initialize();
	reset_num();

	while(1){
		update_display();
	}

}
