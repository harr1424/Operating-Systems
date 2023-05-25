/*
Author: John Harrington
Completed for Operating Systems PA2A

This program expects as sole argument a filepath. The file at this path will be operated upon by this program according to the user input.
A user may either write to a file, read from a file, or seek to a specified offset inside of a file. User input is as follows:

r - Read a specified number of bytes from the file. The user will be prompted to enter how many bytes to read.
w - Write a string to the file. This string is limited to 1024 characters.
s - Call the lseek() system call and set the file offset as specified. See https://man7.org/linux/man-pages/man2/lseek.2.html

At any point the user may utilize the CTRL-D key combination to terminate the program and return an EXIT_SUCCESS status code.

If the user enters a character other than r, w, s, this will be ignored, the user will be informed that this was invalid input,
and they will be prompted for correct input.

If the user enters erroneous input when prompted for bytes, offset, whence, this is not handled, and may result in undefined behavior
or program termination.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

// This function will be called when a user specifies that they want to read a file
int read_file(int fd, int num_bytes)
{
    // dynamically allocate a buffer large enough to read the user-specified number of bytes from the file
    char *buffer;
    buffer = (char *)malloc(sizeof(char) * num_bytes);
    if (buffer == NULL)
    {
        printf("Unable to allocate %d bytes of memory\n", num_bytes);
        exit(EXIT_FAILURE);
    }

    ssize_t result = -1;
    result = read(fd, buffer, num_bytes);

    printf("The first %d bytes (after offset) contains:\n", num_bytes);
    puts(buffer);

    free(buffer);
    return result;
}

// This function will be called when a user specifies that they want to write to a file
int write_string_to_file(int fd, char *user_string)
{
    // dynamically allocate a buffer large enough to write the user-generated string to the file
    char *buffer;
    buffer = (char *)malloc(sizeof(char) * strlen(user_string));
    if (buffer == NULL)
    {
        puts("Unable to allocate write buffer");
        exit(EXIT_FAILURE);
    }
    strcpy(buffer, user_string);

    size_t result = -1;
    result = write(fd, buffer, strlen(user_string));
    if (result < 0)
    {
        printf("Unable to complete write operation on file.\n");
        exit(EXIT_FAILURE);
    }

    printf("Write operation completed with %zu bytes written.\n", result);

    free(buffer);
    return result;
}

// This function will be called when a user specifies that they want to seek to some offset in a file
int seek_to_position(int fd, int offset, int whence)
{
    int ret_offset = -1;
    int result;

    ret_offset = lseek(fd, offset, whence);

    return ret_offset;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    { // no filename specified, or multiple args provided
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *filename = argv[1];
    char mode = 'z';
    
    //open device file to test file operations 
    int fd;
    int close_status;
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Unable to open the file: '%s'. Try again with sudo privileges.\n", filename);
        exit(EXIT_FAILURE);
    }

    // while input is NOT CTRL-D
    while (!feof(stdin))
    {
        puts("Option (r for read, w for write, s for seek):");
        scanf("%c", &mode);
        getchar();

        if (mode == 'r')
        {
            int num_bytes = 0;
            puts("Enter the number of bytes you want to read:");
            scanf("%d", &num_bytes);
            getchar();

            read_file(fd, num_bytes);
            // return EXIT_SUCCESS;
        }

        else if (mode == 'w')
        {
            char user_string[1024];
            puts("Enter the string you want to write:");
            fgets(user_string, 1024, stdin);
            user_string[strlen(user_string) - 1] = '\0';

            write_string_to_file(fd, user_string);
            // return EXIT_SUCCESS;
        }

        else if (mode == 's')
        {
            int result = -1;

            off_t offset = -1;
            int whence = -1;

            puts("Enter an offset value:");
            scanf("%lld", &offset);
            getchar();

            puts("Enter a value for whence (0 for SEEK_SET, 1 for SEEK_CUR, 2 for SEEK_END):");
            scanf("%d", &whence);
            getchar();

            result = seek_to_position(fd, offset, whence);
            if (result < 0)
            {
                puts("lseek operation could not be completed");
                return EXIT_FAILURE;
            }

            // return EXIT_SUCCESS;
        }

        else
        {
            puts("Invalid selection");
        }
    }

    close_status = close(fd);
    if (close_status < 0)
    {
        printf("Error closing fd.\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}