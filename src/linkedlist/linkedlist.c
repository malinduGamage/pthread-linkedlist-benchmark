
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "linkedlist.h"

static node_t *head = NULL;

void list_init(int n) {
    // Empty any existing list
    list_free();

    int count = 0;
    // Populate the list with unique values between 0 and 65535
    while (count < n) {
        int val = rand() % 65536;
        // Attempt to insert; if successful increment count
        if (list_insert(val)) {
            count++;
        }
    }
}

bool list_member(int value) {
    node_t *curr = head;
    while (curr != NULL) {
        if (value == curr->data) {
            return true;
        } else if (value < curr->data) {

            return false;
        }
        curr = curr->next;
    }
    return false;
}

bool list_insert(int value) {
    // First check if value already exists
    node_t *curr = head;
    node_t *pred = NULL;
    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }
    if (curr != NULL && curr->data == value) {
        // Duplicate found
        return false;
    }
    // Create new node
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (!new_node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_node->data = value;
    // Insert at head or between pred and curr
    if (pred == NULL) {
        new_node->next = head;
        head = new_node;
    } else {
        new_node->next = pred->next;
        pred->next = new_node;
    }
    return true;
}

bool list_delete(int value) {
    node_t *curr = head;
    node_t *pred = NULL;
    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }
    if (curr == NULL || curr->data != value) {
        // Not found
        return false;
    }
    // Remove node
    if (pred == NULL) {
        head = curr->next;
    } else {
        pred->next = curr->next;
    }
    free(curr);
    return true;
}

void list_free(void) {
    node_t *curr = head;
    while (curr != NULL) {
        node_t *next = curr->next;
        free(curr);
        curr = next;
    }
    head = NULL;
}