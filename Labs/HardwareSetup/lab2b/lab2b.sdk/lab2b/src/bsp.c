/*****************************************************************************
* bsp.c for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 27,2019
*****************************************************************************/

/**/
#include <stdio.h>
#include <math.h>
#include "qpn_port.h"
#include "bsp.h"
#include "lab2a.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xparameters.h"	// Contains hardware addresses and bit masks
#include "xil_cache.h"		// Cache Drivers
#include "xtmrctr.h"		// Timer Drivers
#include "xtmrctr_l.h" 		// Low-level timer drivers
#include "xil_printf.h" 	// Used for xil_printf()
#include "xgpio.h" 		// LED driver, used for General purpose I/i
#include "xspi.h"
#include "xspi_l.h"
#include "lcd.h"

/*****************************/

/* Define all variables and Gpio objects here  */
XIntc sys_intctr;	//interrupt controller
XGpio enc_gpio; //encoder gpio
XTmrCtr sys_tmrctr;
XGpio dc;
XSpi spi;
XGpio btn;

#define RESET_VALUE 0x5F5E100
#define GPIO_CHANNEL1 1
#define LED_CHANNEL 1
#define BUTTON_CHANNEL 1

int toggle = 1;
int NumOfTri = 0;
int volume = 0;
int count = 0;
int TimerFlag = 0;
int EncTrig = 0;
static int encoder_count = 4;
static Xuint16 state = 0b11;
volatile u16 led_16 = 1;
int VolumeTimeOut = 0;
int TextTimeOut = 0;

enum STATES {
		S0 = 0b11,
		S1 = 0b01,
		S2 = 0b00,
		S3 = 0b10
};
void AQUABLUE(); //drawing background function
void RETEXT(); //function that displays error text
// void debounceInterrupt(); // Write This function

/* ----- New Timer Handler ----- */
void TimerCounterHandler(void *CallBackRef)
{
	if (MainVolumeFlag == 1) {
		if (VolumeFlag == 1) {
			setColor(0, 255, 0);
			fillRect(70, 90, act_volume+70, 110);
			VolumeTimeOut = 0;
			VolumeFlag = 0;
		}
		if (VolumeTimeOut > 3069) {
			//printf("Timed Out");
			DrawRectArea(2);
			MainVolumeFlag = 0;
		}
	}
	VolumeTimeOut++;
	if (MainTextFlag == 1) {
		if (TextFlag == 1) {
			TextTimeOut = 0;
			TextFlag = 0;
		}
		if (TextTimeOut > 3069) {
			//printf("Timed Out");
			DrawRectArea(3);
			MainTextFlag = 0;
		}

	}
	TextTimeOut++;
}

/*..........................................................................*/
void BSP_init(void) {
	
	u32 controlReg;
	XSpi_Config *spiConfig;

	/* ----- Initialize Interrupt ----- */
	XIntc_Initialize(&sys_intctr, XPAR_INTC_0_DEVICE_ID);
	XIntc_Start(&sys_intctr, XIN_REAL_MODE);

	/* ----- Initialize ENCODER ----- */
	XGpio_Initialize(&enc_gpio, XPAR_AXI_GPIO_ENCODER_DEVICE_ID);
	XIntc_Connect(&sys_intctr, XPAR_INTC_0_GPIO_2_VEC_ID, (Xil_ExceptionHandler)TwistHandler, &enc_gpio);
	XIntc_Enable(&sys_intctr, XPAR_INTC_0_GPIO_2_VEC_ID);
	XGpio_InterruptEnable(&enc_gpio, GPIO_CHANNEL1);
	XGpio_InterruptGlobalEnable(&enc_gpio);

	/*  ----- Intialize Button ----- */
	XGpio_Initialize(&btn, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	XIntc_Connect(&sys_intctr, XPAR_INTC_0_GPIO_1_VEC_ID, (Xil_ExceptionHandler)GpioHandler, &btn);
	XIntc_Enable(&sys_intctr, XPAR_INTC_0_GPIO_1_VEC_ID);
	
	XGpio_InterruptEnable(&btn, BUTTON_CHANNEL);
	XGpio_InterruptGlobalEnable(&btn);

	/* ----- New Timer Setup ----- */
	XTmrCtr_Initialize(&sys_tmrctr, XPAR_AXI_TIMER_0_DEVICE_ID);
	XIntc_Initialize(&sys_intctr, XPAR_INTC_0_DEVICE_ID);
	XIntc_Connect(&sys_intctr, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR,
				(XInterruptHandler)XTmrCtr_InterruptHandler,
				(void *)&sys_tmrctr);
	XIntc_Enable(&sys_intctr, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR);
	XTmrCtr_SetHandler(&sys_tmrctr, TimerCounterHandler, &sys_tmrctr);
	XTmrCtr_SetOptions(&sys_tmrctr, 0,
				XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetResetValue(&sys_tmrctr, 0, 0xFFFF0000);
	XTmrCtr_Start(&sys_tmrctr, 0);


	/* ----- Initialize LCD ----- */
	XGpio_Initialize(&dc, XPAR_SPI_DC_DEVICE_ID);
	XGpio_SetDataDirection(&dc, 1, 0x0);
	spiConfig = XSpi_LookupConfig(XPAR_SPI_DEVICE_ID);
	XSpi_CfgInitialize(&spi, spiConfig, spiConfig->BaseAddress);
	XSpi_Reset(&spi);
	controlReg = XSpi_GetControlReg(&spi);
	XSpi_SetControlReg(&spi,
			(controlReg | XSP_CR_ENABLE_MASK | XSP_CR_MASTER_MODE_MASK) &
			(~XSP_CR_TRANS_INHIBIT_MASK));
	XSpi_SetSlaveSelectReg(&spi, ~0x01);

	/* ----- Initialize MICROBLAZE ----- */
	microblaze_register_handler((XInterruptHandler)XIntc_DeviceInterruptHandler,
			(void*)XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	microblaze_enable_interrupts();
}
/*..........................................................................*/
void QF_onStartup(void) {                 /* entered with interrupts locked */

/* Enable interrupts */
	xil_printf("\n\rQF_onStartup\n"); // Comment out once you are in your complete program

	initLCD();
	clrScr();
	//GREEN();
	AQUABLUE();
	xil_printf("ready to run\n");

	// Variables for reading Microblaze registers to debug your interrupts.
//	{
//		u32 axi_ISR =  Xil_In32(intcPress.BaseAddress + XIN_ISR_OFFSET);
//		u32 axi_IPR =  Xil_In32(intcPress.BaseAddress + XIN_IPR_OFFSET);
//		u32 axi_IER =  Xil_In32(intcPress.BaseAddress + XIN_IER_OFFSET);
//		u32 axi_IAR =  Xil_In32(intcPress.BaseAddress + XIN_IAR_OFFSET);
//		u32 axi_SIE =  Xil_In32(intcPress.BaseAddress + XIN_SIE_OFFSET);
//		u32 axi_CIE =  Xil_In32(intcPress.BaseAddress + XIN_CIE_OFFSET);
//		u32 axi_IVR =  Xil_In32(intcPress.BaseAddress + XIN_IVR_OFFSET);
//		u32 axi_MER =  Xil_In32(intcPress.BaseAddress + XIN_MER_OFFSET);
//		u32 axi_IMR =  Xil_In32(intcPress.BaseAddress + XIN_IMR_OFFSET);
//		u32 axi_ILR =  Xil_In32(intcPress.BaseAddress + XIN_ILR_OFFSET) ;
//		u32 axi_IVAR = Xil_In32(intcPress.BaseAddress + XIN_IVAR_OFFSET);
//		u32 gpioTestIER  = Xil_In32(sw_Gpio.BaseAddress + XGPIO_IER_OFFSET);
//		u32 gpioTestISR  = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_ISR_OFFSET ) & 0x00000003; // & 0xMASK
//		u32 gpioTestGIER = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_GIE_OFFSET ) & 0x80000000; // & 0xMASK
//	}
}

void DrawRectArea(int row_val) {
	int NewRow = row_val*40;
	for(int Col=1; Col < 5; Col++) {
		int NewCol = Col*40;
		for (int y=0; y<40; y++) {
			int blue = 2*ceil(y/2);
			setColor(0, 255,255);
			drawHLine(0+NewCol, y+NewRow, 20-(blue/2));
			setColor(0, 0, 255);
			drawHLine(20-(blue/2)+NewCol, y+NewRow, blue);
			setColor(0,255,255);
			drawHLine(20+(blue/2)+NewCol, y+NewRow, 20-(blue/2));
		}
	}
}

void AQUABLUE() {
	clrScr();
	for (int Row = 0; Row<8; Row++) {
		int NewRow = Row*40;
		for (int Col = 0; Col<6; Col++) {
			int NewCol = Col*40;
			for (int y = 0; y<40; y++) {
				int blue = 2*ceil(y/2);
				setColor(0, 255,255);
				drawHSLine(0+NewCol, y+NewRow, 20-(blue/2));
				setColor(0, 0, 255);
				drawHSLine(20-(blue/2)+NewCol, y+NewRow, blue);
				setColor(0,255,255);
				drawHSLine(20+(blue/2)+NewCol, y+NewRow, 20-(blue/2));
			}
		}
	}
}


void RETEXT() {
	setColor(0, 0, 0);
	setColorBg(0, 0, 0);
	setFont(SmallFont);
	lcdPrint("AAAAAAA", 100, 140);
}


void QF_onIdle(void) {        /* entered with interrupts locked */

    QF_INT_UNLOCK();                       /* unlock interrupts */

    {
    	// Write code to increment your interrupt counter here.
    	// QActive_postISR((QActive *)&AO_Lab2A, ENCODER_DOWN); is used to post an event to your FSM



// 			Useful for Debugging, and understanding your Microblaze registers.
//    		u32 axi_ISR =  Xil_In32(intcPress.BaseAddress + XIN_ISR_OFFSET);
//    	    u32 axi_IPR =  Xil_In32(intcPress.BaseAddress + XIN_IPR_OFFSET);
//    	    u32 axi_IER =  Xil_In32(intcPress.BaseAddress + XIN_IER_OFFSET);
//
//    	    u32 axi_IAR =  Xil_In32(intcPress.BaseAddress + XIN_IAR_OFFSET);
//    	    u32 axi_SIE =  Xil_In32(intcPress.BaseAddress + XIN_SIE_OFFSET);
//    	    u32 axi_CIE =  Xil_In32(intcPress.BaseAddress + XIN_CIE_OFFSET);
//    	    u32 axi_IVR =  Xil_In32(intcPress.BaseAddress + XIN_IVR_OFFSET);
//    	    u32 axi_MER =  Xil_In32(intcPress.BaseAddress + XIN_MER_OFFSET);
//    	    u32 axi_IMR =  Xil_In32(intcPress.BaseAddress + XIN_IMR_OFFSET);
//    	    u32 axi_ILR =  Xil_In32(intcPress.BaseAddress + XIN_ILR_OFFSET) ;
//    	    u32 axi_IVAR = Xil_In32(intcPress.BaseAddress + XIN_IVAR_OFFSET);
//
//    	    // Expect to see 0x00000001
//    	    u32 gpioTestIER  = Xil_In32(sw_Gpio.BaseAddress + XGPIO_IER_OFFSET);
//    	    // Expect to see 0x00000001
//    	    u32 gpioTestISR  = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_ISR_OFFSET ) & 0x00000003;
//
//    	    // Expect to see 0x80000000 in GIER
//    		u32 gpioTestGIER = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_GIE_OFFSET ) & 0x80000000;


    }
}

/* Do not touch Q_onAssert */
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    (void)file;                                   /* avoid compiler warning */
    (void)line;                                   /* avoid compiler warning */
    QF_INT_LOCK();
    for (;;) {
    }
}

/* Interrupt handler functions here.  Do not forget to include them in lab2a.h!
To post an event from an ISR, use this template:
QActive_postISR((QActive *)&AO_Lab2A, SIGNALHERE);
Where the Signals are defined in lab2a.h  */

/******************************************************************************
*
* This is the interrupt handler routine for the GPIO for this example.
*
******************************************************************************/
void TwistHandler(void *CallbackRef) {
	XGpio *GpioPtr = (XGpio *)CallbackRef;
	Xuint32 encoderStatus = 0;

	Xuint32 start = XTmrCtr_GetTimerCounterReg(sys_tmrctr.BaseAddress, 0);
	Xuint32 finish = start;

	while (finish < (start + RESET_VALUE/10000)) {
		finish = XTmrCtr_GetTimerCounterReg(sys_tmrctr.BaseAddress, 0);
	}

	encoderStatus = XGpio_DiscreteRead(&enc_gpio, 1);

	if (encoderStatus == 7) {
		state = S0;
		encoder_count = 4;
		QActive_postISR((QActive *)&AO_Lab2A, ENCODER_CLICK);
	}

		switch (state) {

			case S0: {
					if (encoder_count == 8 || encoder_count == 0) {
						encoder_count = 4;
					}
				switch(encoderStatus) {
					case 0b01: {
						if(encoder_count == 4) {
							encoder_count = encoder_count + 1;
							state = S1;
						}
						break;
					}
					case 0b10: {
						if(encoder_count == 4) {
							encoder_count = encoder_count - 1;
							state = S3;
						}
						break;
					}
					break;
				}
				break;
			}

			case S1: {
				switch(encoderStatus) {
					case 0b11: {
						if(encoder_count == 1) {
							encoder_count = encoder_count - 1;
							state = S0;
							QActive_postISR((QActive *)&AO_Lab2A, ENCODER_DOWN);
						}
						break;
					}
					case 0b00: {
						if(encoder_count == 5) {
							encoder_count = encoder_count + 1;
							state = S2;
						}
						break;
					}
					break;
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
					break;
				}
				break;
			}

			case S3: {
				switch(encoderStatus) {
					case 0b00: {
						if(encoder_count == 3) {
							encoder_count = encoder_count - 1;
							state = S2;
						}
						break;
					}
					case 0b11: {
						if(encoder_count == 7) {
							encoder_count = encoder_count + 1;
							state = S0;
							QActive_postISR((QActive *)&AO_Lab2A, ENCODER_UP);
						}
						break;
					}
					break;
				}
				break;
			}
			break;
		}
		XGpio_InterruptClear(GpioPtr, GPIO_CHANNEL1);	// Clearing interrupt
}

void GpioHandler(void *CallbackRef) {
	XGpio *GpioPtr = (XGpio *)CallbackRef;

	XGpio_InterruptClear(GpioPtr, BUTTON_CHANNEL);	// Clearing interrupt

	Xuint32 start = XTmrCtr_GetTimerCounterReg(sys_tmrctr.BaseAddress, 0);
	Xuint32 finish = start;

	while (finish < (start + RESET_VALUE/10000)) {
		finish = XTmrCtr_GetTimerCounterReg(sys_tmrctr.BaseAddress, 0);
	}

	Xuint32 ButtonPressStatus = 0;
	ButtonPressStatus = XGpio_DiscreteRead(&btn, BUTTON_CHANNEL); // Check GPIO output
	if (ButtonPressStatus == 0x04) {
		//RIGHT
		//print("Button Pressed");
		//TimerFlag = 1;
		QActive_postISR((QActive *)&AO_Lab2A, BTNR);
	}
	else if (ButtonPressStatus == 0x02) {
		//LEFT
		//print("Button Pressed");
		//TimerFlag = 1;
		QActive_postISR((QActive *)&AO_Lab2A, BTNL);
	}
	else if (ButtonPressStatus == 0x10) {
		//CENTER
		//print("Button Pressed");
		//TimerFlag = 1;
		QActive_postISR((QActive *)&AO_Lab2A, BTNC);
	}
	else if (ButtonPressStatus == 0x01) {
		//Up
		//print("Button Pressed");
		//TimerFlag = 1;
		QActive_postISR((QActive *)&AO_Lab2A, BTNU);
	}
	else if (ButtonPressStatus == 0x08) {
		//Down
		//print("Button Pressed");
		//TimerFlag = 1;
		QActive_postISR((QActive *)&AO_Lab2A, BTND);
	}
}
