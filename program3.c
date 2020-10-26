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

char *replacePId(char **args, int i, char *pId)
{
    int j = 0;
    int length = strlen(args[j]) + 1;
    int pIdLength = strlen(pId);
    char myVar[length];
    strcpy(myVar, args[j]);
    // We make final var big enough to hold both the pId and the value attached if need be. 
    char finalVar[length + pIdLength];

    /*
    * Adapted from <StackOverFlow> (<12/18/10>) <user257111>[<answer to a question>]. https://stackoverflow.com/questions/4475948/get-a-character-referenced-by-index-in-a-c-string
    * This was used to get individual characters so that I can check for "$$".
    */
    char* s;
    int index = 0;
    char myLetter;

    for ( s=&myVar[0]; *s != '\0'; s++ )
    {   
        // Finds the occurances of $$
        if (strncmp(s, "$$", 2) == 0)
        {
            printf("%c\n", myVar[index]);
            // increment s so we aren't looking at the same value
            s++;
        }

    }
    return replacedArguments 
}


struct userComm *makeStruct(char **args, int i)
{
    int j = 0;
    char *parsedArguments[512];
    struct userComm *commandStruct = malloc(sizeof(struct userComm)); 

    // Allocate space for the command and then assign it.
    commandStruct->command = calloc(strlen(args[j]) + 1, sizeof(char));
    commandStruct->command = args[j];
    j++;

    while (j != i)
    {
        if (strcmp(args[j], "<") == 0)
        {
            // Since we've hit the indicator for input file we know the next arg is the input file
            j++; // hence, we increment j before assignment.
            commandStruct->inputFile = calloc(strlen(args[j]) + 1, sizeof(char));
            commandStruct->inputFile = args[j];
            // Increment again so we don't get a duplicate
            j++; 
        }

        else if (strcmp(args[j], ">") == 0)
        {
            // Same as the input indicator, we know we want to pay attention to the next arg
            j++; // hence, we increment j before assignment.
            commandStruct->outputFile = calloc(strlen(args[j]) + 1, sizeof(char));
            commandStruct->outputFile = args[j];
            // Increment again so we don't get a duplicate
            j++;
        }

        // Since we've already taken the command out, and we know it's not an input or output file
        // We know it's an argument for the command and assign it as such. 
        else
        {
            parsedArguments[j] = args[j];
            j++;
        }

    }

    return commandStruct;
}



void printArgs(char **args, int i)
{
    for (int j=0; j < i; j++)
    {
        printf("%s\n", args[j]);
    }
}

void printCommand(struct userComm* userCommand)
{
    printf("%s, %d, %s, %s\n", userCommand->command,
    strlen(userCommand->arguments),
    userCommand->inputFile,
    userCommand->outputFile);
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
            
            arguments[i] = token;

            // If the token is not a comment
            if (strcmp(token, "#") != 0)
            {
                // Create a pointer array to store the arguments
                do
                {   
                    i++;
                    token = strtok_r(NULL, " ", &ptr);
                     
                    arguments[i] = token;
                } while (token != NULL && i < 513);

                // Now that we have read in and parsed the whole string we create a struct
                char *processedArg = replacePId(arguments, i, processId);
                struct userComm *commandStruct = makeStruct(arguments, i);
                //printArgs(arguments, i);                

            }
        }
        // reset i after each iteration
        i = 0; 
    } while (strcmp(arguments[0], exitCommand) != 0);

    // HERE IS WHERE WE'LL CALL/IMPLEMENT THE EXIT HANDLER.
    free(processId);
    return 0;
}
