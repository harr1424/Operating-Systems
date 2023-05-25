/*
AUTHOR JOHN HARRINGTON

PROGRAMMING ASSIGNMENT 3 PART A

A header file used to implement data structures
and logic to solve the bounded buffer problem.
*/

#ifndef SHARED_ARRAY_H
#define SHARED_ARRAY_H

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_SIZE 10 // ARRAY_SIZE

/*
Declare a struct of type entry that is capable of
containing the address to resolve or filename.

This type will be used to create a dynamically allocated
array of entry structs.
*/
typedef struct
{
    char *address;
} entry;

/*
Declare a struct of type fifo that implements a FIFO
queue. Members front and end allow for ordinal operations,
and member buffer is simply an array of entry structs.

Aldo declared are three semaphores used to solve the bounded
buffer problem. See lecture 10C for pseudocode implementation.

This implementation ensures that the array of entry structs
will occupy a contiguous space in memory, which will be
dynamically allocated at run time.

The semaphores ensure thread safety, eliminate race conditions,
and avoid busy-waiting.
*/
typedef struct
{
    entry *buffer;
    int front, end;
    sem_t M, EMPTY, FULL;
    bool used;
} fifo;

/*
Used to pass multiple args to a start routine in pthread_create()
*/
typedef struct
{
    fifo *data_files;
    fifo *shared_array;
    FILE *req_log;
    pthread_mutex_t *err_lock;
    pthread_mutex_t *out_lock;
} req_arg_struct;

typedef struct
{
    fifo *shared_array;
    FILE *res_log;
    pthread_mutex_t *err_lock;
    pthread_mutex_t *out_lock;
} res_arg_struct;

int init_q(fifo *q);
int en_q(fifo *q, char *address);
char *de_q(fifo *q);
int de_init_q(fifo *q);
void print_q(fifo *q);

#endif