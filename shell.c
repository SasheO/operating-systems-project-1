#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;
pid_t root_parent_pid;

// TODO: cite startsWith https://stackoverflow.com/questions/15515088/how-to-check-if-string-starts-with-certain-string-in-c
bool startsWith(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char* trimString(char * s) // get rid of any white trailing white space and beginning or end
{
  // printf("%s\n";)
    return rtrim(ltrim(s));
}

// TODO: cite sliceString https://medium.com/@kkhicher1/how-to-slice-string-in-c-language-7a5fd3a5db46
char *sliceString(char *str, int start, int end)
{

    int i;
    int size = (end - start) + 2;
    char *output = (char *)malloc(size * sizeof(char));

    for (i = 0; start <= end; start++, i++)
    {
        output[i] = str[start];
    }

    output[size] = '\0';

    return output;
}

void signal_handler(int signum)
{ // should exit if is child process, otherwise do nothing
  pid_t current_pid = getpid();
  if (root_parent_pid!=current_pid){
    exit(0);
  }
}

void killLongRunningChildProcess(int signum)
{
    // TODO: print out message that process is killed. figure out why this print message doesn't work!!!
    char   buf[MAX_COMMAND_LINE_LEN];
    int current_pid = getpid();
    sprintf(buf, "Terminated pocess %d took too long to finish\n", current_pid);
    write(1, buf, strlen(buf));
    kill(current_pid, signum);
}

int main() {
    // Stores the string typed into the command line.
    signal(SIGINT,signal_handler); //register alarm_handler to handle SIGALRM
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
    char current_dir[MAX_COMMAND_LINE_LEN];
    char token_delimiter[] = " ";
    char *target_directory;
    int i, j;
    char * environment_variable;
    char * environment_variable_value;
    pid_t  pid;
    signal(SIGALRM,killLongRunningChildProcess); //register killLongRunningChildProcess to handle SIGALRM
  
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];

    root_parent_pid = getpid();
    	
    while (true) {
      
        do{ 
            getcwd(current_dir, MAX_COMMAND_LINE_LEN);
            // Print the shell prompt.
            printf("%s%s", current_dir, prompt);
            fflush(stdout);

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
        
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
 
        }while(command_line[0] == 0x0A);  // while just ENTER pressed

      
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

        i = 0;
        char *p = strtok (command_line, token_delimiter);

        while (p != NULL)
        {
          if (!startsWith(p, "$")){ // if token not a variable
            arguments[i++] = trimString(p);
          }
          else{ // if token is a variable
            p = getenv(trimString(sliceString(p, 1, strlen(p))));
            arguments[i++] = p;
          }
            
            p = strtok (NULL, token_delimiter);
        }

        arguments[i++] = NULL;


        if (strcmp(arguments[0],"cd") == 0){ // TODO: implement more complex cd scenarios https://www.tutorialspoint.com/how-to-use-cd-command-in-bash-scripts
          for (i=1; arguments[i] != NULL; i++){ // get directory from arguments
              if (!startsWith(arguments[i], "-")){ 
                  target_directory = arguments[i];
                  break;
              }
          }
          if (!startsWith(target_directory, "-")){ // if not a flag
              if (chdir(target_directory) != 0) { // try changing directory
                  printf("%s is not a valid directory\n", target_directory);
              }
          
          }
        }
        else if (strcmp(arguments[0],"pwd") == 0){ // TODO: implement pwd flags/more complex scenarios
          printf("%s\n", current_dir);
        }
        else if (strcmp(arguments[0],"echo") == 0){ // TODO: implement more complex echo scenarios from here https://kodekloud.com/blog/bash-echo-commands-examples/
          printf("%s", arguments[1]);
          for (i=2;arguments[i]!=NULL; i++){
            printf(" %s", arguments[i]);
          }
          printf("\n");
        }
        else if (strcmp(arguments[0],"exit") == 0){ // TODO: implement edge case of exit. ensure that if there are child processes, they end too
          exit(0);
        }
        else if (strcmp(arguments[0],"env") == 0){
          if (arguments[1]!=NULL){
            environment_variable_value = getenv(arguments[1]);
            if (environment_variable_value!=0){
              printf("%s\n", environment_variable_value);
            }
          }
          else{
            for (i=0; environ[i]!=NULL; i++) {
              printf("%d: %s\n", i, environ[i]);
            }
          }
        }
        else if (strcmp(arguments[0],"export") == 0 || strcmp(arguments[0],"setenv")==0){ // TODO: implement flags for export (set environment variables). 
          // get the environment_variable name and environment_variable_value
          if (arguments[1]!=NULL){
            environment_variable = strtok(arguments[1], "=");
            i = strlen(environment_variable);
            environment_variable[i] = '=';
            j = strlen(arguments[1]);
            if (strlen(arguments[1])!=i){ // meaning there is a = in the argument
              environment_variable = sliceString(arguments[1], 0, i-1);
              environment_variable_value = sliceString(arguments[1], i+1, j);
              i = setenv(environment_variable, environment_variable_value, 1);
              if (i!=0){
              printf("Error occured: %d\n", i);
              }
            }
          }
          
        }
        else{
            /* TODO: implement at least > and reimplement &. 
            do this by adding a function that checks the arguments for which is found there.
            this helps in knowing what does what: https://sites.google.com/onprem.com/onprem-solution-partners/qvest-overview?authuser=0
            */
          pid = fork();
          if (pid < 0){
            printf("Error forking\n");
          }
          else if (pid>0){ // parent process
            for (i=0; arguments[i]!=NULL; i++){} // get i to be length of arguments array
            if (trimString(arguments[i-1])!="&"){ // only wait for child process to end if command does not end with &
              wait(NULL);
            }
            
          }
          else{ // child process
            alarm(10);
            i = execvp(arguments[0], arguments);
          }
        }
        
    }
    return -1;
}