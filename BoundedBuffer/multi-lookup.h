/*
AUTHOR JOHN HARRINGTON 

PROGRAMMING ASSIGNMENT 3 PART A

A header file used to implement multi-lookup
*/

#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#define MAX_INPUT_FILES 100
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MAX_NAME_LENGTH 255
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

/*
PRODUCER FUNCTION

Expects as argument a struct containing a pointer to a queue (fifo struct, see shared_array.h)
that already contains all the files to parse for hostnames, a pointer to a separate queue
used to store all hostnames themselves, and a file pointer to a resolution log file

This function will iteratively parse all supplied files line by line and enqueue
the hostname present on a line into a shared array. Continues until all files
have been parsed, and then exits.
*/
void *service_file(void *arguments);


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
void *resolve_addr(void *arguments);

#endif