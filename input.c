#include <pic32mx.h>
#include <math.h>
#include "display.h"
#include "input.h"
#include "main.h"

int lastInput = 0;
int inputReset = 1;

// From: lab 3
int getSw(void)
{
    return (PORTD & 0xF00) >> 8;
}

// From: lab 3
int getBtns(void)
{
    return (PORTD & 0xE0) >> 5;
}

// Author: Olof
void inputUpdate()
{
    // Get the latest input as long as the input has reseted
    if ((getBtns() & 0x04) && inputReset)
    {
        lastInput = 4;
        inputReset = 0;
    }
    if (getBtns() & 0x02 && inputReset)
    {
        lastInput = 3;
        inputReset = 0;
    }
    if (getBtns() & 0x01 && inputReset)
    {
        lastInput = 2;
        inputReset = 0;
    }

    if (!getBtns())
    {
        inputReset = 1;
    }
}
