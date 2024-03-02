#include <pic32mx.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "input.h"
#include "display.h"
#include "main.h"
#include "highscore.h"
#include "gameUpdate.h"
#include "helpers.h"

// Global variables
int gameState = 0;
int selectedMenuItem = 0;

int updateTimeout = 0;
int totalTimeout = 0;

int startTime = 0;

struct Position position;

int foodX = 0;
int foodY = 0;

int spawnFood = 1;
int score = 0;

int dir = 0;

int snakeLength = 0;
struct Position snakeSegments[420];

const int WIDTH = 127;
const int HEIGHT = 31;

void *stdout = (void *)0;
void *errno = (void *)0;

// Interrupt service to handle game loop
void user_isr()
{

	// Author: Olof
	if (IFS(0) & 0x100)
	{
		IFSCLR(0) = 0x100;
		updateTimeout++;
		totalTimeout++;

		// Upadte input at 30 FPS
		inputUpdate();
	}

	// Update the game logic at 3 FPS
	if (updateTimeout >= 10)
	{
		if (gameState == 0)
		{
			menuUpdate();
		}
		else if (gameState == 1)
		{
			gameUpdate();
		}
		else if (gameState == 2)
		{
			deathScreenUpdate();
		}
		else if (gameState == 3)
		{
			scoreViewUpdate();
		}
		updateTimeout = 0;
	}
	// End Author
}

// Author: Tobias
//
// Uses ADC with a floating analog pin A1 together with a time value fetched
// when the user pushes the start game for the first time to generate a random seed
// for the srand function
void randomInit()
{
	// Sets the A1 pin to input
	TRISBSET = 0x10;

	// Sets pin A1 to analog mode and the rest to digital mode
	AD1PCFG = 0xFFEF;

	// Sets the clock source to the RC-clock
	AD1CON3 = 0x8000;

	// Enables auto conversion at the
	// end of the RC-clock count
	AD1CON1 = 0x00E0;

	// Selects the unused analog pin A1 as the positive MUX A input
	AD1CHS = 0x40000;

	// Makes sure that the CON2 register is fully clear
	AD1CON2 = 0;

	// Don't use input scan
	AD1CSSL = 0;

	// Sets the output form as a 32 bit int
	AD1CON1SET = 0x400;

	// Enable the ADC
	AD1CON1SET = 0x8000;

	// Create a small delay between turning the ADC on and the sampling start
	// This prevents false random values that can occur if the reference voltage isn't stabilized.
	quicksleep(100);

	// Starts sampling
	AD1CON1SET = 0x2;

	// Wait for the sampling to start
	while (!(AD1CON1 & (0x1 << 1)))
		;

	// Wait for the conversion to finish
	// by checking if the DONE bit is set
	while (!(AD1CON1 & 0x1))
		;

	// Fetch the converted ADC value
	int adcValue = ADC1BUF0;

	// Calculates a random value via the ADC value and a LCG algorithm
	// Uses the parameter values that is commonly used in C
	unsigned int adcSeed = (unsigned)(1103515245 * adcValue + 12345) % 2147483648U;

	// Calculates a final seed by using a XOR-gate between the time value and the ADC value
	unsigned int finalSeed = (unsigned)startTime ^ adcSeed;

	// Sets the randomizer seed to our generated value
	srand(finalSeed);

	// Turn off the ADC
	AD1CON1CLR = 0x8000;
}
// End Author

void init()
{

	// Set buttons and swithces to input
	TRISDSET = 0xFE0;

	/*
  This will set the peripheral bus clock to the same frequency
  as the sysclock. That means 80 MHz, when the microcontroller
  is running at 80 MHz. Changed 2017, as recommended by Axel.
*/
	SYSKEY = 0xAA996655; /* Unlock OSCCON, step 1 */
	SYSKEY = 0x556699AA; /* Unlock OSCCON, step 2 */
	while (OSCCON & (1 << 21))
		;				  /* Wait until PBDIV ready */
	OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
	while (OSCCON & (1 << 21))
		;		  /* Wait until PBDIV ready */
	SYSKEY = 0x0; /* Lock OSCCON */

	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;

	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;

	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);

	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
	SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;

	display_init();

	// Author: Olof
	// Set timer 2 to count 30 times a second (30 fps)
	T2CON = 0x00;
	TMR2 = 0x00;
	PR2 = 0x28b0; // 0x7a12;
	T2CONSET = 0x70;
	IFSCLR(0) = 0x100;

	IPCSET(2) = 0x1F;

	IECSET(0) = 0x100;

	T2CONSET = 0x8000;

	// Enable interrups globally
	enable_interrupt();

	// End Author
}

// Author: Tobias
// Calculate the distance between the food and a given x and y position
float foodDist(int x, int y)
{
	// Pythagorean theorem
	float distX = (float)x - ((float)foodX + 0.5f);
	float distY = (float)y - ((float)foodY + 0.5f);
	float dist = sqrtf(distX * distX + distY * distY);

	return dist;
}
// End Author

// Author: Tobias
// A function that spawns a new apple at a randmoized position or displays the current
// uneaten apple
void food()
{
	int valid = 0;
	int i;

	// Spawn a new apple at a new location if the previous apple has been eaten.
	if (spawnFood == 1)
	{
		// Loop until the food gets a random position that isn't inside the snake's tail
		while (!valid)
		{
			// Randomizes a new x-coordinate and a y-coordinate for the apples
			foodX = (rand() % (WIDTH - 10)) + 5;
			foodY = (rand() % (HEIGHT - 10)) + 5;
			valid = 1;

			// Checks if the food collides with any tail segment
			for (i = 0; i < snakeLength; i++)
			{
				float dist = foodDist(snakeSegments[i].x, snakeSegments[i].y);
				if (dist <= sqrtf(3.25f))
				{
					valid = 0;
					break;
				}
			}
		}
		// Randomize a value between 5 - 116
		foodX = (rand() % (WIDTH - 10)) + 5;
		// Randomize a value between 5 - 20
		foodY = (rand() % (HEIGHT - 10)) + 5;

		display_rect(foodX, foodY, 2);
		spawnFood = 0;
	}
	else
	{
		// Draw the food with the previously generated position
		display_rect(foodX, foodY, 2);
	}
}
// End Author

// Author: Tobias
// A function that checks collisions between the player and the various obstacles and the apples
void collision()
{
	int i;

	// Calculate distance between food and snake head
	float dist = foodDist(position.x, position.y);

	// Check collision with the 3x3 pixel snake head and the 2x2 pixel apple
	if (dist <= sqrtf(3.25f))
	{
		// The apple is eaten
		spawnFood = 1;
		score++;
		snakeSegments[snakeLength++] = snakeSegments[snakeLength];
	}

	// Check collision with the screen walls
	if (position.x <= 1 || position.x >= 125)
	{
		// The snake dies
		gameState = 2;
	}

	if (position.y <= 1 || position.y >= 30)
	{
		gameState = 2;
	}

	// Check collision with snake tail
	for (i = 2; i < snakeLength; i++)
	{
		int distX = abs(position.x - snakeSegments[i].x);
		int distY = abs(position.y - snakeSegments[i].y);
		if (distX <= 1 && distY <= 1)
		{
			gameState = 2;
			break;
		}
	}
}
// End Author

int main(void)
{
	int i;
	// Initialize the hardware
	init();

	// Display the start screen
	menuUpdate();
	display_buffer(0, 128, displayBuff);

	for (i = 0; i < 3; i++)
	{
		highscores[i] = -1;
	}

	// Keep the program running
	while (1)
	{
	}
	return 0;
}
