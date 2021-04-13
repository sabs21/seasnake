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
#include <time.h>
#include <ncurses.h>

#define INFO_ROW 1

/** prototypes **/
/* RE: snake pit */
void pit_size();
void init_pit_border();
void update();

/* RE: snake */
void starter_snake();
void eat_fruit();
void print_snake();

/* RE: logic */
char choose_random_direction();

/** statics, structs, and constants **/
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
    char token;
    char direction;                 // valid direction: w, a, s, d
    struct node *prev;
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
    /* Initialize global random number generator */
    time_t t;
    srand((unsigned) time(NULL));

    /* set window grid */
    pit_size();                                                             // get/set dimensions of window
    char grid[window_row][window_col];                                      // initialize game dimensions

    /* initialize snake */
    printf("starter snake: ");                                              // DEBUG -
    starter_snake();                                                        // make a 3 segment baby snake
    print_snake();                                                          // DEBUG - SHOWS SNAKE DATA STRUCTURE WORKS
    printf("\n");                                                           // DEBUG -

    /* initialize snake pit */
    init_pit_border(grid);                                                  // initilize pit
    update(grid, head);                                                     // will prolly take in a trophy arg too at some point

    /* logic */

}

/*
 *  MAIN TODO:
 *  1.
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
        //printf("%d rows x %d cols\n", wbuf.ws_row, wbuf.ws_col);
        /* assign global variable values here: */
        window_row = wbuf.ws_row;                                              // assign global row
        window_col = wbuf.ws_col;                                              // assign global column
    }
}
/*
 *  3. update()
 *  Purpose: print game grid including border, snake, trophies
 *  Method: call update() after user input
 *  Input: game board, snake, (trophies when coded)
 *  Returns: updated game grid
 */
void update(char array[window_row-1][window_col-1], struct node snake){
    printf("Welcome to Snake\tScore: %d\tUser input here: \n", score);    // name of game, score, space for user inputs
    /* scanner object to scan snake and print to screen */
    struct node* scanner = head;
    /* draw game tokens */
    for(int i = 0; i < window_row; i++){
        for(int j = 0; j < window_col; j++){
                printf("%c", array[i][j]);
        }
    }
}

/*
 *  PIT TODO:
 *  1. update will need parameters for snake and trophies DONE-ISH
 *  2. now will need to code trophies and pass snake and trophies in, updating the board somehow
 */
/***********************************************************************************************************************
*  SNAKE
*  Use structure to represent node
*  Nodes used to represent a linked list
*  Nodes are only added to the head and nodes are never removed.
*  1) grow snake
 * 2) create baby snake
*  2) debug snake
***********************************************************************************************************************/
/*
 * 1) eat_fruit()
 * Purpose: grows snake
 * Method: call eat_fruit() when head collides with fruit
 * Input: none
 * Returns: LL with new length of +1.
 *
 * A) create a new node
 * B) add new node to head of LL
 *
 * THE DATA STRUCTURE:
 *      TAIL <-- NODE <-- HEAD where
 *      a. TAIL always == to NULL indicating end of snake,
 *      b. NODE are middle snake segments,
 *      c. HEAD always points to front, new nodes always added to head.
 *      d. only can scan from one direction, from head to tail.
 */
void eat_fruit(int col, int row){
    /* A) create a pointer of new node to add */
    struct node *new_head = (struct node*)malloc(sizeof(struct node));
    new_head->token = 'O';
    new_head->column = col;
    new_head->row = row;

    /* B) add new node to head of LL */
    if (head == NULL) {
        head = new_head;
        head -> prev = tail;
        return;
    } else {
        new_head -> prev = head;
        head = new_head;
    }
}

/* 2) starter_snake()
*  Purpose: initializes baby snake for game
*  Method: call before initializing game
*  Input: none
*  Returns: LL with new length 3.
*/
void starter_snake() {
    int x = 10;
    int y = 10;
    for (int i = 0; i < 3; i++){
        eat_fruit(x, y);
        x--;
        y--;
    }
}
/* 3) print_snake()
*  Purpose: test and debug snake data structure
*  Method: call for debugging
*  Input: none
*  Returns: a printed snake
*/
void print_snake() {
    struct node* scanner = head;
    while(scanner != NULL) {
        printf("%c", scanner->token);
        scanner = scanner->prev;
    }
}
/***********************************************************************************************************************
*  LOGIC
*  1) random function for start direction
*  2) check for collision with wall or fruit with each movement
 * 3) another random function for randomly placing trophies
***********************************************************************************************************************/
/*
 *  1. choose_random_direction()
 *  Purpose: return a random direction to start snake in
 *  Method: call update() after user input
 *  Returns: updated game grid
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
 *  LOGIC TODO:
 *  1. create a random function, returns char for direction - DONE
 *  2.
 */