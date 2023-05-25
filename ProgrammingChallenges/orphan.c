/*
This program is used to determine if a child process will continue to run  
after it's parent process has finished.
*/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    pid_t child_pid = fork();

    if (child_pid == 0) sleep(60);

    return EXIT_SUCCESS; //parent process exits
}