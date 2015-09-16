#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char *read_input(void)
{   //read in the input and return it as a string array
    
    char *input = NULL;
    size_t bsize = 0; //allocating buffer
    getline(&input, &bsize, stdin);
    
    return input;
}

#define MAX_LINE 80
#define TOK_DELIM " \t\r\n\a"
char **split_stuff(char *line, int *amp)
{   //parse the input
    
    int bufsize = MAX_LINE, index = 0;
    char **input = malloc(bufsize * sizeof(char*));
    char *word;

    if (!input) {
        // error
        exit(EXIT_FAILURE);
    }

    word = strtok(line, TOK_DELIM);
    while (word != NULL) {
        if (strcmp(word,"&") == 0) {
            // check if the user entered an ampersand, if so, increment
            // index without adding the character to the array
            amp[0] = 1;
            index++;
        }
        else {
            input[index] = word;
            index++;
        }
        if (index > bufsize) {
            input = realloc(word, bufsize * sizeof(char*)); // make more space in array!
            if (!input) {
                exit(EXIT_FAILURE);
            }
        }

        word = strtok(NULL, TOK_DELIM);
    }
    input[index] = NULL; // null pointer at end
    
    return input;
}


int execute(char ** args, int should_wait) {

    pid_t pid;

    pid = fork();
    
    if (pid == 0) {
        // child process, execute char ** command
            execvp(args[0],args);
            exit(0);

    } else if (pid < 0) {
        // error has occured
        return 1;

    } else if (pid > 0) {
        //parent
        
        if (!should_wait) {
            // wait if the user hasn't entered an '&'
            wait(NULL);
        }
    }
        return 0;
}


int check_input(char **args)
{
    //if exit, return 0 and end program (in loop)
    if (strcmp(args[0],"exit") == 0) {
        return 0;
    }
    return 1;
}


void
print_history(char **history, int count)
{
    if (count < 10) {

        for (int i = count-1; i >= 0; i--){
            printf(" %d %s\n", i, history[i]);
        }

    } else {
        
        int number = count-1;
        int index = 0;
        
        while (index < 10) {
            printf("%d %s\n", number, history[number%10]);
            number = number - 1;
            index += 1;
        }
    }
}


int main(void)
{
    int i;
    char *line;
    char **args;
    int status;
    int should_run = 1;
    
    //create the history
    char **history= (char **)malloc(sizeof(char *)*10);
    
    for(i=0;i<10;i++){
      history[i] = (char *)malloc(sizeof(char)*100);
    }
    int command_count = 0;
    // variable that will go into split_stuff
    char temp[100];
    
    int should_wait[1]; //create a should wait flag that will be 1 when user enters &
    int *sw;
    sw = should_wait;
    sw[0] = 0;

    while(should_run) {

        printf("osh> ");
        line = read_input();
        //strcpy so that line will stay the same
        strcpy(temp,line);
        args = split_stuff(temp, sw);
        
        if (strcmp(args[0], "!!") == 0){ //if !! is entered, repeat the last command
           
           strcpy(history[(command_count)%10],history[command_count-1%10]);
           status = execute(split_stuff(history[command_count%10-1], sw),sw[0]);
           ++command_count;
        
        }

        else if (args[0][0] == '!') {
            // check if next character is also !
            // if it is a number n, execute the nth command
            int number;
            char *string = args[0];
            char *newstring = string + 1;
            number = atoi(newstring);
            
            if (number > command_count || number < 0) {
                
                printf("No such command in history.\n");
            
            }
            else {
                //add command back to history and execute it, incrementing command_count
                number = (number)%10; //mod to access location in history
                strcpy(history[(command_count+1)%10],history[command_count%10]);
                status = execute(split_stuff(history[number%10], sw),sw[0]);
                ++command_count;
            
                }
        } 
        else if (strcmp(args[0],"history") == 0) {
            // check if history is empty. if not, print it
            if (command_count == 0) {
                printf("No commands in history.\n");
            }
            
            print_history(history, command_count);
        } 
        else {
            // Now line goes in just as it was entered
            // if line isn't "exit", execute it and add it to the history books
            strcpy(history[command_count%10],line);
            ++command_count;
            should_run = check_input(args);
            
            if (should_run) {
                status = execute(args,sw[0]);
            }
        }
    } 
        return 0;
}
