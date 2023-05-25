#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>

/*
This program tests a hello world style system call. 
If the test succeeds, "hello world" will be printed to 
KERN_ALERT, which can be verified by executing the dmesg command.
*/

int main(void)
{
  puts("Testing sys_helloworld()...");
  long int result = syscall(555); //sys_helloworld() was given table entry 555
  
  if (result == 0) puts("Test passed.");
  else puts("Test FAILED.");

  return EXIT_SUCCESS;
}
