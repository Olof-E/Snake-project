

int gameState;
int selectedMenuItem;
int dir;
int score;

int startTime;
int totalTimeout;

int spawnFood;

struct Position
{
    char x;
    char y;
};

struct Position position;
int snakeLength;
struct Position snakeSegments[420];

void *stdout;

void death();
void resetGame();

void scoreViewUpdate();
char *stringConcat(char *string1, char *string2);
void menuItem(int selected, int pageIndex, char *text);

void gameUpdate();
void menuUpdate();
