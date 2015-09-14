#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length command */

char *read_line(void)
{   //read in the input and return it as a string array(??)
    
    char *line = NULL;
    size_t bufsize = 0; //allocating buffer
    getline(&line, &bufsize, stdin);
    
    return line;
}

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
char **split_line(char *line)
{   //parse the input
    
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position > bufsize) {
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    
    return tokens;
}


int start_shell(char ** args)
{   //make the child process and all that
    
    pid_t pid, wpid;
    int status;
    

    pid = fork();
    if (pid == 0) {
        // child process
        printf("args[0]: %s\n",args[0]);
        
        if (execvp(args[0], args) == -1) {
            perror("osh");
        }

        exit(EXIT_FAILURE);
    
    } else if (pid < 0) {
        // forking error
        
        perror("osh");
    
    } else {
        // parent process
        
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    
    }

    return 1;
}

int execute(char **args)
{
    //just calls the forking function thing
    //should probably eliminate this function
    return start_shell(args);
}

int check_input(char **args)
{
    //if exit, return 0 and end program (in loop)
    if (strcmp(args[0],"exit") == 0) {
        return 0;
    }
    return 1;
}

char **check_history(int count, char **commands, char *command)
{   //add commands to history of commands
    
    //count = count%10; //array only stores 10 values so use mod
    commands[count%10] = command;
    
    printf("command history:\n"); //print just to check what's going on
    if (count < 10) {
        for (int i=0;i<(count%10);i++){
            printf("%s\n at index %d\n",commands[i],i);
        }
    } else {
        for (int i=0;i<10;i++){
            printf("printing because count is greater than 10.");
            printf("%s\n at index %d\n",commands[i],i);
        }
    } 
    
    printf("before error\n");//tryna see where things are going wrong
    
    return commands;
}

void loop(void)
{
    char *line = malloc(MAX_LINE);
    char **args;
    int status;
    int should_run;
    char **history = (char **)malloc(sizeof(char **)*10);
    int command_count = 0;

    do {
        //this is a clusterfuck

        printf("osh> ");
        line = read_line();
        printf("%s\n",line);
        args = split_line(line);

        if (strcmp(args[0], "!!") == 0){ //if !! is entered, repeat the last command
            
            printf("entered !!\n");
            printf("%d\n",command_count);
            //args = split_line(history[command_count%10]);
            printf("command at %d is %s\n",(command_count%10)-1,history[command_count%10-1]);
            char **temp = split_line(history[command_count%10-1]);
            //THIS IS WHERE EVERYTHING IS FUCKING UP
            //status = execute(split_line(history[command_count%10-1]));
            printf("split line\n");
            status = execute(temp);
        
        } else {
            
            history = check_history(command_count, history, line);
            command_count += 1;
            should_run = check_input(args);
            status = execute(args);
            
            if (should_run == 0) {
                //break if exit has been entered
                break;
            }

        }
    } while (status && should_run);
}


int main()
{
    loop();
    return EXIT_SUCCESS;
}
