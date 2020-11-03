#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int backgroundProcesses[100];
int backgroundProcessCount = 0;

int foregroundOnly = 1;
int fgStatus = 0;
int bgStatus;
int fgSignaled = 1;
int bgSignaled = 1;
int fgpid;

struct userComm
{
    char *arguments[512];
    int numberOfArgs;
    char *inputFile;
    char *outputFile;
    char *background;
};

/*
* This function searches through the commands arguments for the $$ character,
* if $$ is found it is replaced with the programs pid.
*/
void replaceCharacters(char **args, int argCount, char *pId)
{
    // For loop to loop through the args
    for (int x=0; x < argCount; x++)
    {
        int argLength = strlen(args[x]);
        int pIdLength = strlen(pId);
        int expandedArgumentLength = argLength + pIdLength;

        // Create variables to hold our strings. 
        char *expandedString = calloc( expandedArgumentLength + 2, sizeof(char));
        char originalString[argLength];

        // we copy over the argument to allow us to access it through subscript
        strcpy(originalString, args[x]);

        /*
        * The loop to search through a string was adapted from <StackOverFlow> (<12/18x/10>) <user257111> 
        * [<answer to a question>]. https://stackoverflow.com/questions/4475948/get-a-character-referenced-by-index-in-a-c-string
        * This was used to get portions of the string so that I can check for "$$".
        */

        char* stringAddress;
        // We use two seperate indexes to account for the possible difference when expansion occurs.
        int arrayIndex = 0;
        int stringIndex = 0;

        // Loop through the string, the first character is excluded on each iteration
        // and the first two characters are evaluated for whether they are $$.
        for ( stringAddress=&originalString[0]; *stringAddress != '\0'; stringAddress++ )
        {   
            if (strncmp(stringAddress, "$$", 2) == 0)
            {
                // If the first two characters of the substring are $$ we concatenated the pId into our array
                strcat(expandedString, pId);
                stringAddress++; // Increment s again so we don't look at the second $

                // arrayIndex is our index for the array, which we increment as much as needed for the pId
                arrayIndex = arrayIndex + pIdLength;

                // We increment our string index twice to skip copying the $$
                stringIndex = stringIndex + 2;
            }

            else
            {
                // if the $$ character is not found we assign an index and increment our pointers. 
                expandedString[arrayIndex] = originalString[stringIndex];
                arrayIndex++;
                stringIndex++;
            }

        }
        // We conclude by reassigning the variable.
        args[x] = expandedString;
    }
}

struct userComm *makeCommandStruct(char **args, int argCount)
{
    int inputArrayIndex = 0;
    int argArrayIndex = 0;
    struct userComm *commandStruct = malloc(sizeof(struct userComm)); 

    // Initialize input, output, and background to contain an empty string. 
    // This allows us to check if they got assigned and handle it accordingly.
    commandStruct->outputFile = calloc(strlen("") + 1, sizeof(char));
    strcpy(commandStruct->outputFile, "");

    commandStruct->inputFile = calloc(strlen("") + 1, sizeof(char));
    strcpy(commandStruct->inputFile, "");

    commandStruct->background = calloc(strlen("") + 1, sizeof(char));
    strcpy(commandStruct->inputFile, "");

    while (inputArrayIndex < argCount)
    {
        if (strcmp(args[inputArrayIndex], "<") == 0)
        {
            // Since we've hit the indicator for input file we know the next arg is the input file
            inputArrayIndex++; // hence, we increment j before assignment.
            free(commandStruct->inputFile); // Deallocte, reallocate
            commandStruct->inputFile = calloc(strlen(args[inputArrayIndex]) + 1, sizeof(char));
            strcpy(commandStruct->inputFile, args[inputArrayIndex]);
        }

        else if (strcmp(args[inputArrayIndex], ">") == 0)
        {
            // Same as the input indicator, we know we want to pay attention to the next arg
            inputArrayIndex++; // hence, we increment j before assignment.
            free(commandStruct->outputFile); // Deallocte, reallocate
            commandStruct->outputFile = calloc(strlen(args[inputArrayIndex]) + 1, sizeof(char));
            strcpy(commandStruct->outputFile, args[inputArrayIndex]);
        }

        // make sure & comes at the end of the string, otherwise it's treated as an argument.
        else if (strcmp(args[inputArrayIndex], "&") == 0 && inputArrayIndex == argCount - 1)
        {
            // Same as the input indicator, we know we want to pay attention to the next arg
            commandStruct->background = calloc(strlen(args[inputArrayIndex]) + 1, sizeof(char));
            strcpy(commandStruct->background, args[inputArrayIndex]);
        }

        // Since we've already taken the command out, and we know it's not an input or output file
        // We know it's an argument for the command and assign it as such. 
        else
        {
            commandStruct->arguments[argArrayIndex] = calloc(strlen(args[inputArrayIndex]) + 1, sizeof(char));
            strcpy(commandStruct->arguments[argArrayIndex], args[inputArrayIndex]);
            argArrayIndex++;
        }

        // Increment j to progress the index
        inputArrayIndex++;
    }
    // Add a null at the end of the array so it works with execvp
    commandStruct->arguments[argArrayIndex + 1] = NULL;
    commandStruct->numberOfArgs = argArrayIndex;
    if (foregroundOnly == 0)
    {
        // If foregroundOnly is true we just overwrite any possible background input 
        strcpy(commandStruct->background, "");
    }

    return commandStruct;
}

void sigintHandler (int signum)
{
    if (fgSignaled == 0)
    {
        char* message = "Terminated by signal 2\n";
        write(STDOUT_FILENO, message, 23);
        fflush(stdout);
    }
    else
    {
        char* newLine = "\n";
        write(STDOUT_FILENO, newLine, 3);
        fflush(stdout);
    }
}

void sigtstpHandler(int signum)
{
    if (foregroundOnly == 1)
    {
        foregroundOnly = 0;
        char* entryNotification = "Entering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, entryNotification, 49);
    }
    else 
    {
        foregroundOnly = 1;
        char* exitNotification = "Exiting foreground-only mode\n";
        write(STDOUT_FILENO, exitNotification, 29);
    }
}

void printArgs(char **args, int argCount)
{
    for (int currentIndex=0; currentIndex < argCount; currentIndex++)
    {
        printf("%s\n", args[currentIndex]);
        fflush(stdout);
    }
}

void printCommand(struct userComm* userCommand)
{
    printf("Input: %s\nOutput: %s\nBackground: %s\n", userCommand->inputFile,
    userCommand->outputFile,
    userCommand->background);
    fflush(stdout);
    printArgs(userCommand->arguments, userCommand->numberOfArgs);
}

void exitFunction()
{
    for ( int index = 0; index < backgroundProcessCount; index++)
    {
        kill(backgroundProcesses[index] , SIGTERM);
        // Why does sigterm show up? 
    }
}

int cdFunction(struct userComm* userCommand)
{
    // We know that the command will be the first index in our arguments. If there is only one we know
    // there are no args for a cd command and we call it on HOME. 
    if (userCommand->numberOfArgs == 1)
    {
        // sends us to the directory specified in home.
        chdir(getenv("HOME"));
    }

    // Since cd should be passed something like .. or CS344/hudsonsc_program3
    // we should only have 1 argument.
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

void displayBackgroundStatus()
{
    int bgSize = snprintf( NULL, 0, "%d", bgStatus);
    char* bgText = malloc( bgSize + 1 );
    snprintf(bgText, bgSize + 1, "%d", bgStatus);
    // Use printf here to avoid what I think was a race condition,
    // it originally used write and would display before the background process message

    if (bgSignaled == 0)
    {
        char bgTerminatedMessage[30] = "terminated by signal ";
        strcat(bgTerminatedMessage, bgText);
        strcat(bgTerminatedMessage, "\n");
        printf(bgTerminatedMessage);
        fflush(stdout);
        bgSignaled = 1;
    }
    else
    {
        char bgExitedMessage[30] = "exit value ";
        strcat(bgExitedMessage, bgText);
        strcat(bgExitedMessage, "\n");
        printf(bgExitedMessage);
        fflush(stdout);
    }
}

// Look familiar?
void displayForegroundStatus()
{
    int fgSize = snprintf( NULL, 0, "%d", fgStatus);
    char* fgText = malloc( fgSize + 1 );
    snprintf(fgText, fgSize + 1, "%d", fgStatus);

    if (fgSignaled == 0)
    {
        char terminatedMessage[30] = "terminated by signal ";
        strcat(terminatedMessage, fgText);
        strcat(terminatedMessage, "\n");
        write(STDOUT_FILENO, terminatedMessage, 30);
        fflush(stdout);
    }
    else
    {
        char exitedMessage[30] = "exit value ";
        strcat(exitedMessage, fgText);
        strcat(exitedMessage, "\n");
        write(STDOUT_FILENO, exitedMessage, 30);
        fflush(stdout);
        
    }
}

/*
* Adapted from <Stephen Brennan> (<16/01/15>) [<Tutorial - Write a Shell in C>]. https://brennan.io/2015/01/16/write-a-shell-in-c/
* Used to create child processes and run shell commands such as ls and ps. 
*/

int spawnChild(struct userComm* userCommand, struct sigaction sigintSignal, struct sigaction sigtstpSignal)
{
    pid_t pid, wpid, cpid;
    int status;
    int i = 0;

    pid = fork();
  
    if (pid == 0) {
    /*
    * dup2() calls adapted from <CS 702 - Operating Systems> (<Spring 2005>) [<Using dup2 for I/O Redirection and Pipes>] <.shttp://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html
    * to get the general structure of using dup2() for I/O redirection. 
    */ 

    // All the children need to ignore sigstp so we install that handler here. 
    sigtstpSignal.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sigtstpSignal, NULL);

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

    // If it's a background process and the output file hasn't been set.
    if (strcmp(userCommand->background, "&") == 0 && strcmp(userCommand->inputFile, "") == 0)
    {
        // dup2 redirects stdin to /dev/null
        int in = open("/dev/null", O_RDONLY);
        dup2(in, 0);
        close(in);
    }

    // If it's a background process and the output file hasn't been set.
    if (strcmp(userCommand->background, "&") == 0 && strcmp(userCommand->outputFile, "") == 0)
    {
        // dup2 redirects stdout to /dev/null
        int out = open("/dev/null", O_WRONLY);
        dup2(out, 1);
        close(out);
    }

    // Check if the input file is present
    if (execvp(userCommand->arguments[0], userCommand->arguments) == -1) 
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

        // Determine whether the process is foreground or background
        if (strcmp(userCommand->background, "&") == 0)
        {
            cpid = waitpid(pid, &bgStatus, WNOHANG);

            // Assign cpid to the cpid traker array
            backgroundProcesses[backgroundProcessCount] = cpid;
            backgroundProcessCount++;

            char message[30] = "background pid is ";
            // convert cpid to text so I can have it printed out. 
            int pidSize = snprintf( NULL, 0, "%d", pid);
            char* childProcessId = malloc( pidSize + 1 );
            snprintf(childProcessId, pidSize + 1, "%d", pid); 

            // Now that we're converted we put our message together
            strcat(message, childProcessId);
            strcat(message, "\n");
            
            // Deallocate our memory and then write our message 
            free(childProcessId);
            write(STDOUT_FILENO, message, 30);
            fflush(stdout);
        }

        // If a command is not meant to be run as a background process it will be run as a foreground by default. 
        else
        {
            // When we fork the child inherets the environment, so we need to reset the default action of control c.
            // We do it here so the background processes don't respond to it. 
            sigintSignal.sa_handler = sigintHandler;
            sigaction(SIGINT, &sigintSignal, NULL);
            do 
            {
                wpid = waitpid(pid, &fgStatus, WUNTRACED);
                fgpid = pid;
            } while (!WIFEXITED(fgStatus) && !WIFSIGNALED(fgStatus));

            if (WIFSIGNALED(fgStatus))
            {
                fgSignaled = 0;

                displayForegroundStatus();
            }
            if (WIFEXITED(fgStatus))
            {
                fgSignaled = 1;
            }
        }
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

    /* 
    * Adpated from <CS344> (<Exploration: Signal Handling API>) https://canvas.oregonstate.edu/courses/1784217/pages/exploration-signal-handling-api?module_item_id=19893105
    * to make the parent process ignore sigint and sigstp
    */
    struct sigaction ignore = {0};
    ignore.sa_handler = SIG_IGN;
    sigfillset(&ignore.sa_mask);
    ignore.sa_flags = 0;
   	sigaction(SIGINT, &ignore, NULL);

    struct sigaction stop_handler = {0};
    stop_handler.sa_handler = sigtstpHandler;
    sigaction(SIGTSTP, &stop_handler, NULL);

    /* 
    * Adapted from <user2622016> (<09/28/15>) [<post response>]. https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
    * Used to change the pid int to a string so it can be concatenated if $$ is present.
    */
    int pId = getpid();
    int size = snprintf( NULL, 0, "%d", pId);
    char* processId = malloc( size + 1 );
    snprintf(processId, size + 1, "%d", pId); 
    int gpid = getpgrp();

    do
    {
        // Check to see if a background process has completed here, it will only return one.
        if (backgroundProcessCount != 0)
        {
            int completedId = waitpid(-1, &bgStatus, WNOHANG);
            if (completedId > 0)
            {
                printf("background pid %d is done: ", completedId);
                displayBackgroundStatus();
            }
        }
        
        // printf should be okay here since this isn't a signal handler.
        printf(": ");
        fflush(stdout);

        // This was a unique bug in my control structure. If a user inputs ctrl-c arguments 0 is never assigned because
        // nothing was read in. As such, it would crash when the while condition evaluated. To get around this we just give
        // it a blank line to initialize the value. 
        arguments[0] = "";

        // Make sure something was read in, if isn't we don't execute the commands
        // this is in place because signals, namely ctrl-z would cause this loop to
        // trigger after it input and call the previous command again. 
        if (fgets(userCommand, 2050, stdin) != NULL)
        {

            if (strcmp(userCommand, newLine) != 0)
            {
                userCommand[strcspn(userCommand, "\n")] = 0;
                // Process the command
                char *ptr;
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
                        replaceCharacters(arguments, i, processId);
                        struct userComm *commandStruct = makeCommandStruct(arguments, i);
                        // displayBackgroundProcesses(); 
                        // printCommand(commandStruct);   

                        if (strcmp(commandStruct->arguments[0], "cd") == 0)
                        {
                            cdFunction(commandStruct);
                        }

                        else if (strcmp(commandStruct->arguments[0], "status") == 0)
                        {
                            displayForegroundStatus();
                        }

                        else
                        {
                            spawnChild(commandStruct, ignore, stop_handler);
                        }
                    }
                }
            }
        }

        // reset i after each iteration
        i = 0;

    } while (strcmp(arguments[0], exitCommand) != 0);

    // HERE IS WHERE WE'LL CALL/IMPLEMENT THE EXIT HANDLER.
    exitFunction();

    free(processId);
    return 0;
}
