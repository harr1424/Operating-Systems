/*
A program to demonstrate that a child process will continue to exist until
its parent calls wait(). Such a child process is known as a zombie.
*/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    pid_t child_pid = fork();

    if (child_pid > 0) sleep(60);

    else exit(0); // child process exits, parent did not wait

    return EXIT_SUCCESS; //parent process exits
}