#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>


struct userComm
{
    char *command;
    char *arguments; // Effectively an array
    char *inputFile;
    char *outputFile;
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
    char buffer[128];
    char exitCommand[] = "exit";
    char expansion[] = "$$";
    char newLine[] = "\n";
    int i = 0;

    /* 
    * Adapted from <user2622016> (<09/28/15>) [<post response>]. https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
    * Used to change the pid int to a string so it can be concatenated if $$ is present.
    */
    int pId = getpid();
    int size = snprintf( NULL, 0, "%d", pId);
    char* processId = malloc( size + 1 );
    snprintf(processId, size + 1, "%d", pId); 

    do
    {
        // printf should be okay here since this isn't a signal handler.
        printf(": ");

        // Read user data
        fgets(userCommand, 2050, stdin);
        if (strcmp(userCommand, newLine) != 0)
        {
            userCommand[strcspn(userCommand, "\n")] = 0;
        
            // Process the command
            char *ptr;
            char *secondPtr;
            char *token = strtok_r(userCommand, " ", &ptr); 
            char *expandToken = strtok_r(token, "$$", &secondPtr); // Doesn't work for a double character. 

            // If the tokens are the same we didn't find a $$
            if (strcmp(token, expandToken) != 0)
            {
                strcpy(buffer, expandToken);
                strcat(buffer, processId);
                printf("%s\n", buffer);
            }
            
            arguments[i] = token;

            if (strcmp(token, "#") != 0)
            {
                do
                {   
                    i++;
                    token = strtok_r(NULL, " ", &ptr);
                     
                    arguments[i] = token;
                } while (token != NULL && i < 513);
                /*
                for (int j=0; j < i; j++)
                {
                    printf("%d %s\n", j, arguments[j]);
                }
                */

                

            }
        }
        // reset i after each iteration
        i = 0; 
    } while (strcmp(arguments[0], exitCommand) != 0);

    // HERE IS WHERE WE'LL CALL/IMPLEMENT THE EXIT HANDLER.
    free(processId);
    return 0;
}
