#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
This program copies the contents of a file at one location to another location. 
If the destination file specified does not exist, it will be created. 
*/

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        puts("Usage: ./FileCopy <source_file> <dest_file>");
    }

    int fd_source, fd_dest;
    char buf[4096];
    size_t bytes_read;

    const char *source = argv[1];
    const char *dest = argv[2];

    /*
    open() returns a file descriptor, a small, nonnegative integer that is an index
    to an entry in the process's table of open file descriptors
    */
    fd_source = open(source, O_RDONLY);
    if (fd_source < 0)
    {
        fprintf(stderr, "Failed to open source file: %s\n", source);
        return EXIT_FAILURE;
    }

    fd_dest = open(dest, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_dest < 0)
    {
        fprintf(stderr, "Failed to open or create destination file:%s\n", dest);
        return EXIT_FAILURE;
    }

    /*
    On success, the number of bytes read is returned (zero indicates
    end of file), and the file position is advanced by this number.
    */
    while (bytes_read = read(fd_source, buf, sizeof buf), bytes_read > 0)
    {
        char *out_ptr = buf;
        size_t bytes_written;

        do
        {
            bytes_written = write(fd_dest, out_ptr, bytes_read);

            if (bytes_written >= 0)
            {
                bytes_read -= bytes_written;
                out_ptr += bytes_written;
            }
        } while (bytes_read > 0);
    }

    if (bytes_read == 0)
    {
        if (close(fd_dest) < 0)
        {
            fd_dest = -1;
            fprintf(stderr, "Error completing write to destination file: %s\n", dest);
            return EXIT_FAILURE;
        }

        close(fd_source);
        return EXIT_SUCCESS;
    }

    close(fd_source);
    if (fd_dest >= 0)
        close(fd_dest);
    return EXIT_FAILURE;
}