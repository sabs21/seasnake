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
void init_snake(int y, int x, char direction);
void move_snake();
void shorten_tail();
void auto_move();

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

/** statics, structs, and constants **/
/* window dimensions*/
static int window_row;
static int window_col;

/* game stats */
static int score = 0;
static int snake_len = 3;
static char key;
int mode = 1;                // 1 = true, 0 = false
int gameTime = 0;            // Tracks how many iterations of the while loop have been performed.
int ticks = 0;               // Keeps track of when checks are performed in the game. When ticks == 0, progress the game forward by 1 gameunit.
int lastTick = -1;           // Used by the inner loop to prevent extra inputs from getting registered when they shouldn't be.
int timeUnit = 128;          // A timeUnit consists of x amount of ticks. So in this case, 8 ticks == 1 timeUnit.


/* snake head location */
int head_y = 5;                     // TODO: change to random
int head_x = 10;                    // TODO: change to random

/* logic */
static char move_up = 'w';
static char move_down = 's';
static char move_left = 'a';
static char move_right = 'd';

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

/* head of snake */
struct node *head = NULL;
struct node *tail = NULL;

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

    /* init snake of size 3 */
    init_snake(head_y, head_x, key);

    /* Print initial pause message */
    move(window_row / 2, window_col / 2);
    addstr("Press any key to start playing!");
    move(window_row - 1, window_col - 1);

    /* Send the first image from the buffer to terminal */
    refresh();

    /* Keeps the user in a paused state until they hit a button ~ Nick Sabia */
    getchar();

    /* Enable settings after the user un-pauses */
    set_settings(); // Set terminal settings for the program.
    set_nodelay_mode(); // Setting this prevents getch() from blocking the program for input.

    /* Erase initial pause message */
    move(window_row / 2, window_col / 2);
    addstr("                                ");
    move(window_row - 1, window_col - 1);

    /* The draw loop */
    while (mode) {
        /* 
         * This inner loop prevents the snake from continuing quickly in a direction after a user stops pressing a directional key. 
         * Without this loop, a lot of extra inputs would get registered that would lead to the snake barreling off into one direction,
         * ignoring the user's attempt at slowing down or changing direction. ~ Nick Sabia
        */
        while(ticks != lastTick) {
            // Set the lastTick equal to the current tick to show that there was a check for input.
            lastTick = ticks;
            //noecho(); echo is already turned off when set_settings is called
            input = getch();
            // Handling of user input: Only specified inputs receive a reaction; Wrong input or no input goes to default case (no input) MM
            switch (input) {
                case (char) KEY_LEFT:
                case 'a':
                    // Check for reversal:
                    if (key == 'd' || key == (char)KEY_RIGHT){
                        game_condition(2);
                    }
                    // Draw the direction moved
                    move(0, DIRECTION_POS);
                    addstr("LEFT ");
                    key = 'a';
                    time_event();
                    break;

                case (char) KEY_DOWN:
                case 's':
                    // Check for reversal:
                    if (key == 'w' || key == (char)KEY_UP){
                        game_condition(2);
                    }
                    // Draw the direction moved
                    move(0, DIRECTION_POS);
                    addstr("DOWN ");
                    key = 's';
                    time_event();
                    //refresh();
                    break;

                case (char) KEY_UP:
                case 'w':
                    // Check for reversal:
                    if (key == 's' || key == (char)KEY_DOWN){
                        game_condition(2);
                    }
                    // Draw the direction moved
                    move(0, DIRECTION_POS);
                    addstr("UP   ");
                    key = 'w';
                    //refresh();
                    time_event();
                    break;

                case (char) KEY_RIGHT:
                case 'd':
                    // Check for reversal:
                    if (key == 'a' || key == (char)KEY_LEFT){
                        game_condition(2);
                    }
                    // Draw the direction moved
                    move(0, DIRECTION_POS);
                    addstr("RIGHT");
                    key = 'd';
                    //refresh();
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

            // Wait a half a second. This sleep does not block interrupts.
            nanosleep(&speed, &rem);
            ticks++;

            // send tokens for border from buffer to terminal
            refresh();

            if (ticks % timeUnit == 0) {
                // One time unit has passed. Increment time elapsed
                time_event(key);
                ticks = 0;
            }
        }
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
/*
 * 1) eat_fruit(), eg add node to head
 * Purpose: grows snake
 * Method: call eat_fruit() when head collides with fruit
 * Input: none
 * Returns: doubly linked list with new length of +1.
 *
 * A) create a new node
 * B) add new node to head of LL
 *
 * THE DATA STRUCTURE:
 *      TAIL <--> NODE <--> HEAD where
 *      a. TAIL contains last element, contains NULL only with <=2 nodes.
 *      b. NODE are middle snake segments,
 *      c. HEAD always points to front, new nodes always added to head.
 */
void eat_fruit(int y, int x){
    /* A) create a pointer of new node to add */
    struct node *new_head = (struct node*)malloc(sizeof(struct node));
    new_head->row = y;
    new_head->column = x;
    /* B) add new node to head of LL */
    if (head == NULL) {                     // list is empty, first item becomes head and tail remains
        head = new_head;
        head->prev = tail;
    }
    if (tail == NULL) {                     // list only has one item (the head), second becomes the tail
        tail = head;
        tail->next = new_head;
        head = new_head;
        head->prev = tail;
    }
    if (tail && head != NULL){
        head->next = new_head;
        new_head -> prev = head;
        head = new_head;
    }
}
/* 2) remove_tail(), remove node from tail
*  Purpose: initializes baby snake for game
*  Method: call before initializing game
*  Input: none
*  Returns: LL with new length 3.
*/
void shorten_tail(){
    // delete tail on screen
    move(tail->row, tail->column);
    addstr(" ");
    // save coordinates of new tail
    int node_y = tail->next->row;
    int node_x = tail->next->column;
    // set pointer to new tail
    tail = tail->next;
    // set coordinates to new tail
    tail->row = node_y;
    tail->column = node_x;
}

/* 3) starter_snake()
*  Purpose: initializes baby snake for game
*  Method: call before initializing game
*  Input: none
*  Returns: LL with new length 3.
*/
void init_snake(int y, int x, char direction){
    /* create starter snake */
    int node_y = y;
    int node_x = x;
    //eat_fruit(node_y, node_x);
    for(int i = 0; i < 3; i++) {
        if (direction == 'w') {
            node_y = node_y - 1;
        } else if (direction == 's') {
            node_y = node_y + 1;
        } else if (direction == 'a') {
            node_x = node_x - 1;
        } else if (direction == 'd') {
            node_x = node_x + 1;
        }
        eat_fruit(node_y,node_x);
        // eatfruit will handle pointers, handle tail coordinates here.
        tail->row = node_y;
        tail->column = node_x;
    }
    // reset head coordinates after scanning linked list.
    head->row = y;
    head->column = x;
}

/* 4) move_snake()
*  Purpose: takes in two coordinate arguments, creates a new head node
*  Method: adjusts pointers as appropriate for moving snake forward one direction
*       - ONLY handles pointers and printing tokens. NOTE that auto-move handles directionality and coordinates
 *      - illusion of movement created by lengthening head while simultaneously shortening tail.
 *      - NOTE: no net addition of nodes occurs.
*  Input: int y coordinate and int x coordinate
*  Returns: a snake whose tokens represent movement
*/
void move_snake(int y, int x){
    // add segment to head
    eat_fruit(y, x);
    // remove from tail
    shorten_tail();
    // save original location of head
    int save_y = head->row;
    int save_x = head->column;
    // scan through snake printing tokens
    struct node* scanner = head;
    while(scanner != tail) {
        move(scanner->row,scanner->column);
        addstr("o");
        scanner = scanner->prev;
    }
    // reset head of snake
    head->row = save_y;
    head->column = save_x;
    // move cursor back to head of snake
    move(head->row,head->column);
    addstr("O");
    refresh();
}
/* 4) auto_move()
*  Purpose: moves snake depending on value of key (current direction of snake).
*  Method: provides move with new y,x coordinates with which to move snake.
*       - ONLY handles coordinates of head node, dependent on key value. NOTE that auto-move pointers and printing
*  Input: none
*  Returns: a snake which appears to move depending on key value. meant to occur each second until win or lose condition
*/
void auto_move(){
    /* Handle user input */
    if (key == 'w') {
        // Draw the direction moved
        move_snake(head->row-1, head->column);
        move(head->row, head->column);
    }
    if (key == 'a') {
        // Draw the direction moved
        move_snake(head->row, head->column-1);
        move(head->row, head->column);
    }
    if (key == 's') {
        // Draw the direction moved
        move_snake(head->row+1, head->column);
        move(head->row, head->column);
    }
    if (key == 'd') {
        // Draw the direction moved
        move_snake(head->row, head->column+1);
        move(head->row, head->column);
    }

    /* check for border collisions */
    if (head->row == 1 || head->row == LINES-1 || head->column == 0 || head->column == COLS-2){
        game_condition(1);
    }

    /* check for running into itself */
    // save head coordinates before scan
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
        return 'w';
    } else if (random_integer == 1){
        return 's';
    } else if (random_integer == 2){
        return 'a';
    } else if (random_integer == 3){
        return 'd';
    } else {
        return 'x';
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
            /* direction reversal */
        case(2):
            move(window_row / 2, window_col / 2);
            addstr("YOU GOOFED!\tYou reversed direction.");
            refresh();
            sleep(2);
            raise(SIGINT);
            break;
            /* run into itself */
        case(3):
            move(window_row / 2, window_col / 2);
            addstr("YOU GOOFED!\tYou bit yourself.");
            refresh();
            sleep(2);
            raise(SIGINT);
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
    auto_move();
    refresh();
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
    settings.c_lflag   &= ~ECHO; /* Turn off echo. */
    settings.c_cc[VMIN] = 1; /* get 1 char at a time */
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
    //<<<<<<< revisions
}
