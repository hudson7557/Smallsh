#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


struct userComm
{
    char *command;
    char *arguments[512];
    int numberOfArgs;
    char *inputFile;
    char *outputFile;
    char *background;
};

void replacePId(char **args, int i, char *pId)
{
    // For loop to loop through the args
    for (int x=0; x < i; x++)
    {
        int argLength = strlen(args[x]);
        int pIdLength = strlen(pId);
        int totalLength = argLength + pIdLength;

        // Create variables to hold our strings. 
        char *array = calloc( totalLength + 2, sizeof(char));
        char myVar[argLength];

        // we copy over the argument to allow us to access it through subscript
        strcpy(myVar, args[x]);

        /*
        * Adapted from <StackOverFlow> (<12/18x/10>) <user257111>[<answer to a question>]. https://stackoverflow.com/questions/4475948/get-a-character-referenced-by-index-in-a-c-string
        * This was used to get individual characters so that I can check for "$$".
        */

        char* s;
        // We use two seperate indexes to account for the possible difference when expansion occurs.
        int q = 0;
        int r = 0;

        // Loop through the string, the first character is removed on each iteration
        // and the first two characters are evaluated for whether they are $$.
        for ( s=&myVar[0]; *s != '\0'; s++ )
        {   
            if (strncmp(s, "$$", 2) == 0)
            {
                // If the first two characters of the substring are $$ we concatenated the pId into our array
                strcat(array, pId);
                s++; // Increment s again so we don't look at the second $

                // q is our index for the array, which we increment as much as needed for the pId
                q = q + pIdLength;

                // r is our index for the "string", we increment it twice to skip copying the $$
                r = r + 2;
            }

            else
            {
                // if the $$ character is not found we assign an index and increment our pointers. 
                array[q] = myVar[r];
                q++;
                r++;
            }

        }
        // We conclude by reassigning the variable.
        args[x] = array;

    }
}

struct userComm *makeStruct(char **args, int i)
{
    int j = 0;
    int x = 0;
    struct userComm *commandStruct = malloc(sizeof(struct userComm)); 

    // Allocate space for the command and then assign it.
    commandStruct->command = calloc(strlen(args[j]) + 1, sizeof(char));
    strcpy(commandStruct->command, args[j]);

    // The original idea behind this struct was to seperate out the command and it's arguments,
    // However this doesn't work with execvp because it needs the command in the array with the args
    // so it was added in here. This is what happens when you build the plain as you fly it and 
    // when you get advice from several TAs. 
    commandStruct->arguments[x] = calloc(strlen(args[j]) + 1, sizeof(char));
    strcpy(commandStruct->arguments[x], args[j]);

    // Initialize input and output file to contain an empty string. 
    // This allows us to check if they got assigned and handle it accordingly.
    commandStruct->outputFile = calloc(strlen("") + 1, sizeof(char));
    strcpy(commandStruct->outputFile, "");

    commandStruct->inputFile = calloc(strlen("") + 1, sizeof(char));
    strcpy(commandStruct->inputFile, "");
    x++;
    j++;

    while (j < i)
    {
        if (strcmp(args[j], "<") == 0)
        {
            // Since we've hit the indicator for input file we know the next arg is the input file
            j++; // hence, we increment j before assignment.
            free(commandStruct->inputFile); // Deallocte, reallocate
            commandStruct->inputFile = calloc(strlen(args[j]) + 1, sizeof(char));
            strcpy(commandStruct->inputFile, args[j]);
        }

        else if (strcmp(args[j], ">") == 0)
        {
            // Same as the input indicator, we know we want to pay attention to the next arg
            j++; // hence, we increment j before assignment.
            free(commandStruct->outputFile); // Deallocte, reallocate
            commandStruct->outputFile = calloc(strlen(args[j]) + 1, sizeof(char));
            strcpy(commandStruct->outputFile, args[j]);
        }

        // make sure & comes at the end of the string, otherwise it's treated as an argument.
        else if (strcmp(args[j], "&") == 0 && j == i - 1)
        {
            // Same as the input indicator, we know we want to pay attention to the next arg
            commandStruct->background = calloc(strlen(args[j]) + 1, sizeof(char));
            strcpy(commandStruct->background, args[j]);
        }

        // Since we've already taken the command out, and we know it's not an input or output file
        // We know it's an argument for the command and assign it as such. 
        else
        {
            commandStruct->arguments[x] = calloc(strlen(args[j]) + 1, sizeof(char));
            strcpy(commandStruct->arguments[x], args[j]);
            x++;
        }

        // Increment j to progress the index
        j++;
    }
    // Add a null at the end of the array so it works with execvp
    commandStruct->arguments[x + 1] = NULL;
    commandStruct->numberOfArgs = x;

    return commandStruct;
}

void printArgs(char **args, int i)
{
    for (int j=0; j < i; j++)
    {
        printf("%s\n", args[j]);
        fflush(stdout);
    }
}

void printCommand(struct userComm* userCommand)
{
    printf("Command: %s\nInput: %s\nOutput: %s\nBackground: %s\n", userCommand->command,
    userCommand->inputFile,
    userCommand->outputFile,
    userCommand->background);
    fflush(stdout);
    printArgs(userCommand->arguments, userCommand->numberOfArgs);
}

int exitFunction(int pid)
{
    // might need to change this to kill the process group
    // if so simply swith -2 to processGroupId.
    int processGroupId = -1 * pid;
    kill(-2 , SIGTERM);
    return 0;
}

int cdFunction(struct userComm* userCommand)
{
    // If there are no args for a cd command. 
    if (userCommand->numberOfArgs == 1)
    {
        // sends us to the directory specified in home.
        chdir(getenv("HOME"));
    }

    // Since cd should be passed something like .. or CS344/hudsonsc_program3
    // we should only have 1 argument
    else if (userCommand->numberOfArgs == 2)
    {
        chdir(userCommand->arguments[1]);
    }

    else
    {
        // cd only takes the one argument, any more and it's incorrect.
        return 1;
    }

    return 0;
}

int statusFunction(int fgExitValue, int fgTermSignal)
{
    // Can print either the exit status or the terminating signal. 
    if (fgTermSignal == 0)
    {
        printf("exit value %d\n", fgExitValue);
    }
    else
    {
        printf("terminated by signal %d\n", fgTermSignal);
    }
}

/*
* Adapted from <Stephen Brennan> (<16/01/15>) [<Tutorial - Write a Shell in C>]. https://brennan.io/2015/01/16/write-a-shell-in-c/
* Used to create child processes and run shell commands such as ls and ps. 
*/

int spawnChild(struct userComm* userCommand, int *startedProcesses, int *index)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    startedProcesses[(*index)] = pid; // Assign the pid to the array storing the values.
    (*index)++; // Increment the process index to
  
    if (pid == 0) {
    /*
    * Adapted from <CS 702 - Operating Systems> (<Spring 2005>) [<Using dup2 for I/O Redirection and Pipes>] <.shttp://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html
    * to get the general structure of using dup2() for I/O redirection. 
    */ 

    // Since we initalize input to an empty string, if it isn't assigned something else it will 
    // still be an empty string.
    if (strcmp(userCommand->inputFile, "") != 0)
    {
        // Copy the outputFile name into a temp var to avoid intermitent issues with name being undefined.
        char *tempIn;
        tempIn = malloc(sizeof(userCommand->inputFile) * sizeof(char));
        strcpy(tempIn, userCommand->inputFile);

        // open the file for write only, set permissions.
        int in = open(tempIn, O_RDONLY );

        if (in < 0) 
        {
            perror("smallsh");
            exit(1);
        }

        // dup2 redirects stdin from our file 
        dup2(in, 0);

        // Close our file since we no longer need it. 
        close(in);

    }

    // Since we initalize output to an empty string, if it isn't assigned something else it will 
    // still be an empty string.
    if (strcmp(userCommand->outputFile, "") != 0)
    {
        // Copy the outputFile name into a temp var to avoid intermitent issues with name being undefined.
        char *tempOut;
        tempOut = malloc(sizeof(userCommand->outputFile) * sizeof(char));
        strcpy(tempOut, userCommand->outputFile);

        // open the file for write only, set permissions.
        int out = open(tempOut, O_WRONLY | O_CREAT | O_TRUNC, 0640 );
        
        if(out < 0) 
        {
            perror("smallsh");
            exit(1);
        }


        // dup2 redirects stdout to our out file 
        dup2(out, 1);

        // Close our file since we no longer need it. 
        close(out);
    }


    // Check if the input file is present
    if (execvp(userCommand->command, userCommand->arguments) == -1) 
    {
        // If the child process fails we print the error with smallsh so it's clear
        // where the error is coming from.
        perror("smallsh");
        fflush(stdout);
    }

    exit(EXIT_FAILURE);
    }

    else if (pid < 0) 
    {
        // If the fork failed we display an error message along with the program title
        perror("smallsh");
        fflush(stdout);
    }
    
    else 
    {
    // Parent process will wait until we're good to go forward. 
        do 
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

  return 0;
}

int main()
{
    // Reserve space for a command up to 2048 characters long with two extra for newline
    char userCommand[2050];
    char *arguments[513];
    char exitCommand[] = "exit";
    char newLine[] = "\n";
    int i = 0;

    int startedProcesses[100]; // Fuck it. 
    int startedProcessIndex = 0;

    // Allocate memory for tracking exit status of fg processes and initialize it to 0
    // PROBS GET RID OF THIS
    int *fgExitValue; 
    fgExitValue = (int*)malloc(sizeof(int));
    *fgExitValue = 0;

    int *fgTermSignal; 
    fgTermSignal = (int*)malloc(sizeof(int));
    *fgTermSignal = 0; // Initialize to 0, could be wrong IDK term signals are only like 5 positive numbers.

    /* 
    * Adapted from <user2622016> (<09/28/15>) [<post response>]. https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
    * Used to change the pid int to a string so it can be concatenated if $$ is present.
    */
    int pId = getpid();
    int size = snprintf( NULL, 0, "%d", pId);
    char* processId = malloc( size + 1 );
    snprintf(processId, size + 1, "%d", pId); 
    int gpid = getpgrp();

    // Not sure if I need this...
    int gpidSize = snprintf( NULL, 0, "%d", gpid);
    char* groupProcessId = malloc( gpidSize + 1 );
    snprintf(groupProcessId, size + 1, "%d", pId); 


    do
    {
        // char cwd[256];
        // getcwd(cwd, sizeof(cwd));
        // printf should be okay here since this isn't a signal handler.
        // printf("%s in %s\n", processId, cwd);
        printf(": ");
        fflush(stdout);

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
                // Also make sure it's not an exit command because we don't want to mess with that.
                if (strcmp(arguments[0], exitCommand) != 0)
                {
                    replacePId(arguments, i, processId);
                    struct userComm *commandStruct = makeStruct(arguments, i);
                    // printArgs(commandStruct->arguments, i); 
                    // printCommand(commandStruct);   

                    if (strcmp(commandStruct->command, "cd") == 0)
                    {
                        cdFunction(commandStruct);
                    }

                    else if (strcmp(commandStruct->command, "status") == 0)
                    {
                        statusFunction(*fgExitValue, *fgTermSignal);
                    }

                    else
                    {
                        spawnChild(commandStruct, startedProcesses, &startedProcessIndex);
                        printf("%d\n %d\n", startedProcessIndex, startedProcesses[startedProcessIndex]);
                    }
                }
            }
        }

        // reset i after each iteration
        i = 0;

    } while (strcmp(arguments[0], exitCommand) != 0);

    // HERE IS WHERE WE'LL CALL/IMPLEMENT THE EXIT HANDLER.
    exitFunction(pId);

    free(processId);
    free(fgExitValue);
    return 0;
}
