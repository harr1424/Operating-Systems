#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>

/*
This program tests a system call written to add two integers passed as arguments 
and stores the resulting sum in a user space memory address, also passed as a parameter. 

See add.c for system call implementation. 
*/

int main(void)
{
  int result = -1;
  int* result_p = &result;

  puts("Testing add()...");
  long exit_code = syscall(400, 2, 2, result_p); //csci460_add() was given table entry 400
  
  if (exit_code == 0) puts("Test passed.");
  else printf("Test FAILED with error code %ld\n", exit_code);

  printf("Result stored in user space memory: %d\n", result);

  return EXIT_SUCCESS;
}
