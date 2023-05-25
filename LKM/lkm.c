/*
Author: John Harrington

This code comprises a loadable kernel module (LKM) that allows for performing operations on a
character device file.

Note that this device file is not created automatically, and must be created by running
the following command:

sudo mknod -m 777 /dev/pa_two_b c 460 0

Once the device file has been created, it is necessary to compile this LKM and load it
using the command:

sudo insmod pa2b.ko

If successful, a message will be printed informing the user of the success. The LKM can then be
tested using the binary compiled from pa2test.c, submitted for part A.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

MODULE_AUTHOR("JOHN HARRINGTON");
MODULE_LICENSE("GPL");

// set fixed size for device buffer to be used in module
const int buffer_size = 1024;

// declare device_buffer, will be instantiated in a call to kmalloc()
void *device_buffer;

// displays output indicating how many times the method has been called
int _open(struct inode *fnode, struct file *fstate)
{
    static int num_opens = 0;
    num_opens++;
    printk(KERN_ALERT "Device 'pa_two_b' has been opened %d times.\n", num_opens);
    return 0;
}

// displays output indicating how many times the method has been called
int _close(struct inode *fnode, struct file *fstate)
{
    static int num_close = 0;
    num_close++;
    printk(KERN_ALERT "Device 'pa_two_b' has been closed %d times.\n", num_close);
    return 0;
}

/*
Attempts to read length number of chars/bytes from the device buffer to user space buffer and updates the
current position of `device_buffer` to the end of the data that was read in.

Returns the number of bytes that were actually read.
*/
ssize_t _read(struct file *pfile, char __user *user_buffer, size_t length, loff_t *offset)
{
    unsigned long bytes_not_copied;
    unsigned long bytes_copied;

    if (*offset > buffer_size) // if offset has exceeded file size
    {
        printk(KERN_ALERT "You are attempting to read outside the bounds of the character device file! Program will terminate.\n");
        return -1;
    }

    else if (*offset + length > buffer_size) // if offset will exceed file size during requested operation
    {
        printk(KERN_ALERT "You have attempted to read one or more bytes beyond the end of character device file. Only bytes contained in the file will be read.\n");
        length = buffer_size - *offset;

        bytes_not_copied = copy_to_user(user_buffer, device_buffer + *offset, length);
        bytes_copied = length - bytes_not_copied;

        *offset += bytes_copied;

        printk(KERN_ALERT "Read %lu actual bytes of %zd bytes requested. *offset is at %lld and pfile->fpos is at %lld.\n", bytes_copied, length, *offset, pfile->f_pos);
        return bytes_copied;
    }
    else
    {
        bytes_not_copied = copy_to_user(user_buffer, device_buffer + *offset, length);
        bytes_copied = length - bytes_not_copied;

        *offset += bytes_copied;

        printk(KERN_ALERT "Read %lu actual bytes of %zd bytes requested. *offset is at %lld and pfile->fpos is at %lld.\n", bytes_copied, length, *offset, pfile->f_pos);
        return bytes_copied;
    }
}

/*
Writes length number of chars/bytes from user buffer to device buffer and updates the
current position of `device_buffer` to the end of the data that was read.

Returns the number of bytes that were actually written.
*/
ssize_t _write(struct file *pfile, const char __user *user_buffer, size_t length, loff_t *offset)
{
    unsigned long bytes_not_copied;
    unsigned long bytes_copied;

    if (*offset > buffer_size) // if offset has exceeded file size
    {
        printk(KERN_ALERT "You are attempting to write outside the bounds of the character device file! Program will terminate.\n");
        return -1;
    }

    else if (*offset + length > buffer_size) // if offset will exceed file size during requested operation
    {
        printk(KERN_ALERT "You have attempted to write one or more bytes beyond the end of character device file. Bytes will only be written to the end of the file, not beyond.\n");
        length = buffer_size - *offset;

        bytes_not_copied = copy_from_user(device_buffer + *offset, user_buffer, length);
        bytes_copied = length - bytes_not_copied;

        *offset += bytes_copied;

        printk(KERN_ALERT "Wrote %lu actual bytes of %zd bytes requested. *offset is at %lld and pfile->fpos is at %lld.\n", bytes_copied, length, *offset, pfile->f_pos);
        return bytes_copied;
    }
    else
    {
        bytes_not_copied = copy_from_user(device_buffer + *offset, user_buffer, length);
        bytes_copied = length - bytes_not_copied;

        *offset += bytes_copied;

        printk(KERN_ALERT "Wrote %lu actual bytes of %zd bytes requested. *offset is at %lld and pfile->fpos is at %lld.\n", bytes_copied, length, *offset, pfile->f_pos);
        return bytes_copied;
    }
}

/*
Used to update the current position of `device_buffer` depending upon the value of offset and whence.

Returns the updated offset.
*/
loff_t _seek(struct file *pfile, loff_t offset, int whence)
{
    loff_t position;

    switch (whence)
    {
    case 0: // SEEK_SET
        position = offset;
        break;

    case 1: // SEEK_CUR
        position = pfile->f_pos + offset;
        break;

    case 2: // SEEK_END
        position = buffer_size + offset;
        break;
    }

    if (position < 0 || position > (loff_t)buffer_size)
    {
        printk(KERN_ALERT "Bounds check failed on seek operation! Program will terminate.\n");
        return -1;
    }

    pfile->f_pos = position;

    printk(KERN_ALERT "Updated offset to %lld.\n", position);
    return position;
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = _open,
    .release = _close,
    .read = _read,
    .write = _write,
    .llseek = _seek};

int pa2b_init(void)
{
    int register_status = -1;

    device_buffer = kmalloc(buffer_size, GFP_KERNEL);

    // attempt to register the character device
    register_status = register_chrdev(460, "pa_two_b", &fops); // sudo mkmod -m 777 /dev/pa_two_b c 460 0
    if (register_status != 0)
    {
        printk(KERN_ALERT "Unable to register character device!\n");
        return -1;
    }

    printk(KERN_ALERT "module_init() called: Loaded programming assignment 2 part b kernel module.\n");
    return 0;
}

void pa2b_exit(void)
{
    kfree(device_buffer);
    unregister_chrdev(460, "pa_two_b"); // sudo mkmod -m 777 /dev/pa_two_b c 460 0
    printk(KERN_ALERT "module_exit() called: Unloaded programming assignment 2 part b kernel module.\n");
}

module_init(pa2b_init);
module_exit(pa2b_exit);
