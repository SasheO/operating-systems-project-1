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

int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
    char current_dir[MAX_COMMAND_LINE_LEN];
    char token_delimiter[] = " ";
    char *target_directory;
    char * environment_variable,environment_variable_value;
  
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];
    	
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

        int i = 0;
        char *p = strtok (command_line, token_delimiter);

        while (p != NULL)
        {
            arguments[i++] = trimString(p);
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
            environment_variable = getenv(arguments[1]);
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
            environment_variable_value = strchr(arguments[i], '=');
          }
          
        }
        // TODO:
        // 
        
			  // 0. Modify the prompt to print the current working directory
			  
			
        // 1. Tokenize the command line input (split it on whitespace)

      
        // 2. Implement Built-In Commands
      
    
        // 3. Create a child process which will execute the command line input

  
        // 4. The parent process should wait for the child to complete unless its a background process
      
      
        // Hints (put these into Google):
        // man fork
        // man execvp
        // man wait
        // man strtok
        // man environ
        // man signals
        
        // Extra Credit
        // man dup2
        // man open
        // man pipes
    }
    // This should never be reached.
    return -1;
}