#include <pic32mx.h>
#include "input.h"
#include "display.h"
#include "helpers.h"
#include "highscore.h"
#include "main.h"

// Author: Tobias & Olof
// A function that gets called when the player dies
void deathScreenUpdate()
{
    int i, j;
    char *convScore = itoaconv(score);
    char *text = "Score: ";

    // Check if we should save a highscore
    if (savingHighscore == -1 && score > 0)
    {
        for (i = 0; i < 3; i++)
        {
            if (score > highscores[i])
            {
                savingHighscore = i;
                // Shift the list of highscores to make room for the new one
                for (j = 2; j > i; j--)
                {
                    if (highscores[j - 1] != -1)
                    {
                        highscores[j] = highscores[j - 1];
                        int counter = 0;
                        while (counter < sizeof(highscoresText[j]))
                        {
                            highscoresText[j][counter] = highscoresText[j - 1][counter];
                            counter++;
                        }
                    }
                }
                break;
            }
        }
    }

    // Handle the saving of highscores based on the previous check
    if (savingHighscore != -1)
    {
        addHighscore(convScore, savingHighscore);
    }
    else
    {
        // Get input from user
        if (lastInput == 4) // Btn 4
        {
            lastInput = 0;
            selectedMenuItem--;
            if (selectedMenuItem < 0)
                selectedMenuItem = 0;
        }
        else if (lastInput == 3) // Btn 3
        {
            lastInput = 0;
            selectedMenuItem++;
            if (selectedMenuItem > 1)
                selectedMenuItem = 1;
        }
        else if (lastInput == 2) // Btn 2
        {
            lastInput = 0;
            if (selectedMenuItem == 0)
            {
                resetGame();
                gameState = 1;
            }
            else
            {
                gameState = 0;
            }
        }

        // Display the death screen
        display_clear();
        menuItem(0, 0, "You died!");
        menuItem(0, 1, stringConcat(text, convScore));
        menuItem(selectedMenuItem == 0, 2, "Play again");
        menuItem(selectedMenuItem == 1, 3, "Back to menu");
        display_buffer(0, 128, displayBuff);
    }
}
// End Author

// Author: Olof
void scoreViewUpdate()
{
    int i;
    if (lastInput == 2)
    {
        lastInput = 0;
        gameState = 0;
    }

    // Display the list of highscores
    display_clear();
    for (i = 0; i < 3; i++)
    {
        if (highscores[i] != -1)
        {
            char *ranking = stringConcat(itoaconv(i + 1), ". ");
            menuItem(0, i, stringConcat(ranking, highscoresText[i]));
        }
    }

    menuItem(1, 3, "Back");
    display_buffer(0, 128, displayBuff);
}

// Author: Olof
void menuItem(int selected, int pageIndex, char *text)
{
    int i;

    // find length of the string ̈́'text'
    int textLength = stringLength(text);

    int blankSpace = (16 - textLength) / 2;
    char temp[16];

    // Center the text on screen by filling each side with blank spaces
    for (i = 0; i < 16; i++)
    {
        if (i < blankSpace || i > blankSpace + textLength)
        {
            temp[i] = 0x80;
        }
        else
        {
            temp[i] = text[i - blankSpace];
        }
    }
    display_string(pageIndex, temp);

    // Draw two boxes by a menu item if its selected
    if (selected)
    {
        display_rect((blankSpace - 1) * 8 - 2, pageIndex * 8 + 4, 2);
        display_rect((blankSpace + textLength + 1) * 8, pageIndex * 8 + 4, 2);
    }
}

// Author: Olof
void menuUpdate()
{
    // Scroll the menu list or click on menu item depending on input
    if (lastInput == 4) // Btn 4
    {
        lastInput = 0;
        selectedMenuItem--;
        if (selectedMenuItem < 0)
            selectedMenuItem = 0;
    }
    else if (lastInput == 3) // Btn 3
    {
        lastInput = 0;
        selectedMenuItem++;
        if (selectedMenuItem > 1)
            selectedMenuItem = 1;
    }
    else if (lastInput == 2) // Btn 2
    {
        lastInput = 0;
        if (selectedMenuItem == 0)
        {
            // Calculates the time that it took to start the game for the first time
            startTime = (int)PR2 * totalTimeout + TMR2;
            randomInit();
            resetGame();
            gameState = 1;
        }
        else if (selectedMenuItem == 1)
        {
            gameState = 3;
        }
    }

    // Update menu screen
    display_clear();
    menuItem(0, 0, "Snek");
    menuItem(selectedMenuItem == 0, 2, "Start Game");
    menuItem(selectedMenuItem == 1, 3, "Scores");
    display_buffer(0, 128, displayBuff);
}

void gameUpdate()
{
    // Author: Olof
    int i;

    // Change snake direction depending on input
    if (lastInput == 4) // Btn 4
    {
        lastInput = 0;
        if (dir == 2 || dir == -2)
        {
            dir = -1;
            position.y++;
        }
        else
        {
            dir++;
        }
    }
    else if (lastInput == 3) // Btn 3
    {
        lastInput = 0;
        if (dir == -2 || dir == 2)
        {
            dir = 1;
            position.y--;
        }
        else
        {
            dir--;
        }
    }

    // Update the snakes head position
    position.x += 2 * (abs(dir) % 2 == 0) * -((abs(dir) / 2) * 2 - 1);
    position.y += 2 * (abs(dir) == 1) * -dir;

    // Check for collisions
    collision();

    // Propogate the snakes movement throught the bodu
    for (i = snakeLength - 1; i >= 0; i--)
    {
        if (i == 0)
        {
            snakeSegments[0] = position;
            continue;
        }
        snakeSegments[i] = snakeSegments[i - 1];
    }

    // Draw the borders,food and snake to the screen
    display_clear();
    food();
    for (i = 0; i < snakeLength; i++)
    {
        display_rect(snakeSegments[i].x - 1, snakeSegments[i].y - 1, 3);
    }
    draw_border();
    display_buffer(0, 128, displayBuff);
    // End Author
}

// Author: Tobias
// A function that resets commonly used varaibles and arrays and clears the display
void resetGame()
{
    position.x = 63;
    position.y = 15;
    dir = 0;
    spawnFood = 1;
    score = 0;
    savingHighscore = -1;
    snakeLength = 0;
    snakeSegments[snakeLength++] = position;
    display_clear();
}