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
    char buf[MAX_COMMAND_LINE_LEN];
    int current_pid = getpid();
    sprintf(buf, "Terminated pocess %d took too long to finish\n", current_pid);
    write(1, buf, strlen(buf));
    kill(current_pid, signum); // TODO: why does kill pid fail?
    exit(0);
}

int* pwd(char *arguments[], int starting_indx){ // no flags implemented
  // TODO: return int array where first number is bool if redirect exists after applying flag. number 2 is pointed to arguments redirect if it exists
  int indx;
  int* redirect = malloc(2*sizeof(int));
  redirect[0] = 0;
  redirect[1] = 0;
  char current_dir[MAX_COMMAND_LINE_LEN];
  getcwd(current_dir, MAX_COMMAND_LINE_LEN);
  sprintf(print_buffer, "%s\n", current_dir);
  char* token;
  
  for (indx=starting_indx; arguments[indx] != NULL; indx++){
    token = trimString(arguments[indx]);
    if (strcmp(token, ">")==0){
      redirect[0] = 1;
      redirect[1] = indx;
    }
  }
  return redirect;
}

int* echo(char *arguments[]){
  int i;
  // TODO: return int array where first number is bool if redirect exists after applying flag. number 2 is pointed to arguments redirect if it exists
  int* redirect = malloc(2*sizeof(int));
  sprintf(print_buffer, "%s", arguments[1]);
  for (i=2;arguments[i]!=NULL; i++){
    sprintf(print_buffer + strlen(print_buffer)," %s", arguments[i]);
  }
  sprintf(print_buffer + strlen(print_buffer), "\n");
  return redirect;
}

int export(char *arguments[]){
  // TODO: return -1 if error, 1 if no error
  char * environment_variable;
  char * environment_variable_value;
  int i,j;
  int error_state = 1;
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
      sprintf(print_buffer, "Error occured: %d\n", i);
      error_state = -1;
      }
    }
  }
  return error_state;
}

void forward_redirection(int j, char * token){
  pid_t current_pid;
  int file_desc;
  
  current_pid = getpid();

  if (j == 0 && token[0] != '0'){   // token is not a number
    file_desc = open (token, O_WRONLY|O_CREAT|O_TRUNC,  0777); // gave bad file descriptor error without 0777
    if (file_desc==-1){
      perror("error opening file:");
      if (root_parent_pid!=current_pid){
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

  // should exit if is child process, otherwise do nothing. 
  // it is expected this function is only run in a child process so this should terminate
  if (root_parent_pid!=current_pid){
    exit(0);
  }
}

int main() {
    // Stores the string typed into the command line.
    signal(SIGINT,signal_handler); //register alarm_handler to handle SIGALRM
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
    char current_dir[MAX_COMMAND_LINE_LEN];
    char token_delimiter[] = " ";
    char *target_directory;
    int i, j, k;
    char *token;
    int* redirect_arr;
    char * environment_variable;
    char * environment_variable_value;
    pid_t  pid;
    int file_desc;
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

        // reset basic variables for each command processing
        i = 0; 
        redirect_stdout = 0; 
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

        /* TODO: implement at least > and reimplement &. 
            do this by adding a function that checks the arguments for which is found there.
            this helps in knowing what does what: 
             https://thoughtbot.com/blog/input-output-redirection-in-the-shell
             https://www.gnu.org/software/bash/manual/html_node/Redirections.html
             https://www.youtube.com/watch?v=XGSK5xr_B_Q

            implementing redirect_stdout:
            https://stackoverflow.com/questions/17071702/c-language-read-from-stdout 
            - check that redirect stdout symbol is there
            - check that there is a filename/descriptor redirecting to afterwards
            // - check the syntax is right e.g. echo hi 1>/dev/null
            - execute


            
            

            solution:
              example: echo hi > hi.txt
              at every command, clear print buffer and reset file descriptors if needed
              implement a check here for syntax and if it includes redirection > (for now, may implement others later)
              have a while loop that runs for each part of the command i.e. echo hi is decoded, then > hi.txt
                idea:
                  - for each command, move it to its own function that accepts outputs
                  - return an array, or an int, or whatever depending on the command
                  - for commands that print e.g. echo hi > hi.txt, after processing the first two, it should return the result of the string. if no more redirect, print to stdout otherwise redirect
              at initial step, do the computation.
              make all printf to be sprintf to a buffer, then decide where to print it to depending on if there is a redirection or not
              if there is more (e.g. a redirection, create the file description and dup2(file description, 1) or whatever needs to be done)
              using dup2: https://www.geeksforgeeks.org/dup-dup2-linux-system-call/
            */

            /*
            syntax check: check if redirection exists. if it does, check if there is a file afterwards. 
            break command according to redirections, execute them one after another in order
            */

        if (strcmp(arguments[0],"cd") == 0){ // TODO: implement more complex cd scenarios https://www.tutorialspoint.com/how-to-use-cd-command-in-bash-scripts
          for (i=1; arguments[i] != NULL; i++){ // get directory from arguments
              if (!startsWith(arguments[i], "-")){ 
                  target_directory = arguments[i];
                  break;
              }
          }
          if (!startsWith(target_directory, "-")){ // if not a flag
              if (chdir(target_directory) != 0) { // try changing directory
                  sprintf(print_buffer, "%s is not a valid directory\n", target_directory);
                  write(2, print_buffer, strlen(print_buffer));
                  // TODO: add write to error descriptor here
              }
          
          }
        }
        else if (strcmp(arguments[0],"pwd") == 0){ // TODO: implement pwd flags/more complex scenarios
          redirect_arr = pwd(arguments, 0);
          // TODO: put while loop here. while redirect_arr[0]!=0 do something and recalculate redirect_arr = pwd(...)
          if (redirect_arr[0]==1){ // if there is a redirect argument
            i = redirect_arr[1]+1; // i = index of next token after >
            if (arguments[i]==NULL){ // if there is just a > but not arguments after
              sprintf(print_buffer, "Invalid command syntax\n");
              write(2, print_buffer, strlen(print_buffer));
            }
            else{
              token = arguments[i];
              j = atoi( token ); // 
              k = fork();
              if (k==0){ // child process
                forward_redirection(j, token);
              }
              else if (k<0){
                printf("Error forking\n");
              }
              else{
                wait(NULL);
              }
            }
          }
          else{
            write(1, print_buffer, strlen(print_buffer));
          }
          // TODO: add write to file descriptor (stdout, or a file if a redirection)
        }
        else if (strcmp(arguments[0],"echo") == 0){ // TODO: implement more complex echo scenarios from here https://kodekloud.com/blog/bash-echo-commands-examples/
          echo(arguments);
          write(1, print_buffer, strlen(print_buffer));
          // TODO: add write to file descriptor (stdout, or a file if a redirection)
        }
        else if (strcmp(arguments[0],"exit") == 0){ // TODO: implement edge case of exit. ensure that if there are child processes, they end too
          exit(0);
        }
        else if (strcmp(arguments[0],"env") == 0){
          if (arguments[1]!=NULL){
            environment_variable_value = getenv(arguments[1]);
            if (environment_variable_value!=0){
              sprintf(print_buffer, "%s\n", environment_variable_value);
              write(1, print_buffer, strlen(print_buffer));
          // TODO: add write to file descriptor (stdout, or a file if a redirection)
            }
          }
          else{
            for (i=0; environ[i]!=NULL; i++) {
              sprintf(print_buffer, "%d: %s\n", i, environ[i]);
            write(1, print_buffer, strlen(print_buffer));
            // TODO: add write to file descriptor (stdout, or a file if a redirection)
            }
          }
        }
        else if (strcmp(arguments[0],"export") == 0 || strcmp(arguments[0],"setenv")==0){ // TODO: implement flags for export (set environment variables). 
          // get the environment_variable name and environment_variable_value
          i = export(arguments);
          if (i==-1){
            write(2, print_buffer, strlen(print_buffer));
            // TODO: add write to error file  descriptor
          }
          
        }
        else{
            
          pid = fork();
          if (pid < 0){
            sprintf(print_buffer, "Error forking\n"); 
            write(2, print_buffer, strlen(print_buffer));
            // TODO: add write to error file  descriptor
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