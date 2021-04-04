/***********************************************************************************************************************
*                                                                                                                      *
*  - SNAKE GAME in C -                                                                                                 *
*  by Abigail Fenerty, Nick Sabia, Justin Merville, Jacob Pelletier                                                    *
*  CS 355 Systems Programming final project, Spring 2021.                                                              *
*                                                                                                                      *
*  To run: 1) compile 2) execute compiled code 3) follow prompts                                                       *
*                                                                                                                      *
*  ** TODO: type up rules here **                                                                                      *
***********************************************************************************************************************/

/** headers **/
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

/** prototypes **/
/* RE: snake pit */
void pit_size();
void init_pit_border();
void update();

/* RE: snake */

/* RE: logic */
char choose_random_direction();


/** statics and constants **/
/* window dimensions*/
static int window_row;
static int window_col;

/* game stats */
static int score = 0;
static int snake_len = 3;

/* logic */
static char move_up = 'w';
static char move_down = 's';
static char move_left = 'a';
static char move_right = 'd';

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
    /* set window grid */
    pit_size();                                                             // get/set dimensions of window
    char grid[window_row][window_col];

    /* initialize snake pit */
    init_pit_border(grid);                                                            // initilize pit
    update(grid);

    /* logic */

}

/*
 *  MAIN TODO:
 *
 */

/***********************************************************************************************************************
*  SNAKE PIT                                                                                                           *
*  1. draw_pit_border                                                                                                  *
*  2. pit_size                                                                                                         *
*  3. update                                                                                                           *
***********************************************************************************************************************/
/*
 *  1. draw_pit_border()
 *  Purpose: initialize snake pit
 *  Method: matrix of array, populated by nested for loops
 *  Returns: grid matrix printed to terminal
 */
void init_pit_border(char array[window_row-1][window_col-1]){
    /* draw window border */
    for(int i = 0; i < window_row; i++){
        for(int j = 0; j < window_col; j++){
            if (i == 0 || i == window_row-1)
                array[i][j] = 'X';
            else if(j == 0 || j == window_col-1)
                array[i][j] = 'X';
            else
                array[i][j] = ' ';
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
        printf("%d rows x %d cols\n", wbuf.ws_row, wbuf.ws_col);
        /* assign global variable values here: */
        window_row = wbuf.ws_row;                                              // assign global row
        window_col = wbuf.ws_col;                                              // assign global column
    }
}
/*
 *  3. update()
 *  Purpose: print game grid including border, snake, trophies
 *  Method: call update() after user input
 *  Returns: updated game grid
 */
void update(char array[window_row-1][window_col-1]){
    /* draw window border */
    printf("Welcome to Snake\tScore: %d\tUser input here: \n", score);    // name of game, score, space for user inputs
    for(int i = 0; i < window_row; i++){
        for(int j = 0; j < window_col; j++){
            printf("%c", array[i][j]);
        }
    }
}

/*
 *  PIT TODO:
 *  1. update will need parameters for snake and trophies
 */
/***********************************************************************************************************************
*  LOGIC                                                                                                               *
***********************************************************************************************************************/
/*
 *  1. choose_random_direction()
 *  Purpose: return a random direction to start snake in
 *  Method: call update() after user input
 *  Returns: updated game grid
 */
char choose_random_direction(int min, int max){
    int random_integer = 0;                                         // DETERMINE RANDOM
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
 *  LOGIC TODO:
 *  1. create a random function, returns char for direction
 */