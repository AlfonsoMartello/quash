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

void sigChildHandler()
  {
  		pid_t	pid;
      int status;

      while((pid = waitpid(pid, &status, WNOHANG)) > 0)
      {
        printf("Process with PID: %d exiting\n", pid );

      }


  }

void backgroundExecute(char** argArray)
    {
      signal(SIGCHLD, sigChildHandler);
      pid_t pid;
      int status;
      pid = fork();
      if(pid < 0)
        {
          printf("Fork failed\n");
          exit(-1);
        }
      else if(pid == 0)
        {
          execvp(argArray[0], argArray);
        }
      else
      {
        printf("Process with pid: %d running in background\n", pid);

      }

    }

  int manageResponse(char** argArray)
  {
    int quitBool = 1;
    int exitBool = 1;
    int cdBool = 1;
    int bgBool = 1;
    if (argArray[0] != NULL)
    {
      quitBool = strcmp(argArray[0], "quit");
      exitBool = strcmp(argArray[0], "exit");
      cdBool = strcmp(argArray[0], "cd");
      if(argArray[1] != NULL)
      {
        bgBool = strcmp(argArray[1], "&");

      }
      if (quitBool == 0 || exitBool == 0) //quitting quash 4.
      {
        return(1);
      }
      else if (cdBool == 0)
      {
        cd(argArray[1]);
      }
      else if(bgBool == 0)
        {
          backgroundExecute(argArray);
        }
      else
      {
        execute(argArray); //doing executables 1. and 2.
      }
    }
    return(0);
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
    while (1)
    {
      input = getCommandLine();
      parsedInput = parse(input);
      quit = manageResponse(parsedInput);
      free(input);
      free(parsedInput);
      if(quit == 1)
      {
        printf("Thanks for using quash\n");
        exit(1);
      }
    }
  }
