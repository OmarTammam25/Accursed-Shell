#include <stdio.h>
#include <stdlib.h>  
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <pwd.h>

#define TOKENIZERS " \t\n"
#define TOKEN_SIZE 300
#define BUILT_IN_COMMANDS_SIZE 4
#define MAX_DIRECTORY_PATH_SIZE 100
#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

// colors
#define AC_BLACK "\x1b[30m"
#define AC_RED "\x1b[31m"
#define AC_GREEN "\x1b[32m"
#define AC_YELLOW "\x1b[33m"
#define AC_BLUE "\x1b[34m"
#define AC_MAGENTA "\x1b[35m"
#define AC_CYAN "\x1b[36m"
#define AC_WHITE "\x1b[37m"
#define AC_NORMAL "\x1b[m"

char* builtInCommands[] = {
    "cd",
    "echo",
    "export",
    "exit"
};
enum commands {cd, echo, export, exitEnum};

void shell();
char* parseInput();
char** createArguments(char* userInput, char* tokenizers);
void dynamicAllocationError();
void evaluateExpression(char** userArguments);

void execute_command(char* userInput, char** userArguments);

void execute_shell_builtin(char* userInput, char** userArguments, enum commands c);
void command_cd(char* userArguments);
void command_echo(char** userArguments); 
void command_export(char** userArguments);

char** checkForSpace(char** args);


int main(){
    printf("\t\t\t%sWELCOME TO ACCURSED SHELL!%s\n\n", AC_MAGENTA, AC_NORMAL);
    shell();
}

void shell(){

    do
    {
        char* userInput;
        // signal (SIGCHLD, proc_exit);
        signal(SIGCHLD, SIG_IGN); // reaps zombie processes

        char directoryPath[MAX_DIRECTORY_PATH_SIZE];
        getcwd(directoryPath, (size_t)MAX_DIRECTORY_PATH_SIZE);
        printf("%s%s%s:$ ",AC_GREEN ,directoryPath, AC_NORMAL);

        userInput = parseInput();
        char** userArguments = createArguments(userInput, TOKENIZERS);
        evaluateExpression(userArguments);
        int isBuiltInCommand = 0; 
        for(int i = 0; i < BUILT_IN_COMMANDS_SIZE; i++){
            if(strcmp(userInput, builtInCommands[i]) == 0){
                isBuiltInCommand = 1;
                execute_shell_builtin(userInput, userArguments, i);
            }
        }
        if(!isBuiltInCommand)
            execute_command(userInput, userArguments);

    } while (1);
}

// takes input from user
char* parseInput(){
    char* userInput = NULL;
    long unsigned MAX_SIZE = 0; // to make the getline() function dynamically allocate space for us
    if(getline(&userInput, &MAX_SIZE, stdin) == -1) { // error happened while taking input
        perror("Couldn't parse input");
        exit(EXIT_FAILURE);
    }

    return userInput;
}

char** createArguments(char* userInput, char* tokenizers){
    int it = 0, currentDynamicSize = TOKEN_SIZE;
    char* token = strtok(userInput, tokenizers); // gets first token
    char** args = malloc(TOKEN_SIZE * sizeof(char*));

    if(!args)
        dynamicAllocationError();

    while (token != NULL)
    {
        args[it++] = token;

        // current size of args 2d array is not enough
        if(it >= TOKEN_SIZE){
            args = realloc(args, currentDynamicSize * 2 * sizeof(char*));
            currentDynamicSize *= 2;
            if(!args)
                dynamicAllocationError();
        }

        // gets next token
        token = strtok(NULL, tokenizers);         
    }
    return args;
}

void dynamicAllocationError(){
    perror("ALLOCATION ERROR");
    exit(EXIT_FAILURE);
}

void evaluateExpression(char** userArguments){
    for(int i = 1; userArguments[i] != NULL; i++){
        char* it = strchr(userArguments[i], '$');
        if(it != NULL){
            char* temp = userArguments[i];
            temp++;
            userArguments[i] = getenv(temp);
        }
    }
}

void execute_command(char* userInput, char** userArguments){
    pid_t pid = fork();
    if (pid < 0){
        perror("couldn't spawn child proccess");
        exit(EXIT_FAILURE);
    }else if(pid == 0){
        char** args = checkForSpace(userArguments);
        if(execvp(userInput, args) == -1){
            perror("Command not found");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }else if(!(userArguments[1] != NULL && userArguments[1][0] == '&')){
        int status;
        waitpid(pid, &status, WUNTRACED);
    }
}

void execute_shell_builtin(char* userInput, char** userArguments, enum commands c){
    switch (c)
    {
    case cd:
        command_cd(userArguments[1]);
        break;
    case echo:
        command_echo(userArguments);
        break;
    case export:
        command_export(userArguments);
        break;
    case exitEnum:
        exit(EXIT_SUCCESS);
        break; 
    default:
        perror("command not found");
        break;
    }
}

void command_cd(char* userArguments){
    if(!userArguments || strcmp(userArguments, "~") == 0){
        userArguments = getenv("HOME");
    }
    chdir(userArguments);
}

void command_echo(char** userArguments){
    int it = 1;
    while (userArguments[it] != NULL)
    {
        int j = 0;
        char* stringRefactored = malloc(sizeof(char) * TOKEN_SIZE);
        int loc = 0;
        while(userArguments[it][j] != '\0'){
            if(userArguments[it][j] != '\"')
                stringRefactored[loc++] = userArguments[it][j];
            j++;
        }
        stringRefactored[loc] = '\0';
        printf("%s ", stringRefactored);
        it++;
    }
    printf("\n");
}

void command_export(char** userArguments){
    if(userArguments[1] != NULL) {
        char newArgument[200];
        int i = 1;
        int loc = 0;
        int flag = 0;
        while (userArguments[i] != NULL)
        {
            int j = 0;
            while (userArguments[i][j] != '\0')
            {
                if(userArguments[i][j] == '=')
                    flag = 1;
                else if(flag && userArguments[i][j] != '\"'){
                    newArgument[loc++] = userArguments[i][j];
                }
                j++;
            }
            newArgument[loc++] = ' ';
            i++;
        }
        
        newArgument[loc-1]= '\0';
        char** myArguments = createArguments(userArguments[1], "=");
        setenv(myArguments[0], newArgument, 1);
    }
}

char** checkForSpace(char** args){
    int i = 0;
    char** args2 = malloc(TOKEN_SIZE * sizeof(char*));

    int loc2 = 0;
    while (args[i] != NULL)
    {
        int j = 0;
        char temp[200];
        int loc = 0;
        while (args[i][j] != '\0')
        {  
            if(args[i][j] == ' '){
                temp[loc++] = '\0';
                args2[loc2] = malloc(TOKEN_SIZE * sizeof(char));
                strcpy(args2[loc2++], temp);
                loc = 0;
            }else{
                temp[loc++] = args[i][j];
            }
            j++;
        }
        temp[loc] = '\0';
        args2[loc2] = malloc(TOKEN_SIZE * sizeof(char));
        strcpy(args2[loc2++], temp);
        i++;
    }
    return args2;
}


// void proc_exit()
// {
// 		int wstat;
// 		union wait wstat;
// 		pid_t	pid;

// 		while (1) {
// 			pid = wait3 (&wstat, WNOHANG, (struct rusage *)NULL );
// 			if (pid == 0)
// 				return;
// 			else if (pid == -1)
// 				return;
// 			else
// 				printf ("Return code: %d\n", wstat.w_retcode);
// 		}
// }