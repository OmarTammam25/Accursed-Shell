#include <stdio.h>
#include <stdlib.h>  
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>

#define TOKENIZERS " \t\n"
#define TOKEN_SIZE 300
#define BUILT_IN_COMMANDS_SIZE 4
#define MAX_DIRECTORY_PATH_SIZE 100
#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))


char* builtInCommands[] = {
    "cd",
    "echo",
    "export",
    "exit"
};
enum commands {cd, echo, export, exitEnum};

// takes input from user
char* parseInput(){
    char* userInput = NULL;
    long unsigned MAX_SIZE = 0; // to make the getline() function dynamically allocate space for us
    if(getline(&userInput, &MAX_SIZE, stdin) == -1) // error happened while taking input
            exit(EXIT_FAILURE);

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
    printf("ALLOCATION ERROR\n");
    exit(EXIT_FAILURE);
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
        printf("exit called");
        break;
    default:
        printf("command not found\n");
        break;
    }
}
void command_cd(char* userArguments){
    if(!userArguments || strcmp(userArguments, "~") == 0){
        userArguments = getenv("HOME");
    }
    chdir(userArguments);
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
        // char** temp = createArguments(myArguments[1], "\"");
        char** myArguments = createArguments(userArguments[1], "=");
        setenv(myArguments[0], newArgument, 1);
    }
    // if(myArguments[1] != NULL){
    //     int loc = 0;
    //     int i = 1;
    //     while(myArguments[i] != NULL){
    //         int j = 0;
    //         while(myArguments[i][j] != NULL){
    //             if(myArguments[i][j] != '\"'){
    //                 newArgument[loc++] = myArguments[i][j];
    //             }
    //             j++;
    //         }
    //         i++;
    //     }
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

void execute_command(char* userInput, char** userArguments){
    pid_t pid = fork();
    // printf("HEYEYYYY %s", userArguments[1]);
    if (pid < 0){
        printf("Error occurred\n");
        exit(EXIT_FAILURE);
    }else if(pid == 0){
        char** args = checkForSpace(userArguments);
        execvp(userInput, args);
        exit(EXIT_SUCCESS);
    }else{
        int status;
        waitpid(pid, &status, WUNTRACED);
    }
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

void shell(){
    char* userInput;

    do
    {
        char directoryPath[MAX_DIRECTORY_PATH_SIZE];
        getcwd(directoryPath, (size_t)MAX_DIRECTORY_PATH_SIZE);
        printf("%s: ", directoryPath);

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


int main(){
    printf("\t\t\tWELCOME TO ACCURSED SHELL!\n");
    shell();
}