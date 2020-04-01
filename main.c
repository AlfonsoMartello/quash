#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>
#include <strings.h>
#include <signal.h>
#include <termios.h>
#define JOB_SIZE 150

void execute(char** argArray);
void cd(char* dir);
void set(char* str, char* path, char* home);

int numjobs = 0;

struct Job {
    int pid;
    int id;
    char* cmd;
};

static struct Job joblist[JOB_SIZE];


char* removeSpaces(char *str)
{
    int count = 0;

    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i];
    str[count] = '\0';
    return str;
}

int setEnvPath(char* str)
{
  char* env = strtok(str, "=");
  char* path = strtok(str, strchr(str,'/'));
  if(setenv(env, path, 1) == -1)
    {
      perror("Error, path not set\n");
    }
  else
  {
    return 1;
  }
}

void printjoblist()
  {
    printf("Current job: \n");
    for(int i = 0; i < JOB_SIZE; i++)
      {
        printf("Job ID: %d, PID: %d, Command: %s\n", joblist[i].id, joblist[i].pid, joblist[i].cmd);
      }
  }

  int manageResponse(char** argArray, char* path, char* home)
  {
    int quitBool = 1;
    int exitBool = 1;
    int cdBool = 1;
    int setBool = 1;
    if (argArray[0] != NULL)
    {
      quitBool = strcmp(argArray[0], "quit");
      exitBool = strcmp(argArray[0], "exit");
      cdBool = strcmp(argArray[0], "cd");
      setBool = strcmp(argArray[0], "set");
      if (quitBool == 0 || exitBool == 0) //quitting quash 4.
      {
        return(1);
      }
      else if (cdBool == 0)
      {
        cd(argArray[1]);
      }
      else if (setBool == 0)
      {
        set(argArray[1], path, home);
      }
      else
      {
        execute(argArray); //doing executables 1. and 2.
      }
    }
    return(0);
  }

  void set(char* str, char* path, char* home) //must preserve strings throughout execution for putenv to work
  {
    int homeBool = 1;
    int pathBool = 1;
    size_t size = 32;
    if (str == NULL) //not enough arguments provided
    {
      fprintf(stderr, "quash: set expects 1 additional argument\n");
    }
    else
    {
      char* tempStr;
      tempStr = malloc(size * sizeof(char));
      strcpy(tempStr, str);
      char* envVariable = strtok(tempStr, "="); //gets if set is working with HOME or PATH
      homeBool = strcmp(envVariable, "HOME");
      pathBool = strcmp(envVariable, "PATH");
      envVariable = strtok(NULL, "="); //gets remainder of string
      if (homeBool == 0) //setting HOME variable
      {
        strcpy(home, str);
        if(putenv(home) != 0)
        {
          perror("Error, home not set\n");
        }
      }
      else if (pathBool == 0) //setting PATH variable
      {
        strcpy(path, str);
        if(putenv(path) != 0)
        {
          perror("Error, path not set\n");
        }
      }
      else //user entered invalid input
      {
        fprintf(stderr, "quash: set expects argument of form HOME=/example/... or PATH=/example/bin:/newexample. Neither was provided.\n");
      }
      free(tempStr);
    }
  }

  void cd(char* dir)
  {
    char* home = getenv("HOME");
    if (dir == NULL) // cd changes to home directory
    {
      chdir(home);
    }
    else if(chdir(dir) == -1)
    {
      perror("quash: Specified directory does not exist");
    }
    else
    {
      chdir(dir);
    }
  }

  void execute(char** argArray)
  {
    pid_t pid;
    int status;
    pid = fork();
    if(pid < 0)
    {
      fprintf(stderr, "fork failed\n");
      exit(-1);
    }
    else if (pid == 0)
    {
      if(execvp(argArray[0], argArray) < 0) //Should automatically inherit environment of the parent 7.
      {
        char* my_var = argArray[0];
        fprintf(stderr, "quash %s: ", my_var); //PATH error message 6.
        perror("");
        exit(-1);
      }
    }
    else
    {
      wait(&status);
    }
  }

  char* getCommandLine()
  {
    char* inputLine = NULL; //Get command line input from user
    char* removeNewLine;
    size_t charNum = 0;
    size_t size = 0; //Initial size for all arrays
    printf("$> ");
    charNum = getline(&inputLine, &size, stdin);
    removeNewLine = strchr(inputLine, '\n');
    if (removeNewLine) *removeNewLine = 0; //gets rid of the newline char
    return(inputLine);
  }

  char** parse(char* inputLine)
  {
    char* tempString; //temperary string used for transfering from array of char to array of strings
    char** argArray; //array of strings holding each command line argument at a seperate index
    char* delim = " "; //The char that strtok uses to parse the inputLine
    size_t size = 32; //Initial size for all arrays
    int i = 0;
    int stringLen = -1; //The number of elements in argArray

    argArray = malloc(size * sizeof(char*));
    tempString = strtok(inputLine, delim);
    while (tempString != NULL)
    {
      argArray[i] = tempString;
      i++;
      tempString = strtok(NULL, delim);
    }
    argArray[i] = NULL; //now have array of strings of all command line arguments that is null terminated
    while (argArray[++stringLen] != NULL) //get length of string array for testing purposes
    {
      //do nothing
    }
    return(argArray);
  }

  int main(int argc, char** argv, char** envp)
  {
    int quit = 0;
    char* input;
    char** parsedInput;
    char* path;
    char* home;
    path = malloc(32 * sizeof(char));
    home = malloc(32 * sizeof(char));
    while (1)
    {
      input = getCommandLine();
      parsedInput = parse(input);
      quit = manageResponse(parsedInput, path, home);
      free(input);
      free(parsedInput);
      if(quit == 1)
      {
        free(path);
        free(home);
        printf("Thanks for using quash\n");
        exit(1);
      }
    }
  }
