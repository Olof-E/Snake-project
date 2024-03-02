#include "input.h"
#include "display.h"
#include "helpers.h"
#include "main.h"

int highscores[3];
int savingHighscore = -1;
char name[3];
char highscoresText[3][9];
char currentChar = 0x41;
int count = 0;

// Author: Tobias & Olof
// Highscore function that adds the users score to a highscore array
void addHighscore(char *charScore, int highscoreIndex)
{
    int i;
    int saveChar = 0;
    int nameSaved = 0;
    char *b = "Score: ";

    // btn4
    if (lastInput == 4)
    {
        lastInput = 0;
        // Go to the previous ASCII character, as long as the current charactet isn't 0x41 = A
        if (currentChar > 0x41)
        {
            currentChar--;
        }
    }
    // btn3
    if (lastInput == 3)
    {
        lastInput = 0;
        // Go to the next ASCII character, as long as the current charactet isn't 0x5A = Z
        if (currentChar < 0x5A)
        {
            currentChar++;
        }
    }
    // btn2
    if (lastInput == 2)
    {
        lastInput = 0;
        // Choose the selected character
        saveChar = 1;
    }

    name[count] = currentChar;
    // Save the selected character to the name and go to the next character position
    if (saveChar)
    {
        name[count] = currentChar;
        count++;
        saveChar = 0;
    }

    // If 3 characters have been selected and entered, then the name is done
    if (count == 3)
        nameSaved = 1;

    if (nameSaved == 1)
    {
        int scoreLength = stringLength(charScore);

        char finalName[5 + scoreLength];
        // Add the 3 character name and the score to a final name string
        for (i = 0; i < sizeof(finalName); i++)
        {
            if (i < 3)
            {
                finalName[i] = name[i];
            }
            else if (i == 3)
            {
                finalName[3] = 0x80;
            }
            else
            {
                finalName[i] = charScore[i - 4];
            }
        }
        // Add a null character to the end of the final string
        finalName[sizeof(finalName) - 1] = 0x00;
        i = 0;
        // Copies the players 3 character name and score to a string array which houses all the highscores
        while (i < sizeof(finalName))
        {
            highscoresText[highscoreIndex][i] = finalName[i];
            i++;
        }
        nameSaved = 0;
        count = 0;
        currentChar = 0x41;
        highscores[highscoreIndex] = score;
        resetGame();
        gameState = 0;
        // Clears the name string
        for (i = 0; i < 3; i++)
        {
            name[i] = 0x00;
        }
    }

    // Display a highscore message and prompt the user to enter 3 initials
    display_clear();
    menuItem(0, 0, "HIGHSCORE!");
    menuItem(0, 1, stringConcat(b, charScore));
    menuItem(0, 2, "Enter name");
    menuItem(1, 3, name);
    display_buffer(0, 128, displayBuff);
}