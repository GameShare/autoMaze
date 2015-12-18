#ifndef MOVECONTROL
#define MOVECONTROL

#include "highgui.h"
#include "cv.h"
#include "Control.h"
#include <sstream>
#include <Windows.h>

ComPort port("\\\\.\\COM9");
OVERLAPPED Wol;

void sendMessage1(char c)
{
	DWORD  BytesToSend = 0;
	//ComPort port("\\\\.\\COM9");
	unsigned char* temp;
	temp = new unsigned char[1];
	for (int i = 0; i<1; i++) temp[i] = c;
	Wol.Internal = 0;
	Wol.InternalHigh = 0;
	Wol.Offset = 0;
	Wol.OffsetHigh = 0;
	Wol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	long unsigned int len = 1;
	WriteFile(port.hCom, temp, len, NULL, &Wol);
	Wol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	len = 1;
	WriteFile(port.hCom, temp, len, NULL, &Wol);
}

void GoAhead()
{
	sendMessage1('A');
}

void TurnLeft()
{
	sendMessage1('L');
}

void TurnRight()
{
	sendMessage1('R');
}

#endif