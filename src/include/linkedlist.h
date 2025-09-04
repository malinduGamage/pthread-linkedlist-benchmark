/*
 * linkedlist.h
 *
 * Simple singly‑linked list API.  This header defines a minimal
 * interface for a list of integer keys.  The list is used for
 * experiments in concurrent programming.  All functions operate on
 * a global list head defined in the corresponding implementation.
 *
 * The list functions themselves are not thread safe; external
 * synchronization must be applied when calling them concurrently.
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdbool.h>
#include <stddef.h>


typedef struct node {
    int data;
    struct node *next;
} node_t;


void list_init(int n);


bool list_member(int value);


bool list_insert(int value);

bool list_delete(int value);

void list_free(void);

#endif /* LINKEDLIST_H */