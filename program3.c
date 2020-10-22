#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

struct parameters
{
    char *parameter;
    struct parameters *next;
};

int processCommand(char *userCmd)
{
    printf("%s\n", userCmd);
    return 0;
}
/* 
* Parse the userCommand which is space delimited and create a parameters struct with the data in the line
* Adapted from <CS 344> (<10/12/20>) <main.c> [<sample program>]. https://repl.it/@cs344/studentsc#main.c.
* to create a semi efficient list to handle parameters and allow for checking for $$.

struct parameters processCommand(char *userCmd)
{
    struct parameters *currParameter = malloc(sizeof(struct parameters));

    // For use with strtok_r
    char *savePtr;
    int ampersandPresent = 1; 

    // The first token is the command 
    char *token = strtok_r(userCmd, " ", &saveptr);
    char command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(command, token);

    printf("%s\n",command);

    // START THE LOOP HERE. TERMINATE WHEN WE FIND THAT NEWLINE CHARACTER
    while (token)
    {
        token = strtok_r(NULL, ",", &saveptr);
        parameters->parameter = calloc(strlen(token) + 1, sizeof(char));
        strcpy(parameters->parameter, token);
    }
    return 0;
}*/

int main()
{
    
    // Reserve space for a command up to 2048 characters long with two extra for newline
    char userCommand[2050];
    char exitCommand[] = "exit\n";
    char expansion[] = "$$";

    do
    {

    // printf should be okay here since this isn't a signal handler.
    printf(": ");

    // Read user data
    fgets(userCommand, 2050, stdin);

    // Process command
    //Remove the newline char to allow fair strcmp.
    //userCommand[strcspn(userCommand, "\n")] = 0;


    // If the input is not a newline we process the command.
    if (strcmp(userCommand, "\n") != 0)
    {
        printf("%s\n", userCommand);
        // If the command is not a newline it is either a comment or command
        // We use strtok_r to get the first piece of the command terminated by a space or null
        char *token = strtok(userCommand, " ");
        char *command = calloc(strlen(token) + 1, sizeof(char));
        strcpy(command, token);
        
        // Check if we have a comment or command. 
        if (strcmp(command, "#") != 0)
        {
            printf("%s\n %s\n %s\n", token, command, userCommand);
            processCommand(userCommand);
        }

    }

    //execlp(userCommand);

    } while (strcmp(userCommand, exitCommand) != 0);

    // HERE IS WHERE WE'LL CALL/IMPLEMENT THE EXIT HANDLER.

    return 0;
}
