/*****************************************************************************
 * lab2a.h for Lab2A of ECE 153a at UCSB
 * Date of the Last Update:  October 23,2014
 *****************************************************************************/

#ifndef lab2a_h
#define lab2a_h

enum Lab2ASignals
{
	ENCODER_UP = Q_USER_SIG,
	ENCODER_DOWN,
	ENCODER_CLICK,
	BTN_UP,
	BTN_DOWN,
	BTN_CENTER,
	BTN_LEFT,
	BTN_RIGHT,
	IDLE,
};

extern struct Lab2ATag AO_Lab2A;

void Lab2A_ctor(void);
void GpioHandler(void *CallbackRef);
void TwistHandler(void *CallbackRef);
void extra_handler(void *CallbackRef);
void Template();
void Menu_stateA();
void Menu_stateB();
void Menu_stateC();
void Menu_stateD();
void SelectedA();
void SelectedB();
void SelectedC();
void SelectedD();
void updateHistogram();
void updateSpacing();
void drawRainbowNote();
void MenuTitle();
void drawErrorBar(int error);
void drawError(int error);
void SelectedMenu();
void NotSelectedMenu();
void BackMenu();
void A4_Title();
void Octave_Title();
void Tuner_Title();
void Histogram_Title();
void UpdateFrequency(int freq);
void UpdateOct(int oct);
void UpdateNote(int currNote);
void updateA(int a);


#endif
