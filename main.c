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

void cd(char* dir)
  {
    if(dir == NULL)
      {
        chdir(getenv("HOME"));
      }
    else if(chdir(dir) == -1)
      {
        perror("No such directory found\n");
      }
    chdir(dir);
    setenv("PWD", dir, 1);
  }


void quit()
  {
    printf("Thanks for using quash\n");
    exit(1);
  }

void printjoblist()
  {
    printf("Current job: \n");
    for(int i = 0; i < JOB_SIZE; i++)
      {
        printf("Job ID: %d, PID: %d, Command: %s\n", joblist[i].id, joblist[i].pid, joblist[i].cmd);
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
      if(execvp(argArray[0], argArray) < 0)
      {
        perror("Error: ");
      }
    }
    else
    {
      wait(&status);
    }
  }

  char* getCommandLine()
  {
    char* inputLine; //Get command line input from user
    char* removeNewLine;
    size_t size = 0; //Initial size for all arrays
    printf("$> ");
    getline(&inputLine, &size, stdin);
    removeNewLine = strchr(inputLine, '\n');
    if (removeNewLine) *removeNewLine = 0; //gets rid of the newline char
    //free(removeNewLine);
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
    free(inputLine);
    return(argArray);
  }

  int main(int argc, char** argv, char** envp)
  {
    char* input;
    char** parsedInput;
    int i = 0;
    while (1)
    {
      input = getCommandLine();
      parsedInput = parse(input);
      execute(parsedInput);
      i++;
    }
    free(input);
    free(parsedInput);
  }
