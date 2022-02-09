This program can find the shortest path through a maze, and can also figure out if no possible path to the goal exists. 
It's based on the Lee algorithm which uses BFS to create a ring of increasing "distances" from the starting square until 
the goal is reached. Then, the program backtracks in a very simple manner, starting from the goal and going to any square 
with a lower "distance" value than the current square, and so on until the starting sqaure is reached. This is guaranteed 
to be the shortest possible path between the start and goal squares. The robot is then told to follow this backtracking 
path in reverse, leading it from the start to the goal in the most optimal manner. The positions of the goal, robot 
and obstacles can be entered in the parameter.txt file in the following manner.

How to organize parameter.txt file:

Line 1: Starting X position, Starting Y position
Line 2: Goal X position, Goal Y position 
Line 3: Obstacle X position, Obstacle Y position (one pair per line)
Line 4: Obstacle 2 X position, Obstacle 2 Y position
...
...

Example file:
3, 0
13, 19
1, 1
2, 5
3, 9

Example of an invalid file layout: (Read notes to understand why)

3, -1
13, 19


1 1
2, 5

15 17

Notes:
1. All X, Y values will be read in a 0-indexed manner. So the top left square is 0,0 not 1,1.
2. Start X and Y, and Goal X and Y should be in bounds (not outside the grid defined by the ROW and COL in the main program)
3. Obstacles should not overlap with the robot or goal. 
4. Any empty lines before or after pairs of values might cause some errors.
5. Seperate values by a comma
6. If you want to change row and column size of grid, you have to do so in the main.c program.

If these aren't followed the program will print an error or not work as expected