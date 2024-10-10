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
    return rtrim(ltrim(command_line));
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
    char *arguments[MAX_COMMAND_LINE_ARGS];
    char* command_line_copy; // for trimming command_line input string of trailing spaces
    char token_delimiter[] = " ";
    char * new_dir; // for cd
    
    	
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
            token = strtok(command_line_copy, token_delimiter); // get first instructions. for some reason, this modifies both command_line and command_line_copy by adding a /0 after first token???

            /*
                while (token) {
            printf("%s\n", token);
            token = strtok(NULL, token_delimiter);
        }
 
 */
            
            
            // TODO: parse command_line input into flags where necessary https://www.geeksforgeeks.org/string-tokenization-in-c/

            // TODO: rigorous testing of directory and testing commands implemented in the parent directory itself e.g. cd, pwd, exit, etc
            
            if (strcmp(token,"cd") == 0){ // TODO: implement more compled cd scenarios https://www.tutorialspoint.com/how-to-use-cd-command-in-bash-scripts
                new_dir = strtok(NULL, token_delimiter);
                if (chdir(new_dir) != 0) {
                    printf("%s is not a valid directory\n", new_dir); 
                }     
            }

            if (strcmp(token,"pwd") == 0){ // TODO: implement pwd flags
                printf("%s\n", current_dir);
            }

            if (strcmp(token,"echo") == 0){
              // TODO: implement more complex echo scenarios from here https://kodekloud.com/blog/bash-echo-commands-examples/
              /* TODO: follow instructions 
              to  implement echo, 
              write a loop that runs over 
              the command arguments to print 
              the string values.*/
              printf("%s\n", sliceString(command_line, strlen("echo "), strlen(command_line)));
            }
            if (strcmp(token,"exit") == 0){ // TODO: implement edge case of exit. ensure that if there are child processes, they end top
                return 0;
            }
            /* TODO: implement env
            To implement env, you need to access to 
            the global environment variables array. 
            To this end, add the following declaration 
            to your code: extern char **environ; 
            (see manual pages for environ) For example, 
            environ[0] contains the first variable 
            name=value pair, environ[1] contains 
            the second pair, and so on until you hit 
            a terminating NULL string.
            */
            // TODO: implement setenv
            
 
        }while(command_line[0] == 0x0A);  // while just ENTER pressed

      
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