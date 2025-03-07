/***************************************************************************************
*    Title: sleep.c
*    Author: Mezisashe Ojuba
*    Date: 23 Oct 2024
*    Code version: 1.0
*    Availability: https://github.com/SasheO/operating-systems-project-1
*
***************************************************************************************/

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
bool redirect_stdout;
char print_buffer[MAX_COMMAND_LINE_LEN];

bool starts_with(const char *a, const char *b) // [1]
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

char* trim_string(char * s) // get rid of any white trailing white space and beginning or end
{
    return rtrim(ltrim(s));
}

char *slice_string(char *str, int start, int end) // [2]
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

void kill_long_running_child_process(int signum)
{
    // TODO: print out message that process is killed. figure out why this print message doesn't work!!!
    char buf[MAX_COMMAND_LINE_LEN];
    int current_pid = getpid();
    sprintf(buf, "Terminated process %d took too long to finish\n", current_pid);
    write(1, buf, strlen(buf));
    kill(current_pid, signum); // TODO: why does kill pid fail?
    exit(0);
}

void inbuilt_forward_redirection(int j, char * token){
  pid_t current_pid;
  int file_desc;
  current_pid = getpid();
  if (j == 0 && token[0] != '0'){   // token is not a number
    file_desc = open (token, O_WRONLY|O_CREAT|O_TRUNC,  0777); // gave bad file descriptor error without 0777
    if (file_desc==-1){
      perror("error opening file:");
        // it is expected this function is only run in a child process so this should terminate
        if (root_parent_pid!=current_pid){ // this is running in a child process
          exit(0);
        }
    }
    dup2(file_desc, 1); // instead of writing to stdout (1), will write to file_desc
  }
  else { // token is a number e.g. 2 for stderr
    if (j==0){ // TODO: 0 is for stdin. invalid input
    }
    else{
      dup2(j,1);
    }
  }

  write(1,print_buffer, strlen(print_buffer));

  j = close(file_desc); // have to close file descriptor after opening
  if (j==-1){
    perror("error closing file:");
  }

  // it is expected this function is only run in a child process so this should terminate
  if (root_parent_pid!=current_pid){ // this is running in a child process
    exit(0);
  }
}

void inbuilt_forward_redirection_loop(char *arguments[], int * redirect_arr){
  int i, num, pid;
  char * token;
  i = redirect_arr[1]+1; // i = index of next token after >
  if (arguments[i]==NULL){ // if there is just a > but not arguments after
    sprintf(print_buffer, "Invalid command syntax\n");
    write(2, print_buffer, strlen(print_buffer));
  }
  else{
    token = arguments[i];
    num = atoi( token ); // 
    pid = fork();
    if (pid==0){ // child process
      inbuilt_forward_redirection(num, token);
    }
    else if (pid<0){
      perror("Error forking:");
    }
    else{
      wait(NULL);
    }
  }
}

int* pwd(char *arguments[], int starting_indx){ // no flags implemented
  int indx;
  int* redirect = malloc(2*sizeof(int));
  redirect[0] = 0;
  redirect[1] = 0;
  char current_dir[MAX_COMMAND_LINE_LEN];
  getcwd(current_dir, MAX_COMMAND_LINE_LEN);
  sprintf(print_buffer, "%s\n", current_dir);
  char* token;
  
  for (indx=starting_indx; arguments[indx] != NULL; indx++){
    token = trim_string(arguments[indx]);
    if (strcmp(token, ">")==0){
      redirect[0] = 1;
      redirect[1] = indx;
    }
  }
  return redirect;
}

int* echo(char *arguments[]){
  int i;
  int* redirect = malloc(2*sizeof(int));
  redirect[0] = 0;
  redirect[1] = 0;
  sprintf(print_buffer, "%s", arguments[1]);
  for (i=2;arguments[i]!=NULL; i++){
    if (strcmp(arguments[i], ">")==0){
      redirect[0] = 1;
      redirect[1] = i;
      break;
    }
    sprintf(print_buffer + strlen(print_buffer)," %s", arguments[i]);
  }
  sprintf(print_buffer + strlen(print_buffer), "\n");
  return redirect;
}

int * env(char * arguments[]){
  int * redirect = malloc(2*sizeof(int));
  redirect[0] = 0;
  redirect[1] = 0;
  char * environment_variable;
  char * environment_variable_value;
  int indx;

  if (arguments[1]!=NULL){
    if (strcmp(arguments[1],">") != 0){ // not a redirection, but rather the name of the environment variable to check
      environment_variable_value = getenv(arguments[1]);
      sprintf(print_buffer, "%s\n", environment_variable_value);
        // check if any more redirections
        for (indx=2;arguments[indx]!=NULL; indx++){
          if (strcmp(arguments[indx], ">")==0){
            redirect[0] = 1;
            redirect[1] = indx;
            break;
          }
        }
    }

    else{ // i.e. the command is "env > filename"
        redirect[0] = 1;
        redirect[1] = 1; // indx of >
        sprintf(print_buffer, "1: %s\n", environ[indx]);
        for (indx=1; environ[indx]!=NULL; indx++) {
          sprintf(print_buffer + strlen(print_buffer),"%d: %s\n", indx, environ[indx]);     
        }          
    }

  }

  else{ // argument[1]==NULL, meaning the command is just env with no other tokens
    sprintf(print_buffer, "1: %s\n", environ[indx]);
    for (indx=1; environ[indx]!=NULL; indx++) {
      sprintf(print_buffer + strlen(print_buffer),"%d: %s\n", indx, environ[indx]);
    }
    
  }
  return redirect;
}

int export(char *arguments[]){
  // TODO: change this to return int* redirect like pwd and env functions
  char * environment_variable;
  char *  environment_variable_value;
  int i,j;
  int error_state = 1;
  if (arguments[1]!=NULL){
  environment_variable = strtok(arguments[1], "=");
  i = strlen(environment_variable);
  environment_variable[i] = '=';
  j = strlen(arguments[1]);
  if (strlen(arguments[1])!=i){ // meaning there is a = in the argument
    environment_variable = slice_string(arguments[1], 0, i-1);
    environment_variable_value = slice_string(arguments[1], i+1, j);
    i = setenv(environment_variable, environment_variable_value, 1);
    if (i!=0){
      sprintf(print_buffer, "Error occured: %d\n", i);
      error_state = -1;
      }
    }
  }
  return error_state;
}

int shell_command(char *arguments[]){
  // TODO: should this function be the one to print error message or its caller? should it just be returning error codes? decide and change all the perror messages if needed
  int i, j, file_desc;
  char * token;
  int * redirect_arr = malloc(2*sizeof(int));
  redirect_arr[0] = 0;
  redirect_arr[1] = 0;
  for (i=1; arguments[i] != NULL; i++){
    token = trim_string(arguments[i]);
    if (strcmp(token, ">")==0){
      redirect_arr[0] = 1;
      redirect_arr[1] = i;
      arguments[i] = NULL;
      break;
    }
  }

  if (redirect_arr[0]==1)  {
    token = arguments[i+1];
    if (token==NULL){
      perror("Invalid syntax:");
      return -1;
    }
      j = atoi( token ); // 
    if (j == 0 && token[0] != '0'){   // token is not a number
      file_desc = open (token, O_WRONLY|O_CREAT|O_TRUNC,  0777); // gave bad file descriptor error without 0777
      if (file_desc==-1){
        perror("error opening file:");
        return -1;
      }
      dup2(file_desc, 1); // instead of writing to stdout (1), will write to file_desc
    }
    else { // token is a number e.g. 2 for stderr
      if (j==0){ // TODO: 0 is for stdin. invalid input
      }
      else{
        dup2(j,1);
      }
    }
  }
  

  i = execvp(arguments[0], arguments);
  return i;
}

int main() {
    // Stores the string typed into the command line.
    signal(SIGINT,signal_handler); //register alarm_handler to handle SIGALRM
    char command_line[MAX_COMMAND_LINE_LEN];
    char misc_buffer[MAX_COMMAND_LINE_LEN];
    char current_dir[MAX_COMMAND_LINE_LEN];
    char token_delimiter[] = " ";
    char *target_directory;
    int i, j; // integer variables used in for loops, etc
    char *token; // string used to hold next token
    int* redirect_arr; // pointer to array for holding redirections instructions (e.g. for >, |, < commands)
    char * environment_variable;
    char * environment_variable_value;
    pid_t  pid; // will hold process id whenever fork is done
    char *arguments[MAX_COMMAND_LINE_ARGS]; // Stores the tokenized command line input.
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

        // reset basic variables for each command processing
        i = 0; 
        redirect_stdout = 0; 
        char *p = strtok (command_line, token_delimiter);

        while (p != NULL)
        {
          if (!starts_with(p, "$")){ // if token not a variable
            arguments[i++] = trim_string(p);
          }
          else{ // if token is a variable
            p = getenv(trim_string(slice_string(p, 1, strlen(p))));
            if (p==NULL){p="";}
            arguments[i++] = p;
          }
            
            p = strtok (NULL, token_delimiter);
        }

        arguments[i++] = NULL;

        if (strcmp(arguments[0],"cd") == 0){ 
          for (i=1; arguments[i] != NULL; i++){ // get directory from arguments
              if (!starts_with(arguments[i], "-")){ // check if not a flag e.g. -P
                  target_directory = arguments[i];
                  if (chdir(target_directory) != 0) { // try changing directory
                    sprintf(print_buffer, "%s is not a valid directory\n", target_directory);
                    write(2, print_buffer, strlen(print_buffer));
                    // TODO: implement error forward redirect, move write to error descriptor here
                  }
                  break;
              }
          }
        }
        else if (strcmp(arguments[0],"pwd") == 0){ 
          redirect_arr = pwd(arguments, 0);
          if (redirect_arr[0]==1){ // if there is a redirect argument
            inbuilt_forward_redirection_loop(arguments, redirect_arr);
          }
          else{
            write(1, print_buffer, strlen(print_buffer));
          }
        }
        else if (strcmp(arguments[0],"echo") == 0){ 
          redirect_arr = echo(arguments);
          if (redirect_arr[0]==1){ // if there is a redirect argument
            inbuilt_forward_redirection_loop(arguments, redirect_arr);
          }
          else{
            write(1, print_buffer, strlen(print_buffer));
          }
        }
        else if (strcmp(arguments[0],"exit") == 0){ // TODO: implement edge case of exit. ensure that if there are child processes, they end too
          exit(0);
        }
        else if (strcmp(arguments[0],"env") == 0){
          redirect_arr = env(arguments);
          if (redirect_arr[0]==1){
            inbuilt_forward_redirection_loop(arguments, redirect_arr);
          }
          else{
            write(1, print_buffer, strlen(print_buffer));
          }
        }
        else if (strcmp(arguments[0],"export") == 0 || strcmp(arguments[0],"setenv")==0){ 
          // TODO: standardize the output of export to be an int * array of redirect_arr, like pwd and env
          i = export(arguments); 
          if (i==-1){
            write(2, print_buffer, strlen(print_buffer));
            // TODO: add write to error file  descriptor
          }
          
        }
        else{
          signal(SIGALRM,kill_long_running_child_process); //register kill_long_running_child_process to handle SIGALRM
          
          pid = fork();
          if (pid < 0){
            sprintf(print_buffer, "Error forking\n"); 
            write(2, print_buffer, strlen(print_buffer));
            // TODO: add write to error file  descriptor
            
          }
          else if (pid>0){ // parent process

            for (i=0; arguments[i]!=NULL; i++){} // get i to be length of arguments array
            if (trim_string(arguments[i-1])!="&"){ // only wait for child process to end if command does not end with &
              wait(NULL);
            }
            
          }
          else{ // child process
            sprintf(misc_buffer, "which %s > /dev/null 2>&1", arguments[0]);
            if (system(misc_buffer)) { // command doesn't exist e.g. typo
              printf("%s command not found.\n", arguments[0]);
            }
            else{ // command exists
              alarm(10);
              // TODO: should shell_command function be the one to print error message or main? should shell_command just be returning error codes which main interprets? decide and change all the perror messages if needed
              i = shell_command(arguments);
              pid = getpid();
              if (pid!=root_parent_pid){
                exit(0);
              }
            }
          }
        }
        
    }
    return -1;
}