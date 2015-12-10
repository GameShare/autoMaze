#include "highgui.h"
#include "cv.h"
#include "Control.h"

extern ComPort port;
extern OVERLAPPED Wol;

void sendMessage(char c)
{
    DWORD  BytesToSend = 0;
    //ComPort port("\\\\.\\COM13");
    unsigned char* temp;
    temp = new unsigned char[10];
    for (int i=0; i<10; i++) temp[i]=c;
    Wol.Internal = 0;
    Wol.InternalHigh = 0;
    Wol.Offset = 0;
    Wol.OffsetHigh = 0;
    Wol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    long unsigned int len = 10;
    WriteFile(port.hCom, temp, len, NULL, &Wol);
    Wol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    len = 10;
    WriteFile(port.hCom, temp, len, NULL, &Wol);
}

void goAhead()
{
    sendMessage('A');
}

void turnLeft()
{
    sendMessage('L');
}

void turnRight()
{
    sendMessage('R');
}

