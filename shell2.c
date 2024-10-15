/*
    Author: Mezisashe Ojuba
    Written: 
*/


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
char current_dir[MAX_COMMAND_LINE_LEN];
char delimiters[] = " \t\r\n";
extern char **environ;
// Stores the string typed into the command line.
char command_line[MAX_COMMAND_LINE_LEN];
char* token;
char cmd_bak[MAX_COMMAND_LINE_LEN];

// TODO: cite trim, ltrim and rtrim from https://stackoverflow.com/questions/656542/trim-a-string-in-c
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

// TODO: cite startsWith https://stackoverflow.com/questions/15515088/how-to-check-if-string-starts-with-certain-string-in-c
bool startsWith(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
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

int main() { 
    // Stores the tokenized command line input.
    int i; // for for loops
    char arguments[MAX_COMMAND_LINE_ARGS][MAX_COMMAND_LINE_LEN];
    char* command_line_copy; // for trimming command_line input string of trailing spaces
    char token_delimiter[] = " ";
    char * current_arg; 
    char * current_arg2; // for cd
    char * command;
    char equals_sign_char = '=';
    char * commands_to_not_tokenize [] = { "export"}; // commands to not tokenize because tokenizing by spacing will cause issues
    
    	
    while (true) {
        do{ 
            // Print the shell prompt.
            getcwd(current_dir, MAX_COMMAND_LINE_LEN);
            printf("%s%s", current_dir, prompt);
            fflush(stdout);

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
            
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }

            
            command_line_copy = trimString(command_line); 
            strcpy(command_line, command_line_copy);
            command = strtok(command_line_copy, token_delimiter); // get first instructions. for some reason, this modifies both command_line and command_line_copy by adding a /0 after first token???
            current_arg = strtok(NULL, token_delimiter);
            
            for (i=0; i<sizeof(commands_to_not_tokenize)/sizeof(char*); i++){
                if (strcmp(command, commands_to_not_tokenize[i])==0){
                    i=-1;
                    break;
                }
            }

            if (i==-1){
                i = 0;
                    while (current_arg) {
                        strcpy(arguments[i], current_arg);
                        i++;
                        current_arg = strtok(NULL, token_delimiter);
                    }
                    strcpy(arguments[i], "\0");
            }
            
            
            // TODO: rigorous testing of directory and testing commands implemented in the parent directory itself e.g. cd, pwd, exit, etc
            
            /*
            setenv: sets an environment variable
            */
            if (strcmp(command,"cd") == 0){ // TODO: implement more complex cd scenarios https://www.tutorialspoint.com/how-to-use-cd-command-in-bash-scripts
                for (i=0; strcmp(arguments[i],"\0") != 0; i++){ // get directory from arguments
                    if (!startsWith(arguments[i], "-")){
                        current_arg = arguments[i];
                        break;
                    }
                }
                if (!startsWith(current_arg, "-")){ // if not a flag
                    if (chdir(current_arg) != 0) { // try changing directory
                        printf("%s is not a valid directory\n", current_arg);
                    }
                
                }
                else{
                    // deal with flags
                }
            }

            else if (strcmp(command,"pwd") == 0){ // TODO: implement pwd flags/more complex scenarios
                printf("%s\n", current_dir);
            }

            else if (strcmp(command,"echo") == 0){
              // TODO: implement more complex echo scenarios from here https://kodekloud.com/blog/bash-echo-commands-examples/
              token = strtok(NULL, token_delimiter);
              for (i=0; strcmp(arguments[i],"\0") != 0; i++){
                printf("%s ", arguments[i]);
              }
              printf("\n");
              
            }
            else if (strcmp(command,"exit") == 0){ // TODO: implement edge case of exit. ensure that if there are child processes, they end too
                return 0;
            }
            else if (strcmp(command,"env") == 0){ // TODO: implement env flags
                for (i=0; environ[i]!=NULL; i++) {
                    printf("%d: %s\n", i, environ[i]);
                }
            }
            // TODO: implement setenv
            else if (strcmp(command,"export") == 0){ // TODO: implement flags for export (set environment variables). 
            // TODO: export VAR="my value" can have spaces, meaning the tokenization is tricky here. how to do this?
                // current_arg = strchr(arguments[0], "=" );
                // printf("%s", arguments[0]);
                // printf("%s\n", current_arg);
                // check if one and only one = is in next token, export
            //     for (i=0; strcmp(arguments[i],"\0") != 0; i++){
            //     printf("%s\n", arguments[i]);
            //   }


            command = sliceString(command_line_copy, 0, strlen(command));
            command_line_copy[strlen(command_line_copy)] = ' ';
            // current_arg = trimString(sliceString(command_line_copy, strlen(command), strlen(command_line_copy)));
            // putenv(current_arg);

            current_arg = strchr(command_line_copy, ' ');
            current_arg2 = strchr(command_line_copy, '=');
            i = strlen(command_line_copy) - strlen(current_arg2)+1;
            int b=strlen(command_line_copy) - strlen(current_arg)+1;
            current_arg = trimString(sliceString(command_line_copy, b,i-2));
            current_arg2 = trimString(sliceString(command_line_copy, i, strlen(command_line_copy)));
            i = setenv(current_arg, current_arg2, 1);
            if (i!=0){
              printf("Error occured: %d\n", i);
            }
            }
 
        } while(command_line[0] == 0x0A);  // while just ENTER pressed

      
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
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