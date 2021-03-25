Seasnake is the final project for CCSU CS 355. 

RUBRIC:

Study chapters 5-7 of the textbook before starting any work on the project.
This project is designed to be completed over a period of 2-3 weeks by a team of four students.
The team members of each group should be emailed me by March 22 (only one email per team).
Each student must have very clear roles and responsibilities in the project.
At the very beginning of each function/code block, add a comment describing the primary author of that code and the main functionality declared/implemented.
This project must utilize the curses or ncurses library for all visuals.
Grading rubric (out of 20 pts):
Indentation: 2
Commenting: 3
Variable/function naming: 3
Snake movement/growth: 4
Trophies: 4
Overall gameplay: 4
Write a C program snake that implements the classic snake game.
The snake pit:
The snake pit is the area where the snake can move.
The snake pit must utilize all available space of the current terminal window.
There must be a visible border delineating the snake pit.
The snake:
The initial length of the snake is three characters.
Initial direction of the snake's movement is chosen randomly.
The user can press one of the four arrow keys to change the direction of the snake's movement.
The snake's speed is proportional to its length.
The trophies:
Trophies are represented by a digit randomly chosen from 1 to 9.
There's always exactly one trophy in the snake pit at any given moment.
When the snake eats the trophy, its length is increased by the corresponding number of characters.
A trophy expires after a random interval from 1 to 9 seconds.
A new trophy is shown at a random location on the screen after the previous one has either expired or is eaten by the snake.
The gameplay:
The snake dies and the game ends if:
It runs into the border; or
It runs into itself; or
The user attempts to reverse the snake's direction.
The user wins the game if the snake's length grows to the length equal to half the perimeter of the border.

