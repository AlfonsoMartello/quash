#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define JOB_SIZE 150

void execute(char** argArray);
void cd(char* dir);
void set(char* str);
void backgroundExecute(char** argArray);
void checkBackground();
void updateJobs();
void printjoblist();
int pipeCheck(char** argArray);
void executePipe(char** arr1, char** arr2);
int fileDirIn(char** argArray);
void fileDirOut(char** argArray);
char** parse(char* inputLine);

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

void cleanUp()
{
  for (int i = 0; i < JOB_SIZE; i++)
  {
    if (joblist[i].jobid != 0)
    {
      free(joblist[i].cmd);
    }
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

int manageResponse(char** argArray)
{
  int fileInQuit = 0;
  int quitBool = 1;
  int exitBool = 1;
  int cdBool = 1;
  int setBool = 1;
  int backgroundBool = 0;
  int jobBool = 1;
  int pipeBool = 0;
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
    pipeBool = pipeCheck(argArray); //has index of pipe in argArray
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
    else if (pipeBool != 0)
    {
      char** arr1;
      char** arr2;
      arr1 = malloc(128 * sizeof(char*));
      arr2 = malloc(128 * sizeof(char*));
      int length2 = 0;
      while (argArray[length2] != NULL)
      {
        length2++;
      }
      int i = 0;
      while (i < pipeBool)
      {
        arr1[i] = argArray[i];
        i++;
      }
      arr1[i] = NULL;
      int j = 0;
      i++;
      while (j < length2 - pipeBool)
      {
        arr2[j] = argArray[i];
        j++;
        i++;
      }
      arr2[j] = NULL;
      int k = 0;
      executePipe(arr1, arr2);
      free(arr1);
      free(arr2);
    }
    else if (cdBool == 0)
    {
      cd(argArray[1]);
    }
    else if (setBool == 0)
    {
      set(argArray[1]);
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
      fileInQuit = fileDirIn(argArray);
      return(fileInQuit);
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

void set(char* str) //must preserve strings throughout execution for putenv to work
{
  char* path;
  char* home;
  path = getenv("PATH");
  home = getenv("HOME");
  //printf("Home before: %s\n", home);
  //printf("Path before: %s\n", path);
  int homeBool = 1;
  int pathBool = 1;
  size_t size = 128;
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
      if(setenv("HOME", envVariable, 1) != 0)
      {
        perror("Error, home not set\n");
      }
    }
    else if (pathBool == 0) //setting PATH variable
    {
      if(setenv("PATH", envVariable, 1) != 0)
      {
        perror("Error, path not set\n");
      }
    }
    else //user entered invalid input
    {
      fprintf(stderr, "quash: set expects argument of form HOME=/example/... or PATH=/example/bin:/newexample. Neither was provided.\n");
    }
    path = getenv("PATH");
    home = getenv("HOME");
    //printf("Home after: %s\n", home);
    //printf("Path after: %s\n", path);
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

void executePipe(char** arr1, char** arr2)
{
  pid_t pid1;
  pid_t pid2;
  int fds[2]; //0 is read, 1 is write
  pipe(fds);
  pid1 = fork();
  if (pid1 < 0)
  {
    fprintf(stderr, "fork failed\n");
    exit(-1);
  }
  else if (pid1 == 0)//child 1
  {
    close(fds[0]);
    dup2(fds[1], STDOUT_FILENO);
    close(fds[1]);
    if(execvp(arr1[0], arr1) < 0)
    {
      char* my_var = arr1[0];
      fprintf(stderr, "quash %s: ", my_var); //PATH error message 6.
      perror("");
      exit(-1);
    }
  }
  else //parent
  {
    pid2 = fork();
    if (pid2 < 0)
    {
      fprintf(stderr, "fork failed\n");
      exit(-1);
    }
    else if (pid2 == 0)//child 2
    {
      close(fds[1]);
      dup2(fds[0], STDIN_FILENO);
      close(fds[0]);
      if(execvp(arr2[0], arr2) < 0)
      {
        char* my_var = arr2[0];
        fprintf(stderr, "quash %s: ", my_var); //PATH error message 6.
        perror("");
        exit(-1);
      }
    }
    else //the parent, again
    {
      wait(NULL); //double, as only waits for one child to exit
      //wait(NULL);
    }
  }
}

int pipeCheck(char** argArray)
{
  int i = 0;
  int testResult = 1;
  while(argArray[i] != NULL)
  {
    testResult = strcmp(argArray[i], "|");
    if (testResult == 0)
    {
      return(i); //contains pipe
    }
    i++;
  }
  return(0);
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

int fileDirIn(char** argArray)
{
  int numOfElements = 0;
  int numDeleted = 0;
  int quitBool = 0; //0 means don't quit
  char* cmd = NULL;
  char* removeNewLine;
  char** newStrArray;
  size_t charNum = 0;
  size_t sizeI = 0;
  size_t size = 128;
  char* delim = " ";
  char* filename = argArray[1];
  FILE* file;
  int i = 0;
  int k = 0;
  newStrArray = malloc(size * sizeof(char*)); //needs to be freed
  file = freopen(filename, "r", stdin);
  if(file != NULL)
  {
    while(!feof(file))
    {
      charNum = getline(&cmd, &sizeI, file);
      removeNewLine = strchr(cmd, '\n');
      if (removeNewLine) *removeNewLine = 0; //gets rid of the newline char
      newStrArray[i] = malloc(size * sizeof(char*)); //needs to be freed
      strcpy(newStrArray[i], cmd);
      i++;
    }
    newStrArray[i] = NULL;
    free(newStrArray[i-1]);
    newStrArray[i-1] = NULL;
  }
  char** parsedCommand;
  int j = 0;
  while (newStrArray[j] != NULL)
  {
    if (quitBool != 1)
    {
      checkBackground();
      parsedCommand = parse(newStrArray[j]);
      quitBool = manageResponse(parsedCommand);
      j++;
      free(parsedCommand);
    }
    if (quitBool == 1)
    {
      j++;
    }
  }
  while(newStrArray[k] != NULL)
  {
    free(newStrArray[k]);
    k++;
  }
  free(cmd);
  free(newStrArray);
  freopen("/dev/tty", "w+", stdin);
  if (quitBool == 1)
  {
    return(1);
  }
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
    if(execvp(argArray[0], argArray) < 0)
    {
      char* my_var = argArray[0];
      fprintf(stderr, "quash %s: ", my_var); //PATH error message 6.
      perror("");
      exit(-1);
    }
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
  size_t size = 128; //Initial size for all arrays
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
  char* pathCheck = getenv("PATH");
  while (1)
  {
    checkBackground();
    printf("$> ");
    input = getCommandLine();
    parsedInput = parse(input);
    quit = manageResponse(parsedInput);
    free(input);
    free(parsedInput);
    if(quit == 1)
    {
      cleanUp();
      printf("Thanks for using quash\n");
      exit(1);
    }
  }
}
