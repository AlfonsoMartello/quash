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



void execute(char** command)
{
      int status;
      pid_t pid;
      pid = fork();
      if (pid == 0)
      {
          if (strlen(command[0]) > 0)
          {
              if (execvp(command[0], command) < 0)
              {
                  fprintf(stderr, "Command execution error\n");
                  exit(0);
              }
          } else {
              if (execvp(command[0], NULL) < 0)
              {
                  fprintf(stderr, "Command execution error \n");
                  exit(0);
              }
          }
      }
      else
      {
          waitpid(pid, &status, 0);
      }
  }


void parse(char* cmd)
  {

  }




int main(int argc, char** argv, char** envp)
  {

  }
