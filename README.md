# Smallsh.c

Click run to run Smallsh or use "./smallsh"

## Overview ##
smallsh.c is a compileable C program which masks and mimicks a bash terminal. 
The "exit", "cd", and "status" commands were custom implemented. All other commands are run using fork() and execvp() to spawn child processes. It supports foreground and background processes and uses signal handlers to control behavior. The command prompt is displayed using ":" and the program will only exit when "exit" is entered. 

*To  compile use gcc -std=gnu99 -o smallsh program3.c*

## Commands ##

- Lines begining with # are comments and will be ignored (Ex. "# echo 74" will not execute).

- $$ will be expanded into the process ID if it is included in a command (Ex. "echo $$" will display the PID).

- To use a background process enter & at the end of a command. (Ex. "sleep 10 &").

- Signal handlers catch Ctrl-Z and Ctrl-C. 
  - Ctrl-Z will cause the program to enter/exit foreground only mode.
  - Ctrl-C is simply ignored.

- All other commands such as ls, ps, sleep, etc. can be run as normal.
