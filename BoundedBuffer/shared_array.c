/*
AUTHOR JOHN HARRINGTON

PROGRAMMING ASSIGNMENT 3 PART A

Implementation of data structures
and logic to solve the bounded buffer problem.
*/

#include "shared_array.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
Intended to be called once by the calling program.

Expects as sole argument an address of a fifo struct.

Dynamically allocates an array of entry structs,
the size of which is defined in shared_array.h.

Initializes the front and end members.

Initializes the semaphore members.

Returns 0 on success.
*/
int init_q(fifo *q)
{
    q->buffer = malloc(BUFFER_SIZE * sizeof(entry));
    q->front = -1;
    q->end = -1;
    q->used = false;
    sem_init(&q->M, 0, 1);
    sem_init(&q->EMPTY, 0, BUFFER_SIZE);
    sem_init(&q->FULL, 0, 0);

    return 0;
}

/*
Adds an address to the queue.

Expects two arguments:
1. Address of fifo struct
2. A c string representing the address to enqueue

A new entry struct will be created and added to the end
of the queue.

Returns 0 on success.

*/
int en_q(fifo *q, char *address)
{
    sem_wait(&q->EMPTY); // decrements semaphore EMPTY beginning at BUFFER_SIZE, if 0 waits for item to be removed
    sem_wait(&q->M);     // decrements M, enforcing mutual exclusion on the following CIS

    if (q->front == -1 && q->end == -1) // fifo is empty
    {
        q->front = 0;
        q->end = 0;

        entry new;
        new.address = address;

        q->buffer[q->end] = new;
    }
    else
    {
        q->end = (q->end + 1) % BUFFER_SIZE;

        entry new;
        new.address = address;

        q->buffer[q->end] = new;
    }
    q->used = true;
    sem_post(&q->M);    // increments M, allowing blocked threads to access CIS
    sem_post(&q->FULL); // increments FULL beginning at 0

    return 0;
}

/*
Used to return the entry address stored at the front of the
queue.

Expects as sole argument an address of a fifo struct.
*/
char *de_q(fifo *q)
{
    while (!q->used); // ensure queue has been pushed to one time 

    sem_wait(&q->M); // enforce mutex on CIS

    if ((q->front == -1) && q->end == -1) // fifo is empty
    {
        sem_post(&q->M); // release mutex on CIS
        return "empty";
    }

    sem_wait(&q->FULL); // decrements FULL as an item is removed, if 0 waits for an item to be added

    if (q->front == q->end) // one element remains in fifo
    {
        char *address = q->buffer[q->front].address;
        q->front = -1;
        q->end = -1;
        sem_post(&q->M);     // release mutex on CIS
        sem_post(&q->EMPTY); // increments EMPTY -> this value represents how many more items can be added
        return address;
    }
    else
    {
        char *address = q->buffer[q->front].address;
        q->front = (q->front + 1) % BUFFER_SIZE;
        sem_post(&q->M);     // release mutex on CIS
        sem_post(&q->EMPTY); // increments EMPTY -> this value represents how many more items can be added
        return address;
    }
}

/*
Used to free the memory dynamically allocated when the
fifo struct was initialized and close semaphores.

Returns 0 on success.
*/
int de_init_q(fifo *q)
{
    free(q->buffer);
    sem_close(&q->M);
    sem_close(&q->EMPTY);
    sem_close(&q->FULL);

    return 0;
}

/*
Used to debug and test my implementation.

Prints the address of each entry struct present
in the fifo struct, to include null entry structs.

Useful for checking what remains in the fifo after more is produced
than what is consumed.
*/
void print_q(fifo *q)
{
    printf("Buffer of size %d contains:\n", BUFFER_SIZE);

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        printf("%s\n", q->buffer[i].address);
    }
}