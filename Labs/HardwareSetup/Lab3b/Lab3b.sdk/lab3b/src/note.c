#include "note.h"
#include "lcd.h"
#include "math.h"

// array to store note names for findNote
//static char notes[12][3] = {"A ", "A#", "B ", "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#"};

int currNote;
int currOct;
int currFreq;
int aValue;
int freqOffset;
int cents;

// finds and prints note of frequency and deviation from note
void findNote(float f)
{
	float c = aValue * 0.594;
	int oct = 4;
	int note = 0;
	float a;
	float b;
	int premod;
	int n;

	// determine which octave frequency is in
	if (f >= c)
	{
		while (f > c * 2)
		{
			c = c * 2;
			oct++;
		}
	}
	else
	{ // f < C4
		while (f < c)
		{
			c = c / 2;
			oct--;
		}
	}

	a = (f + freqOffset) / aValue;
	b = 12 * log2(a) + 49;
	n = (int)round(b);
	premod = (n - 1) % 12;
	note = premod;
	float realFreq = aValue * pow(root2, (n - 49));
	cents = (int)round(1200 * log2(f / realFreq));
	int d = (int)(f + 0.5);

	currNote = note;
	currOct = oct;
	currFreq = d;
	setOctave(oct);
}

void setOctave(int octave)
{
	octaveSelect = octave;
}
