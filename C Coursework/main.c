// A simple implementation of the Lee algorithm

#include <math.h>
#include <stdio.h>
#include "graphics.h"
#include <stdlib.h>

#define COL 25
#define ROW 20 /* number of rows and columns - not 0 indexed. Needs to be edited right here if you want to change grid size.
                    Rectangular grids are also allowed*/

#define PROGRAMSPEED 8 //defines how long the sleep() should be inbetween drawings. Lower programspeed = faster animations

typedef struct robot
{ 
    int grid_x, grid_y, orientation;
} robot;

// global variables
int START_X;
int START_Y;
int GOAL_X;
int GOAL_Y; // x,y position of start and goal - 0 indexed. Initialized to -1 to represent that they haven't been read from the file yet.
robot Robot;
int grid[ROW][COL] = {0}; /* all values are initialized to 0. will store positions of start, goal and obstacles. Will also 
                            store distance of each sqaure from the start based on BFS search depth - Used for Lee algorithm */

int SPACING; // side length of grid square. used in almost all drawing functions

void parse_file(void);

void find_spacing(void);

int atMarker(int x, int y);

int canMoveForward(int x, int y);

int search_path(int found_goal);

void initialize_robot(void);

void draw_grid(void);

void draw_robot(void);

void initialize_display(void);

void move_robot(int destination_x, int destination_y);

void backtrack(void);

int main(void)
{
    parse_file();
    find_spacing();
    initialize_robot();
    initialize_display();

    int temp = 0;
    int found_goal = search_path(temp);

    if (!(found_goal))
    { // no possible path exists
        foreground();
        setRGBColour(219, 121, 15);
        drawString("No possible path", 50, 50);
        return 1;
    }
    backtrack();
    return 0;
}

void parse_file(void)
{ // read position of robot, goal and obstacles and check if they are valid. If they are, assign them to the appropriate variables.
    FILE *f = fopen("parameters.txt", "r");                                

    START_X = -1;
    START_Y = -1;
    GOAL_X = -1;
    GOAL_Y = -1; /* everything set to -1. if sscanf cant convert some characters in parameter file to 
                    integers,some of the variables will keep these values. We can use this to check if the 
                    all characters in the parameters file are integers */
    
    int obstacles_buffer_x;
    int obstacles_buffer_y; // stores the obstacles position. 
    
    char line[20]; /* magic number of 20 chosen. each line stores 2 numbers so unless 
                    each number is around 10 digits long (which would be impossible to display
                    because the grid sqaures would have to be impossibly small), it should be fine */

    fgets(line, 19, f);
    sscanf(line, "%d,%d", &START_X, &START_Y); 
    fgets(line, 19, f);
    sscanf(line, "%d,%d", &GOAL_X, &GOAL_Y); 

    if ((COL < 0) || (ROW < 0) || (START_X < 0) || (START_X > COL - 1) || (START_Y < 0) || (START_Y > ROW - 1)
         || (GOAL_X < 0) || (GOAL_X > COL - 1) || (GOAL_Y < 0) || (GOAL_Y > ROW - 1)) 
         /* invalid - characters in file aren't convertable to int or values are out of the bounds of the grid */
    {
        printf("Invalid Start or Goal position. Check read.md for layout of file\n");
        exit(0);
    }
    
    grid[GOAL_Y][GOAL_X] = ROW * COL; /* represents goal. very high number chosen because it should be the highest valued square
                                         in the grid for Lee algorithm to work. While we do assign other values in the grid based
                                         on the BFS search depth, they wil never exceed ROW*COL */

    grid[START_Y][START_X] = 1; // represents start

        while (1) // scan remaining lines to get obstacle positions.
        {
            if (fgets(line, 19, f) == NULL){ // If we reach the end of the file, break out of the while loop
                break;
            }
            sscanf(line, "%d,%d", &obstacles_buffer_x, &obstacles_buffer_y);

            if ((obstacles_buffer_x > COL - 1) || (obstacles_buffer_y > ROW - 1) || 
                ((obstacles_buffer_x == START_X) && (obstacles_buffer_y == START_Y)) 
                || ((obstacles_buffer_x == GOAL_X) && (obstacles_buffer_y == GOAL_Y)))
            /* invalid - values are out of the bounds of the grid, or overlap with goal or robot */
            {
                printf("Invalid obstacle positions. Check read.md for layout of file\n");
                exit(0);
            }
            else
            {
                grid[obstacles_buffer_y][obstacles_buffer_x] = -1; //represents obstacles. arbitrary value of -1 chosen
            }
        }
    fclose(f);
}

void find_spacing(void)
{
    if (COL > ROW)
    {
        SPACING = (int)(600 / COL); //tries to fit all grid squares within 600px.
    }
    else
    {
        SPACING = (int)(600 / ROW);
    }
}

int atMarker(int x, int y)
{
    if (x == GOAL_X && y == GOAL_Y)
    {           
        return 1;
    }
    else
    {
        return 0; 
    }
}

int canMoveForward(int x, int y)
{
    if (x > (COL - 1) || x < 0 || y > (ROW - 1) || y < 0 || grid[y][x] >= 1 || grid[y][x] == -1)
    { // check if square is invalid (out of bounds, already visited, an obstacle)
        return 0;
    }
    else
    {
        return 1; 
    }
}

int search_path(int found_goal)
{ // returns 1 if a path to goal is found

    int total_squares_visited = 1; /* keeps track of how many valid grid squares in total have been visited. 
                                      visited means the square has been traversed by the algorithm and 
                                      is a valid square (not an obstacle, not already visited, not out of bounds).
                                      This is incremented every time we encounter a valid neighbouring square. Indicates which
                                      queue position the next neighbour we encounter should be placed at */

    int queue[(ROW * COL) + 5][2] = {0}; /* stores squares to be processed in order of when they were encountered. processing
                                            means we check all the neighbours of a square and add valid ones to the bottom of the
                                            queue. queue[a][0] = y position of grid square, queue[a][1] = x position of grid square 
                                            at arbitrary queue position a */

    int nodes_processed = 0; /* represents the number of grid squares for which all the valid neighbouring grid squares
                                      have been visited. This is incremented every time we check all valid neighbouring squares
                                      for a particular square. Also acts as a queue position index, since once we finish 
                                      checking all the neighbours of a square, we want to move 1 place down the queue and repeat 
                                      the process  */

    int BFS_level = 1; /* represents the "depth" of the BFS search. It is incremented to 2 once you process all the neighbours of
                         the starting square. It is then incremented to 3 once you process all neighbours of all neighbours 
                         of the starting square and so on. If a neighbouring square at position x,y is found and checked to be 
                         valid, the [x][y] position in the global grid array is assigned this value. Can be thought of as building
                         a map of distances from the starting square. Also helps indicate whether a valid square has been visited. 
                         This is because, since the global grid array was initialized with all 0s, if this value is >=1 at 
                         grid[x][y] and the square is valid, it means the grid square at position x,y has been visited */

    queue[0][0] = START_Y;
    queue[0][1] = START_X; 
    int moves_x[] = {1, 0, -1, 0};
    int moves_y[] = {0, -1, 0, 1}; /* each pair of values (moves_x[i], moves_y[i]) represent the 4 ways the x and y values 
                                      of a grid square can be changed to reach its neighbours */
    int current_node_y = queue[0][0];
    int current_node_x = queue[0][1]; 

    do
    {
        sleep(2*PROGRAMSPEED);

        int nodes_not_processed = total_squares_visited - nodes_processed; /* tells us how many squares we've visited but not 
                                                                            visisted the neighbours of. Represents how many nodes
                                                                            are to be processed at each BFS level. Starts off at
                                                                            1, indicating we have to process 1 node 
                                                                            (the starting node) */

        for (int a = 0; a < nodes_not_processed; a++)
        { // repeat the neighbour searching process for however many unprocessed nodes we have

            current_node_y = queue[nodes_processed][0];
            current_node_x = queue[nodes_processed][1]; /* nodes_processed is incremented every time the bottom for loop 
                                                           (which checks all neighbours of a given node) is completed. 
                                                           Once this happens, we know we're done with the current node and have 
                                                           to move down the queue */

            // for loop to check immediate neighbours of a node at queue position nodes_processed
            for (int i = 0; i < 4; i++)
            {

                int new_y = moves_y[i] + current_node_y;
                int new_x = moves_x[i] + current_node_x; // iterates through the 4 possible x,y movements to reach a neighbour

                if (atMarker(new_x, new_y))
                {
                    return 1; // some path to goal found
                }

                if (!(canMoveForward(new_x, new_y)))
                {
                    continue;
                }

                else
                {
                    queue[total_squares_visited][0] = new_y;
                    queue[total_squares_visited][1] = new_x; // adds this valid neighbour below all other valid neighbours in the queue
                    total_squares_visited++;                 // increment this to signify one more valid square has been visited

                    grid[new_y][new_x] = BFS_level; // build a map of distance from the starting square
                    background();
                    setRGBColour(0, 150, 255);
                    fillRect((new_x * SPACING), (new_y * SPACING), SPACING, SPACING); // visualizes search path
                }
            }

            nodes_processed++; /* increment this to signify all valid neighbours of the current node have been visited and it is
                                time to move down the queue */
        }

        BFS_level++; // once we process all unprocessed nodes, move up a level. Represents increasing distance from the start.

    } while (!((queue[nodes_processed][0] == 0) && (queue[nodes_processed][1] == 0)));
    /* queue array was initialized with all x,y pairs to be 0s. for all neighbours, if absolutely no valid neighbours are found, 
       no new x,y pairs will be added to the queue, and any increment down the queue will return 0s. thus, getting x and y values
       of 0 upon an increment of the queue indicates there are no more potential neighbours and the search should end. 
       used a do-while loop to guarantee first iteration. avoids problem of while loop immediately evaluating to false and 
       skipping the entire search when start point is 0,0 */

    return 0; //no path to goal found
}

void initialize_robot(void)
{
    Robot.grid_x = START_X;
    Robot.grid_y = START_Y;
    Robot.orientation = 0; // Increment of orientation represents turning robot by 90 degrees. 0->left, 1->up, 2->right, 3->down
}

void draw_grid(void)
{
    background();
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            setColour(black);
            if ((i + 1) <= ROW)
            {
                drawLine(j * SPACING, i * SPACING, (j + 1) * SPACING, i * SPACING);
            }
            if ((j + 1) <= COL)
            {
                drawLine(j * SPACING, i * SPACING, j * SPACING, (i + 1) * SPACING);
            }

            //above functions are for drawing grid lines

            if (grid[i][j] == ROW * COL)
            { //represents goal

                setColour(gray);
                fillRect(j * SPACING, i * SPACING, SPACING, SPACING);
            }
            if (grid[i][j] == -1)
            { // represents obstacle.
                setColour(red);
                fillRect(j * SPACING, i * SPACING, SPACING, SPACING);
            }
            if (grid[i][j] == 1)
            { // represents start
                setRGBColour(170, 250, 150);
                fillRect(j * SPACING, i * SPACING, SPACING, SPACING);
            }
        }
    }
}

void draw_robot(void)
{ // draws a green triangle
    foreground();
    setColour(green);
    int x_offset = (Robot.grid_x * SPACING) + (SPACING / 2); //the x and y pixel positions of the centre of specific grid square
    int y_offset = (Robot.grid_y * SPACING) + (SPACING / 2);
    int x_arr[3];
    int y_arr[3];
    double angle_in_radians = (double)(Robot.orientation * (M_PI / 2)); /* every increment of orientation turns robot by 
                                                                            90 degrees. 0->left, 1->up, 2->right, 3->down */

    for (int i = -1; i < 2; i++)
    {
        double scale_factor = (SPACING / 2) * ((((2 / sqrt(2)) - 1) * (abs(i))) + 1); /* half of the grid length multiplied by a 
                                                                                        scaling factor. Compensates for diagonal 
                                                                                        of square being longer than side */

        x_arr[i + 1] = (int)(x_offset + (scale_factor * cos(angle_in_radians + (0.75 * M_PI * i)))); /* cycles between cosine 
                                                                of (Robot angle-135) degrees, (Robot angle) degrees,
                                                                and (Robot angle+135) degrees to get x components of all 
                                                                the rotations. Multiplied by scaling factor to compensate for 
                                                                vertices on corner vs middle of grid sqare and then offset based
                                                                on the grid */

        y_arr[i + 1] = (int)(y_offset - (scale_factor * sin(angle_in_radians + (0.75 * M_PI * i)))); /* similar to above but 
                                                                                                          for y component */
    }
    fillPolygon(3, x_arr, y_arr);
}

void initialize_display(void)
{
    setWindowSize(SPACING * COL, SPACING * ROW);
    draw_grid();
    draw_robot();
}

void move_robot(int destination_x, int destination_y)
{
    foreground();
    int direction = (destination_x - Robot.grid_x) - 2 * (destination_y - Robot.grid_y); /* a way of telling apart the movement. 
                                                            If robot has to move left-> direction = 1, right->direction = -1,
                                                            up->direction = 2, down->direction = -2 */

    if (direction == 1)
    {                          // moving right
        Robot.orientation = 0; //face right (read comment in initialize_robot to understand orientation values)
    }
    if (direction == -1)
    {                          // moving left
        Robot.orientation = 2; //face left
    }
    if (direction == 2)
    {                          // moving up
        Robot.orientation = 1; //face up
    }
    if (direction == -2)
    {                          // moving down
        Robot.orientation = 3; //face down
    }

    // turning the robot
    clear();
    draw_robot();
    sleep(PROGRAMSPEED);
    clear();

    // change global attributes of the robot
    Robot.grid_x = destination_x;
    Robot.grid_y = destination_y;

    // coloring destination square
    background();
    setRGBColour(170, 250, 150);
    fillRect(Robot.grid_x * SPACING, Robot.grid_y * SPACING, SPACING, SPACING);
    foreground();

    // drawing robot on destination square
    draw_robot();
    sleep(PROGRAMSPEED);
}

void backtrack(void)
{
    int smallest_distance = grid[GOAL_Y][GOAL_X]; /* our search_path function created a map of distances from the start based
                                                     on BFS search depth. We just follow these distances down from the goal until
                                                     we converge to the start. This is based on the Lee algorithm */
    int moves_x[] = {1, 0, -1, 0};
    int moves_y[] = {0, -1, 0, 1}; // all possible moves to reach neighbours
    int current_x = GOAL_X;
    int current_y = GOAL_Y;
    int path[ROW * COL][2] = {0}; // all values are initialized to 0
    path[(ROW * COL) - 1][0] = GOAL_Y;
    path[(ROW * COL) - 1][1] = GOAL_X;
    int path_position = (ROW * COL) - 2;

    //trace back path from goal to robot and add directions backwards to path array

    while (!(current_y == START_Y && current_x == START_X))
    { // search through immediate neighbours until one with a lower distance is found. Then repeat process at neighbour until we reach the start.
        for (int i = 0; i < 4; i++)
        {
            int new_y = moves_y[i] + current_y;
            int new_x = moves_x[i] + current_x;

            if (new_y == START_Y && new_x == START_X)
            {
                current_x = new_x;
                current_y = new_y;
                break;
            }

            if (grid[new_y][new_x] == -1 || new_x > (COL - 1) || new_x < 0 || new_y > (ROW - 1) || new_y < 0 || grid[new_y][new_x] == 0)
            { /* invalid - an obstacle, out of bounds or a square that wasn't traversed by the search_path function
                (which means the goal was visited before the square and so the square shouldn't be part of the optimal path.) */
                continue;
            }

            if (grid[new_y][new_x] < smallest_distance)
            { //if a square with a smaller distance to the start is found, set it as the new square to start the backtracking from
                smallest_distance = grid[new_y][new_x];
                current_x = new_x;
                current_y = new_y;
                break;
            }
        }
        path[path_position][0] = current_y;
        path[path_position][1] = current_x;
        path_position--; /* keep adding direction backwards from goal to start. Thus, once the backtracking is completed, 
                            the path will describe a path from the start to the goal */
    }

    for (int i = 0; i < ROW * COL; i++)
    { // for loop to make robot follow the path until it gets to the goal
        if (((Robot.grid_x == 0 && Robot.grid_y == 1)||(Robot.grid_x == 1 && Robot.grid_y==0)) && (GOAL_Y==0 && GOAL_X==0))
        {
            move_robot(0,0);
        } /* a bit hacky, but if the goal is 0,0, the bottom if statement would cause the robot to stop one step short of the goal. 
            The only possible steps to reach 0,0 are from 0,1 or 1,0 and so if the robot manages to get there, we just move it
            to 0,0. */
        
        if ((path[i][0] == 0 && path[i][1] == 0))
        { 
            continue;
        } /* if the x,y positions in the path for the robot are 0s it means they weren't added to the path and we should skip
             them. If statement before this deals with the edge case where the 0,0 in the path could represent that the goal
             is at 0,0*/
       
        move_robot(path[i][1], path[i][0]); // move robot to x,y position specified by the path array
    }
}