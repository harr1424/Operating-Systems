/*
A program to measure the execution time in milliseconds of another program.
The program to time is passed as the only expected command line argument.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        puts("Usage: ./time <command_to_time>");
        return EXIT_FAILURE;
    }

    // FIXME pass default args to timed command 
    char *args[2] = {".", NULL};

    printf("Timing %s with default argument: %s\n", argv[1], args[0]);

    clock_t begin = clock();

    int pid = fork();

    if (pid == 0) // child process -> execute passed command
    {
        execvp(argv[1], args);
    }

    else if (pid > 0) // parent process -> wait for child process to complete with 0 exit code
    {
        int status;
        waitpid(pid, &status, 0);
        clock_t end = clock();
        double exec_time = (double) (end - begin) / CLOCKS_PER_SEC;
        printf("\n%s took %f seconds to fork, execute, and complete.\n", argv[1], exec_time);
    }

    else
    {
        fprintf(stderr, "Error: fork() did not succeed");
    }

    return EXIT_SUCCESS;
}
