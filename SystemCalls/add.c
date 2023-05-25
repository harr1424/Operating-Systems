#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE3(csci460_add, int, number1, int, number2, int __user *, result)
{
  printk("Adding %d to %d:\n", number1, number2);

  int sum = number1 + number2;
  *result = sum;

  printk("%d\n", sum);

  return 0;
}