/***********************************************************************************************************************
*                                                                                                                      *
*  - SNAKE GAME in C -                                                                                                 *
*  by Abigail Fenerty, Nick Sabia, Justin Merville, Jacob Pelletier, Mateusz Mirga                                     *
*                                                                                                                      *
*  CS 355 Systems Programming final project, Spring 2021.                                                              *
*                                                                                                                      *
*  To run: 1) compile 2) execute compiled code 3) follow prompts                                                       *
*
*      NOTE: THIS IS FROM snake.c FROM LOGIC BRANCH PUSHED TO MAIN BRANCH ON 4/25
*
*  ** TODO: type up rules here **                                                                                      *
***********************************************************************************************************************/

/** headers **/
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>

#define INFO_ROW 1
#define STDIN_FD 0

/* Statistics cursor positions */
#define CLOCK_POS 28
#define TICK_POS 46
#define DIRECTION_POS 65
#define LAST_PRESSED_POS 89

/* Pause menu */
#define PAUSE_MSG_LEN 32

/** prototypes **/
/* RE: snake pit */
void pit_size();
void init_pit_border();

/* RE: snake */
void eat_fruit(int y, int x);
void init_snake(int, int);
void move_snake();
void shorten_tail();
void detect_collisions();

void move_snake_2();
void grow_snake(int);

/* RE: logic */
char choose_random_direction();
void game_condition(int option);
void time_event();

/* RE: Input */
void tty_mode (int action);
void set_settings();
void set_nodelay_mode();
int get_movement_input();
void end_snake(int signum);

/* Trophies */
void init_trophy();
struct trophy new_trophy();
void print_trophy();
int snake_hit_trophy();

/** statics, structs, and constants **/
/* window dimensions*/
static int window_row;
static int window_col;

/* game stats */
static int score = 0;
static char key;
short mode = 1;                       // 1 = true, 0 = false
unsigned int gameTime = 0;            // Tracks how many iterations of the while loop have been performed.
unsigned short ticks = 0;             // Keeps track of when checks are performed in the game. When ticks == 0, progress the game forward by 1 gameunit.
unsigned int timeUnit = 128;          // A timeUnit consists of x amount of ticks. So in this case, 8 ticks == 1 timeUnit.


/* snake head location */
int head_y;                     // TODO: change to random
int head_x;                    // TODO: change to random

/* logic */
static short dirY;
static short dirX;

/* timing */
struct timespec speed, rem; // speed governs the rate at which the screen refreshes. rem is unused, but will hold the time saved from the user's interupt signal

/*
 * -- STRUCT NODE --
 * structure defining node of link list that will represent the list
 * row represents y coordinate
 * column represents x coordinate
 * token is what will represent the snake segment (node) on the board
 * *next points towards head
 * *prev points towards tail
 */
struct node{
    int row;
    int column;
    struct node *prev;
    struct node *next;
};

struct trophy{
    int row;
    int column;
    int value;
};

/* head of snake */
struct node *head;
struct node *tail;

/* trophy */
struct trophy *trophy;

/***********************************************************************************************************************
*  DRIVER CODE                                                                                                         *
***********************************************************************************************************************/
/*
 *  main()
 *  Purpose: initiate and run a game of snake according to the defined rules
 *  Method: use snake pit, snake, trophy, and logic methods to initialize a game of snake
 *  Returns: interactive game on terminal
 */
int main(){
    /* Initialize global random number generator by Nick Sabia */
    srand((unsigned) time(NULL));

    /* Manage terminal settings by Nick Sabia */
    tty_mode(0); // Save original settings
    signal(SIGINT, end_snake); // Revert to original settings on program termination
    signal(SIGQUIT, SIG_IGN);

    /* start curses, set settings */
    initscr();
    /* clear screen */
    clear();
    /* turn off cursor */
    curs_set(0);
    /* get screen dimensions, alternatively could use LINES and COLS from curses */
    pit_size();
    /* draw the border */
    init_pit_border(window_col, window_row);
    /* center the snake */
    head_y = window_row/2;
    head_x = window_col/2;
    /* set random initial direction */
    key = choose_random_direction();

    /* setup the sleep timer by Nick Sabia*/
    speed.tv_sec = 0;
    //speed.tv_nsec = 50000000;   // 0.05 seconds in nanoseconds
    speed.tv_nsec = 781250; // 1/128th of a second in nanoseconds

    char gameTimeStr[6];        // Used to store gameTime as a string.
    char input;                 // The key the user pressed.
    char keyStr[4];             // Used to store key as a string.
    char ticksStr[2];           // Used to store ticks as a string.

    /* use key pad  by Mateusz Mirga   */
    keypad(stdscr,TRUE);        //Handel arrow input MM

    /* init snake of size 5 */
    init_snake(head_y, head_x);
    init_trophy();
    new_trophy();
    print_trophy();

    /* Print initial pause message */
    move(window_row / 2, window_col / 2 - PAUSE_MSG_LEN/2);
    addstr("Press any key to start playing!");
    move(window_row - 1, window_col - 1);

    /* Send the first image from the buffer to terminal */
    refresh();

    /* Keeps the user in a paused state until they hit a button ~ Nick Sabia */
    getchar();

    /* Enable settings after the user un-pauses */
    set_settings(); // Set terminal settings for the program.
    noecho();
    set_nodelay_mode(); // Setting this prevents getch() from blocking the program for input.

    /* Erase initial pause message */
    move(window_row / 2, window_col / 2 - PAUSE_MSG_LEN/2);
    addstr("                                ");
    move(window_row - 1, window_col - 1);

    /* The draw loop */
    while (mode) {
        /* 
         * This inner loop prevents the snake from continuing quickly in a direction after a user stops pressing a directional key. 
         * Without this loop, a lot of extra inputs would get registered that would lead to the snake barreling off into one direction,
         * ignoring the user's attempt at slowing down or changing direction. ~ Nick Sabia
        */
        while ((input = getch() ) == ERR) {
            nanosleep(&speed, &rem);
            ticks++;
            if (ticks % timeUnit == 0) {
                // One time unit has passed. Increment time elapsed
                time_event();
            }
        }

        // Handling of user input: Only specified inputs receive a reaction; Wrong input or no input goes to default case (no input) MM
        switch (input) {
            case (char) KEY_LEFT:
            case 'a':
                // Draw the direction moved
                move(0, DIRECTION_POS);
                addstr("LEFT ");

                /* Sets the last key pressed and the direction variables used to move the snake. */
                key = 'a';
                dirY = 0;
                dirX = -1;

                /* Go forward in time */
                time_event();
                break;

            case (char) KEY_DOWN:
            case 's':
                // Draw the direction moved
                move(0, DIRECTION_POS);
                addstr("DOWN ");

                /* Sets the last key pressed and the direction variables used to move the snake. */
                key = 's';
                dirY = 1;
                dirX = 0;

                /* Go forward in time */
                time_event();
                break;

            case (char) KEY_UP:
            case 'w':
                // Draw the direction moved
                move(0, DIRECTION_POS);
                addstr("UP   ");

                /* Sets the last key pressed and the direction variables used to move the snake. */
                key = 'w';
                dirY = -1;
                dirX = 0;

                /* Go forward in time */
                time_event();
                break;

            case (char) KEY_RIGHT:
            case 'd':
                // Draw the direction moved
                move(0, DIRECTION_POS);
                addstr("RIGHT");

                /* Sets the last key pressed and the direction variables used to move the snake. */
                key = 'd';
                dirY = 0;
                dirX = 1;
                
                /* Go forward in time */
                time_event();
                break;
            case ' ':
                game_condition(4);
                break;
            default:
                break;
        }

        // Draw the current time elapsed
        move(0, CLOCK_POS);
        sprintf(gameTimeStr, "%d", gameTime); // Convert the integer from the gameTime counter into a string.
        addstr(gameTimeStr);

        // Draw the current number of ticks
        move(0, TICK_POS);
        sprintf(ticksStr, "%d", ticks); // Convert the integer from ticks into a string.
        addstr(ticksStr);

        // Reset cursor position
        move(window_row - 1, window_col - 1);

        // send tokens for border from buffer to terminal
        refresh();
    }
}

/***********************************************************************************************************************
*  SNAKE PIT                                                                                                           *
*  1. draw_pit_border                                                                                                  *
*  2. pit_size                                                                                                         *                                                                                                        *
***********************************************************************************************************************/
/*
 *  1. init_pit_border()
 *  Purpose: initialize snake pit
 *  Method: matrix of array, populated by nested for loops
 *  Returns: grid matrix printed to terminal
 */
void init_pit_border(int x, int y) {
    addstr("Welcome to Snake  |  Clock: ------  |  Ticks: ---  |  Direction: -----  |  Last Pressed: -  |  Press Space to exit.\n");    // name of game, score, space for user inputs
    /* place border tokens in appropriate cells */
    for (int i = 1; i < y; i++) {
        for (int j = 0; j < x-1; j++) {
            move(i, j);
            if (i == 1 || i == y - 1)
                addstr("X");
            else if (j == 0 || j == x - 2)
                addstr("X");
        }
    }
}
/*
 *  2. pit_size()
 *  Purpose: get number of rows and columns of current terminal window
 *  Method: use ioctl to access terminal attributes
 *  Returns: assign window row and column values to static (global) row and col variables
 *  Reference: Moley Chapter 5, page 164.
 */
void pit_size(){

    /* the address wbuf is the argument to that device controlled function. */
    struct winsize wbuf;

    /*
     *  The ioctl system call provides access to attributes of the device driver (terminal in this case) connected to fd.
     *  TIOCGWINSZ is the function code needed to access this attribute.
     */
    if (ioctl(0, TIOCGWINSZ, &wbuf) != -1){
        /* DEBUG LINE */
        //printf("%d rows x %d cols\n", wbuf.ws_row, wbuf.ws_col);
        /* assign global variable values here: */
        window_row = wbuf.ws_row;                                              // assign global row
        window_col = wbuf.ws_col;                                              // assign global column
    }
}

/***********************************************************************************************************************
*  SNAKE
*  by Jacob Pelletier
*  Use structure to represent node
*  Nodes used to represent a linked list
*  Nodes are only added to the head and nodes are never removed.
*  1) grow snake
*  2) shorten snake
*  3) create baby snake
*  4) move snake - handles printing tokens and pointers
*  5) auto move - handles snake coordinates and directionality
***********************************************************************************************************************/
/* starter_snake()
*  Purpose: initializes baby snake for game
*  Method: part of initializing the game
*  Input: none
*  Returns: LL with new length 3.
*/
void init_snake (int start_y, int start_x) {
    // Allocate memory for the head and tail.
    head = (struct node*)malloc(sizeof(struct node));
    tail = (struct node*)malloc(sizeof(struct node));

    // Connect the snake head and tail to form the body.
    head->prev = tail;
    tail->next = head;

    // Set the position of the snake's head.
    head->row = start_y;
    head->column = start_x;

    // Add some starter snake pieces to the snake.
    grow_snake(3);
}
/* grow_snake()
 * Purpose: Grows the snake by x snake pieces.
 * Method: Every iteration, add a new node inbetween the tail and body.
 * Input: x number of snake pieces to add.
 * Returns: Longer snake.
*/
void grow_snake(int x) {
    for (int i = 0; i < x; i++) {
        // This is the node which will be closer to the head than the new_piece.
        struct node* body = tail->next;

        // Allocate a space in memory for this node.
        struct node* new_piece = (struct node*)malloc(sizeof(struct node)); 

        // Set the coordinates of this new snake piece
        new_piece->row = tail->row;
        new_piece->column = tail->column;

        // Insert the new_piece between the tail and the body node
        new_piece->next = tail->next; // Point the new_piece to the right nodes.
        new_piece->prev = tail;
        tail->next = new_piece; // Change the tail and body node pointers to point to the new_piece.
        body->prev = new_piece;
    }
}
/* move_snake()
 * Purpose: Move the snake head and let the snakes body follow suit.
 * Method: Movement of the head is governed by the dirY and dirX values. the rest of the pieces take the place of whatever their next pointer was.
 * Input: None
 * Returns: A new frame.
 */
void move_snake() {
    // The snake will always move to fill the placeholder.
    // save placeholder position 
    int placeholder_y = head->row;
    int placeholder_x = head->column;

    // Move the head forward to the next position and draw it.
    head->row += dirY;
    head->column += dirX;
    move(head->row,head->column);
    addstr("O");

    // scan through snake printing tokens
    struct node* scanner = head->prev;
    while(scanner != NULL) {
        // Store the placeholder values from last iteration that this node to move to.
        int temp_y = placeholder_y;
        int temp_x = placeholder_x;

        // This node's current position will be the new placeholder for the next node.
        placeholder_y = scanner->row;
        placeholder_x = scanner->column;

        // Move the node to the new, correct position
        scanner->row = temp_y;
        scanner->column = temp_x;

        // Print the snake piece
        move(scanner->row,scanner->column);
        addstr("o");

        // Prepare for the next iteration
        scanner = scanner->prev;
    }

    // Avoid leaving a trail behind the snake. 
    move(placeholder_y, placeholder_x);
    addstr(" ");
    refresh();
}
/*
 * Purpose: Checks if the snake ran into itself.
 * Method: Check each piece of the snake to see if it is in the same position as the head. If so, the snake ran into itself.
 * Input: None.
 * Returns: 1 if snake ran into self, 0 otherwise.
*/
int snake_hit_self() {
    // Use a scanner node to check each part of the snake.
    struct node* scanner = head->prev;
    while(scanner != NULL) {
        if (scanner->row == head->row && scanner->column == head->column) {
            // The head is in the exact same spot as a piece of it's body.
            // This means the snake has collided into itself.
            return 1;
        }
        else {
            // Move onto the next piece of the snake.
            scanner = scanner->prev;
        }
    }
    return 0;
}
/* 4) detect_collisions()
*  Purpose: Performs checks after the snake is moved to see if the snake is colliding with anything.
*  Method: Checks whether the snake hits the walls (game_condition(1)), itself (game_condition(2)), or a trophy (game_condition(3)).
*  Input: none
*  Returns: a snake which appears to move depending on key value. meant to occur each second until win or lose condition
*/
void detect_collisions(){
    /* check for border collisions */
    if (head->row == 1 || head->row == LINES-1 || head->column == 0 || head->column == COLS-2){
        game_condition(1);
    }

    /* check for running into itself */
    if (snake_hit_self()) {
        game_condition(2);
    }

    /* check for trophy collision */
    if (snake_hit_trophy()) {
        game_condition(3);
    }
}

/***********************************************************************************************************************
*  LOGIC
*  1) random function for start direction by Nick Sabia
*  2) check for collision with wall or fruit with each movement
 * 3) another random function for randomly placing trophies
***********************************************************************************************************************/
/*
 *  1. choose_random_direction()
 *  Purpose: return a random direction to start snake in
 *  Method: call update() after user input
 *  Returns: char of random direction determined.
 */
char choose_random_direction(){
    int random_integer = rand() % 4; // DETERMINE RANDOM NUMBER BETWEEN 0 AND 3
    if (random_integer == 0){
        dirY = -1;
        dirX = 0;
        return 'w';
    } else if (random_integer == 1){
        dirY = 1;
        dirX = 0;
        return 's';
    } else if (random_integer == 2){
        dirY = 0;
        dirX = -1;
        return 'a';
    } else {
        dirY = 0;
        dirX = 1;
        return 'd';
    }
}
/*
 *  2. game_condition(int option)
 *  Purpose: return target game condition determined by option
 *  Method: call game_condition with argument; 1 = wall collision, 2 = reversal, 3 = run into self, 4 = user end
 *  Returns: end of program with comment
 */
void game_condition(int option){
    switch(option) {
        /* border collisions */
        case(1):
            move(window_row / 2, window_col / 2);
            addstr("YOU GOOFED!\tYou hit the wall.");
            refresh();
            sleep(2);
            raise(SIGINT);
            break;
            /* run into itself */
        case(2):
            move(window_row / 2, window_col / 2);
            addstr("YOU GOOFED!\tYou bit yourself.");
            refresh();
            sleep(2);
            raise(SIGINT);
            break;
            /* snake reaches trophy */
        case(3):
            grow_snake(trophy->value);
            new_trophy();
            print_trophy();
            break;
            /* user exit */
        case(4):
            move(window_row / 2, window_col / 2);
            addstr("Good Bye.");
            refresh();
            sleep(2);
            raise(SIGINT);
            break;
    }
}
/*
 *  3. time_event()
 *  Purpose: increment gameTime and move snake, occurs every second.
 *  Method: call each second of gameplay and w/ user input
 *  Returns: snake will move on screen, gameTime will increment.
 */
void time_event(){
    ticks = 0;
    gameTime++;
    move_snake();
    detect_collisions();
    refresh();
}
/***********************************************************************************************************************
*  TROPHIES
*  
***********************************************************************************************************************/
void init_trophy() {
    trophy = (struct trophy*)malloc(sizeof (struct trophy*));
}

/* Outputs a new, randomly placed trophy */
struct trophy new_trophy() {
    // We must place the trophy in a free, empty space.
    // Detect if the randomly generated coordinates are on the snake. If so, re-roll.
    int valid_space = 0;
    while(!valid_space) {
        // Subtract 1 from both randomly picked values to avoid getting the trophy stuck in the wall
        trophy->row = (rand() % window_row-1) - 1;
        trophy->column = (rand() % window_col-1) - 1;

        // Use a scanner node to check if the trophy is in the snake.
        struct node* scanner = head;
        while(scanner != NULL) {
            if (scanner->row == trophy->row && scanner->column == trophy->column) {
                // Break the loop pre-maturely. 
                // As a result, scanner will not be null. 
                break;
            }
            else {
                // Move onto the next piece of the snake.
                scanner = scanner->prev;
            }
        }

        if (scanner == NULL) {
            // If the scanner is null, then the while loop did not pre-maturely break and we have found a valid space for the new trophy.
            valid_space = 1;
        }

        // If the scanner is not null, then the while loop got broken out of pre-maturely. 
        // Since valid_space is still 0, we will re-roll and try finding another spot to place the trophy.
    }
    
    // Assign a random value to the trophy between 1 and 9.
    trophy->value = (rand() % 9) + 1;
}

void print_trophy() {
    // Convert the random value into a string for printing to the screen.
    char* valueStr;
    move(trophy->row, trophy->column);
    sprintf(valueStr, "%d", trophy->value);
    addstr(valueStr);
    //refresh();
}

int snake_hit_trophy() {
    if (head->row == trophy->row && head->column == trophy->column) {
        // The head is in the exact same spot as the trophy.
        // This means the snake has successfully reached the trophy.
        return 1;
    }

    // The snake's head is not on a trophy.
    return 0;
}

/***********************************************************************************************************************
*  TERMINAL SETTINGS
*  by Nick Sabia
***********************************************************************************************************************/

/*
 * Saves the original terminal settings.
 * This is useful for when we want to revert to the original settings when the user exits this program.
 * PARAMS:
 * action is an integer which governs whether the original settings are saved or loaded. 0 means to save, 1 means to load.
 */
void tty_mode (int action) {
    static struct termios original_settings;
    static int original_flags;
    static int stored = 0;
    if (action == 0) {
        // Save the original terminal settings
        tcgetattr(STDIN_FD, &original_settings);
        original_flags = fcntl(STDIN_FD, F_GETFL);
        stored = 1;
    }
    else if (stored) {
        // Restore the original terminal settings
        tcsetattr(STDIN_FD, TCSANOW, &original_settings);
        fcntl(0, F_SETFL, original_flags);
    }
}

/*
 * Set terminal driver settings.
 */
void set_settings() {
    struct termios settings;
    int result = tcgetattr(STDIN_FD, &settings); /* Read values from driver */
    if (result == -1) {
        perror("Unable to get values from stdin via tcgetattr");
        exit(1);
    }
    settings.c_lflag   &= ~ICANON; /* No buffering */
    //settings.c_lflag   &= ~ECHO; /* Turn off echo. */
    //settings.c_cc[VMIN] = 1; /* get 1 char at a time */
    tcsetattr(STDIN_FD, TCSANOW, &settings);
}

/*
 * Converts I/O into non-blocking mode.
 * Turns on nodelay mode by using fcntl.
 * I'm uncertain if we need this, but I'm keeping the function here just in case.
 */
void set_nodelay_mode() {
    int termflags;
    termflags = fcntl(STDIN_FD, F_GETFL);
    termflags |= O_NDELAY;
    fcntl(STDIN_FD, F_SETFL, termflags);
}

/*
 * This is the function which runs when the user terminates the program.
 */
void end_snake(int signum) {
    endwin();       // Terminate curses window
    tty_mode(1);    // Restore terminal settings
    exit(1);        // End the program
}
