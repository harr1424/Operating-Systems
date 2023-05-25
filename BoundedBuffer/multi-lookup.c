#include "multi-lookup.h"
#include "shared_array.h"
#include "util.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>

/*
PRODUCER FUNCTION

Expects as argument a struct containing a pointer to a queue (fifo struct, see shared_array.h)
that already contains all the files to parse for hostnames, a pointer to a separate queue
used to store all hostnames themselves, and a file pointer to a resolution log file

This function will iteratively parse all supplied files line by line and enqueue
the hostname present on a line into a shared array. Continues until all files
have been parsed, and then exits.
*/
void *service_file(void *arguments)
{
    req_arg_struct *args = arguments;
    int files_serviced = 0;
    FILE *data_file;
    char *line = NULL; // lineptr buffer passed to getline()  -- dynamically modified in getline
    size_t len = 0;    // length of lineptr buffer -- dynamically modified in getline()
    ssize_t read;      // length of bytes read by getline()
    char *saved;       // used to enqueue an address at a unique memory address

    char *curr_data_file = de_q(args->data_files); // get first address name in queue
    while (strcmp(curr_data_file, "empty") != 0)   // as long as the queue is not empty
    {
        data_file = fopen(curr_data_file, "r");
        if (data_file == NULL)
        {
            pthread_mutex_lock(args->err_lock);
            fprintf(stderr, "Unable to open file %s\n", curr_data_file);
            pthread_mutex_unlock(args->err_lock);

            curr_data_file = de_q(args->data_files);
            continue; // this is error is considered recoverable, don't use bad pointer, but try next file
        }

        // while getline() returns a valid char*
        while ((read = getline(&line, &len, data_file)) != -1)
        {
            if (read <= MAX_NAME_LENGTH)
            {
                // remove trailing newline
                for (int i = 0;; i++)
                {
                    if (line[i] == '\n')
                    {
                        line[i] = '\0';
                        break;
                    }
                }

                saved = malloc(MAX_NAME_LENGTH);
                strcpy(saved, line);

                en_q(args->shared_array, saved); // enqueue the address as char *saved

                pthread_mutex_lock(args->out_lock);
                fprintf(args->req_log, "Added %s for resolution\n", saved);
                pthread_mutex_unlock(args->out_lock);
            }
            else
            {
                pthread_mutex_lock(args->out_lock);
                printf("Skipping %s: Address name length must not exceed %d\n", line, MAX_NAME_LENGTH);
                fprintf(args->req_log, "Skipping %s: Address name length must not exceed %d\n", line, MAX_NAME_LENGTH);
                pthread_mutex_unlock(args->out_lock);
            }
        }

        fclose(data_file);
        files_serviced++;
        curr_data_file = de_q(args->data_files); // get next file
    }

    free(line);

    pthread_mutex_lock(args->out_lock);
    printf("thread %lud serviced %d files\n", pthread_self(), files_serviced);
    pthread_mutex_unlock(args->out_lock);

    pthread_exit(NULL);
}

/*
CONSUMER FUNCTION

Expects as argument a struct containing a pointer to the shared array (queue) used to store address names
and a pointer to a requester log file.

Dequeues address names from the shared array and passes these hostnames to the dnslookup()
function declared in util.h and defined in util.c

If the DNS lookup was successful, writes the original host address name and ip address to the resolver log file.
If the lookup failed, writes the original host address name and "NOT RESOLVED" to the resolver log file.

Continues until the queue is empty, and then exits.
*/
void *resolve_addr(void *arguments)
{
    res_arg_struct *args = arguments;
    int num_hosts = 0;
    char *curr_address = de_q(args->shared_array);

    while (strcmp(curr_address, "empty") != 0)
    {
        char ip_addr[MAX_IP_LENGTH];
        int lookup_res = dnslookup(curr_address, ip_addr, MAX_IP_LENGTH);

        if (lookup_res == 0)
        {
            pthread_mutex_lock(args->out_lock);
            fprintf(args->res_log, "%s, %s\n", curr_address, ip_addr);
            pthread_mutex_unlock(args->out_lock);
        }

        else
        {
            pthread_mutex_lock(args->out_lock);
            fprintf(args->res_log, "%s, NOT_RESOLVED\n", curr_address);
            pthread_mutex_unlock(args->out_lock);
        }

        free(curr_address);
        num_hosts++;
        curr_address = de_q(args->shared_array);
    }

    pthread_mutex_lock(args->out_lock);
    printf("thread %lud resolved %d hostnames\n", pthread_self(), num_hosts);
    pthread_mutex_unlock(args->out_lock);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // structs for measuring runtime
    struct timeval tv_start;
    struct timeval tv_end;

    // variables local to main() declared
    int num_req = 0; // number of requester threads
    int num_res = 0; // number of resolver threads

    FILE *req; // points to requester log file
    FILE *res; // points to resolver log file

    // mutex locks for IO write operations
    pthread_mutex_t stderr_lock, stdout_lock;

    // begin measuring runtime
    gettimeofday(&tv_start, 0);

    // initialize synchronization mechanisms 
    pthread_mutex_init(&stderr_lock, NULL);
    pthread_mutex_init(&stdout_lock, NULL);

    // if too few args passed
    if (argc < 6)
    {
        puts("Usage: ./multi-lookup <# requesters> <# resolvers> <requester log> <resolver log> [ <data file> ... ]");
        return EXIT_FAILURE;
    }

    // check and handle arg values
    if (argv[1] != NULL)
    {
        int temp = atoi(argv[1]);
        if (temp > 0 && temp <= MAX_REQUESTER_THREADS)
        {
            num_req = temp;
        }
        else
        {
            perror("Number of requester/resolver threads must be between 1 and 10");
            return EXIT_FAILURE;
        }
    }
    else
    {
        perror("Invalid argument passed for number of requester threads");
        puts("Usage: ./multi-lookup <# requesters> <# resolvers> <requester log> <resolver log> [ <data file> ... ]");
        return EXIT_FAILURE;
    }

    if (argv[2] != NULL)
    {
        int temp = atoi(argv[2]);
        if (temp > 0 && temp <= MAX_RESOLVER_THREADS)
        {
            num_res = temp;
        }
        else
        {
            perror("Number of requester/resolver threads must be between 1 and 10");
            return EXIT_FAILURE;
        }
    }
    else
    {
        perror("Invalid argument passed for number of resolver threads");
        puts("Usage: ./multi-lookup <# requesters> <# resolvers> <requester log> <resolver log> [ <data file> ... ]");
        return EXIT_FAILURE;
    }

    if (argv[3] != NULL)
    {
        req = fopen(argv[3], "w");
        if (req == NULL)
        {
            perror("Unable to open resolver log file as specified: ");
            return EXIT_FAILURE;
        }
    }
    else
    {
        perror("Please enter a resolver log filename");
        return EXIT_FAILURE;
    }

    if (argv[4] != NULL)
    {
        res = fopen(argv[4], "w");
        if (req == NULL)
        {
            perror("Unable to open requester log file as specified: ");
            return EXIT_FAILURE;
        }
    }
    else
    {
        perror("Please enter a requester log filename");
        return EXIT_FAILURE;
    }

    // check number of data files is within limits
    if (argc - 5 > MAX_INPUT_FILES)
    {
        perror("The maximum number of data files is 100");
        return EXIT_FAILURE;
    }
    if (argc - 5 == 0)
    {
        perror("You must enter at least one data file");
        return EXIT_FAILURE;
    }

    // create shared_array data structure for filenames
    fifo files;
    init_q(&files);
    // use argc - 5 to determine how many files passed
    for (int i = 5; i < argc; i++)
    {
        en_q(&files, argv[i]); // en_q each file (sync mechanisms defined in shared_array.c)
    }

    // create shared_array data structure for addresses
    fifo shared_array;
    init_q(&shared_array);

    // instantiate arg_structs for calls to pthread_create()
    req_arg_struct req_args = {.data_files = &files, .shared_array = &shared_array, .req_log = req, .err_lock = &stderr_lock, .out_lock = &stdout_lock};
    res_arg_struct res_args = {.shared_array = &shared_array, .res_log = res, .err_lock = &stderr_lock, .out_lock = &stdout_lock};

    // create requester and resolver threads
    pthread_t req_pool[num_req];
    pthread_t res_pool[num_res];

    for (int i = 0; i < num_req; i++)
    {
        if (pthread_create(&req_pool[i], NULL, &service_file, (void *)&req_args))
        {
            pthread_mutex_lock(&stderr_lock);
            perror("Unable to create thread, terminating");
            pthread_mutex_unlock(&stderr_lock);

            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < num_res; i++)
    {
        if (pthread_create(&res_pool[i], NULL, &resolve_addr, (void *)&res_args))
        {
            pthread_mutex_lock(&stderr_lock);
            perror("Unable to create thread, terminating");
            pthread_mutex_unlock(&stderr_lock);

            return EXIT_FAILURE;
        }
    }

    // wait for threads to complete
    for (int i = 0; i < num_req; i++)
    {
        pthread_join(req_pool[i], NULL);
    }

    for (int i = 0; i < num_res; i++)
    {
        pthread_join(res_pool[i], NULL);
    }

    // clean up
    fclose(req);
    fclose(res);
    de_init_q(&files);
    de_init_q(&shared_array); // this frees the buffer variable inside the queue, so why isn't all memory freed?

    // complete runtime measurement and output result
    gettimeofday(&tv_end, 0);
    long elapsed = (tv_end.tv_sec - tv_start.tv_sec);
    printf("%s: total time is %ld seconds\n", argv[0], elapsed);

    return EXIT_SUCCESS;
}
