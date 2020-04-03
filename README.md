# quash
A simple yet powerful shell program

# Build Instructions
To build quash, simply type "make" into your terminal. If for some reason make fails, quash can also be built by
running "gcc -o quash main.c".

# Running quash with a file from bash
Once quash has been built, it is possible to run quash with a file of commands for quash to execute. This can be done by 
starting quash with the command "bash> ./quash "<" commands.txt". Note that the parentheses around the < character are 
required, as otherwise bash would think you are telling it to do file redirection, not run quash. 

# Running quash normally
Once quash has been built, quash can be ran with the command "bash> ./quash". The functionality of running commands using
a file can still be achieved by using "< commands.txt" from within quash itself.
