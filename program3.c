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

void printArgs(char *args, int i)
{
    for (int j=0; j < i; j++)
    {
        printf("%s\n", args[j]);
    }
}


int main()
{
    
    // Reserve space for a command up to 2048 characters long with two extra for newline
    char userCommand[2050];
    char *arguments[513];
    char exitCommand[] = "exit";
    char expansion[] = "$$";
    int i = 0;

    do
    {

    // printf should be okay here since this isn't a signal handler.
    printf(": ");

    // Read user data
    fgets(userCommand, 2050, stdin);
    userCommand[strcspn(userCommand, "\n")] = 0;

    // Process the command
    char *ptr;
    char *token = strtok_r(userCommand, " ", &ptr); 
    arguments[i] = token;

    if (strcmp(token, "#") != 0)
    {
        do
        {   
            i++;
            token = strtok_r(NULL, " ", &ptr); 
            arguments[i] = token;

        } while (token != NULL && i < 513);
    }

    for (int j=0; j < i; j++)
    {
        printf("%s\n", arguments[j]);
    }

    } while (strcmp(arguments[0], exitCommand) != 0);

    // HERE IS WHERE WE'LL CALL/IMPLEMENT THE EXIT HANDLER.

    return 0;
}
