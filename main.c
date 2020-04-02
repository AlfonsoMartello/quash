#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define JOB_SIZE 150

void execute(char** argArray);
void cd(char* dir);
void set(char* str, char* path, char* home);
void backgroundExecute(char** argArray);
void checkBackground();
void updateJobs();
void printjoblist();
void fileDirIn(char** argArray);
void fileDirOut(char** argArray);

int backNumJobs = 0;
int foreNumJobs = 0;
int jobID = 0;


struct Job {
    int jobid;
    pid_t pid;
    char* cmd;
    int numofforeground;
};

static struct Job joblist[JOB_SIZE];

void init()
{
  for (int i = 0; i < JOB_SIZE; i++)
  {
    joblist[i].jobid = 0;
    joblist[i].pid = 0;
    joblist[i].cmd = "";
    joblist[i].numofforeground = 0;
  }
}

int containsAmper(char** argArray)
{
  int returnVal = 0;
  int i;
  for(i = 0; argArray[i] != NULL; i++)
  {
    //loop through all elements
  }
  returnVal = strcmp(argArray[i-1], "&");
  if (returnVal == 0)
  {
    argArray[i-1] = NULL; //removes & from argArray
    return(1); //contains & at last position
  }
  return(0);
}

int manageResponse(char** argArray, char* path, char* home)
{
  int quitBool = 1;
  int exitBool = 1;
  int cdBool = 1;
  int setBool = 1;
  int backgroundBool = 0;
  int jobBool = 1;
  int fileInBool = 1;
  int fileOutBool = 1;
  if (argArray[0] != NULL)
  {
    quitBool = strcmp(argArray[0], "quit");
    exitBool = strcmp(argArray[0], "exit");
    cdBool = strcmp(argArray[0], "cd");
    setBool = strcmp(argArray[0], "set");
    backgroundBool = containsAmper(argArray);
    jobBool = strcmp(argArray[0], "jobs");
    fileInBool = strcmp(argArray[0], "<");
    if(argArray[1] != NULL && fileInBool != 0)
      {
        fileOutBool = strcmp(argArray[1], ">");
        argArray[1] = NULL;
      }
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
    else if (jobBool == 0)
    {
      printjoblist();
    }
    else if (backgroundBool == 1)
    {
      backgroundExecute(argArray);
    }
    else if (fileInBool == 0)
    {
      fileDirIn(argArray);
    }
    else if (fileOutBool == 0)
    {
      fileDirOut(argArray);
    }
    else
    {
      foreNumJobs++;
      execute(argArray); //doing executables 1. and 2.
      updateJobs();
      foreNumJobs = 0;
    }
  }
  return(0);
}

void printjoblist()
{
  printf("Current jobs: \n");
  for(int i = 0; i < JOB_SIZE; i++)
  {
    if (joblist[i].jobid != 0)
    {
      printf("[%d] %d %s\n", joblist[i].jobid, joblist[i].pid, joblist[i].cmd);
    }
  }
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
  else //parent
  {
    wait(NULL);
  }
}

void updateJobs()
{
  for (int i = 0; i < JOB_SIZE; i++)
  {
    if (joblist[i].jobid != 0)
    {
      joblist[i].numofforeground = joblist[i].numofforeground + foreNumJobs;
    }
  }
}

int getValidIndex()
{
  for (int i = 0; i < JOB_SIZE; i++)
  {
    if (joblist[i].jobid == 0)
    {
      return(i);
    }
  }
  return(-1);
}

int getJob(pid_t pid)
{
  int i;
  for (i = 0; i < JOB_SIZE; i++)
  {
    //printf("Value of pid passed in: %d\n", pid);
    //printf("Value of i in getJob: %d\n", i);
    //printf("Value of pid in joblist: %d\n", joblist[i].pid);
    //printf("Value of foreNum: %d\n", joblist[i].numofforeground);
    if (joblist[i].pid == (pid - joblist[i].numofforeground))
    {
      return(i);
    }
  }
  return(-1);
}

void checkBackground()
{
  int status;
  pid_t pid;
  int jobIndex;
  while((pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
    jobIndex = getJob(pid);
    if (jobIndex == -1)
    {
      printf("Oh no... Something went wrong with the background process...\n");
    }
    else
    {
      int i;
      printf("[%d] %d finished %s\n", joblist[jobIndex].jobid, joblist[jobIndex].pid, joblist[jobIndex].cmd);
      free(joblist[jobIndex].cmd);
      joblist[jobIndex].jobid = 0;
      joblist[jobIndex].pid = 0;
      joblist[jobIndex].cmd = "";
      joblist[jobIndex].numofforeground = 0;
      backNumJobs--;
    }
  }
}


void fileDirOut(char** argArray)
  {
    char* filename = argArray[2];
    FILE* file;
    file = freopen(filename, "w+", stdout);
    if(file != NULL)
      {
        execute(argArray);
        freopen("/dev/tty", "w", stdout);
      }
  }

void fileDirIn(char** argArray)
  {
    char* cmd;
    int i = 0;
    char* filename = argArray[1];
    FILE* file;
    file = freopen(filename, "r", stdin);
//    argArray[1] = NULL;
    if(file != NULL)
      {
        while(!feof(file))
        {

          cmd = fgets(argArray[i], 20, file);
          argArray[i] = strtok(cmd, " ");
        //  argArray[i+1] = strtok(NULL, "\n");
          i++;
        }
      }

      for(i = i; i >= 0; i--)
        {
          argArray[i] = strtok(argArray[i],"\n");
        }

   //argArray[i-1] = strtok(argArray[i-1], "\n");
    execute(argArray);
    freopen("/dev/tty", "w+", stdin);
  }



void backgroundExecute(char** argArray)
{
  pid_t pid;
  int status;
  int index = 0;
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
    if (backNumJobs < JOB_SIZE)
    {
      jobID = jobID + 1;
      index = getValidIndex();
      joblist[index].jobid = jobID;
      joblist[index].pid = pid;
      joblist[index].cmd = (char *) malloc(100); //must be freed in checkBackground
      strcpy(joblist[index].cmd, argArray[0]);
      strcat(joblist[index].cmd, " ");
      int i = 1;
      while (argArray[i] != NULL)
      {
        strcat(joblist[index].cmd, argArray[i]);
        strcat(joblist[index].cmd, " ");
        i++;
      }
      printf("[%d] %d running in background\n", joblist[index].jobid, joblist[index].pid);
      backNumJobs++;
    }
    else //too many background jobs
    {
      printf("Sorry, quash cannot support that many background jobs. Your background exection failed.\n");
    }
  }
}

char* getCommandLine()
{
  char* inputLine = NULL; //Get command line input from user
  char* removeNewLine;
  size_t charNum = 0;
  size_t size = 0; //Initial size for all arrays
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
  init();
  int quit = 0;
  char* input;
  char** parsedInput;
  char* path;
  char* home;
  path = malloc(32 * sizeof(char));
  home = malloc(32 * sizeof(char));
  while (1)
  {
    checkBackground();
    printf("$> ");
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
