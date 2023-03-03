#include  <stdio.h>
#include  <sys/types.h>
#include <string.h>
#define   MAX_COUNT  200

void  ChildProcess(void);                /* child process prototype  */
void  ParentProcess(void);               /* parent process prototype */

void  main(void)
{
    //  pid_t  pid;

    //  pid = fork();
    //  if (pid == 0) 
    //       ChildProcess();
    //  else 
    //       ParentProcess();
    char* str;
    long unsigned MAX_SIZE = 0; // to make the getline() function dynamically allocate space for us

    getline(&str, &MAX_SIZE, stdin);
    if(strcmp(str, "hey\n") == 0){
        printf("success");
    }
}

void  ChildProcess(void)
{
     int   i;

     for (i = 1; i <= MAX_COUNT; i++)
          printf("   This line is from child, value = %d\n", i);
     printf("   *** Child process is done ***\n");
}

void  ParentProcess(void)
{
     int   i;

     for (i = 1; i <= MAX_COUNT; i++){
          printf("This line is from parent, value = %d\n", i);
            wait();
     }
     printf("*** Parent is done ***\n");
}
