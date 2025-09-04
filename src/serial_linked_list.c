#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "workload.h"
#include "utils.h"
#include "timing.h"

// Linked list node structure
struct list_node_s {
    int data;
    struct list_node_s* next;
};

// Global head pointer
struct list_node_s* head = NULL;

// Function to check if a value is in the list
int Member(int value, struct list_node_s** head_pp) {
    struct list_node_s* curr_p = *head_pp;

    while (curr_p != NULL && curr_p->data < value) {
        curr_p = curr_p->next;
    }

    if (curr_p == NULL || curr_p->data > value) {
        return 0; // Not found
    } else {
        return 1; // Found
    }
}

// Function to insert a value into the list
int Insert(int value, struct list_node_s** head_pp) {
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;
    struct list_node_s* temp_p;

    while (curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p == NULL || curr_p->data > value) {
        temp_p = malloc(sizeof(struct list_node_s));
        if (temp_p == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return 0; // Failure
        }
        temp_p->data = value;
        temp_p->next = curr_p;
        if (pred_p == NULL) { /* New first node */
            *head_pp = temp_p;
        } else {
            pred_p->next = temp_p;
        }
        return 1; // Success
    } else { /* Value already in list */
        return 0; // Failure
    }
}

// Function to delete a value from the list
int Delete(int value, struct list_node_s** head_pp) {
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;

    while (curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p != NULL && curr_p->data == value) {
        if (pred_p == NULL) { /* Deleting first node in list */
            *head_pp = curr_p->next;
            free(curr_p);
        } else {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1; // Success
    } else { /* Value isn't in list */
        return 0; // Failure
    }
}

// Function to count the number of items in the linked list
int CountList(struct list_node_s* head) {
    int count = 0;
    struct list_node_s* curr = head;
    while (curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
}

void FreeList(struct list_node_s** head_pp) {
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* temp_p;

    while (curr_p != NULL) {
        temp_p = curr_p;
        curr_p = curr_p->next;
        free(temp_p);
    }
    *head_pp = NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s <num_threads> <n_initial_nodes> <n_total_operations> <member_frac> <insert_frac> <delete_frac>\n", argv[0]);
        return 1;
    }

    int n_initial_nodes = atoi(argv[2]);
    int n_total_operations = atoi(argv[3]);
    double member_frac = atof(argv[4]);
    double insert_frac = atof(argv[5]);
    double delete_frac = atof(argv[6]);

    long i;
    int value;
    double elapsed_time;

    // Seed the random number generator
    srand(time(NULL));

    // Populate the initial linked list with unique, random values
    for (i = 0; i < n_initial_nodes; i++) {
        do {
            value = generate_random_value();
        } while (Insert(value, &head) == 0);
    }

    // Generate and shuffle the operations array
    operation_t* operations = generate_operations(n_total_operations, member_frac, insert_frac, delete_frac);
    if (operations == NULL) {
        return 1; // Exit on allocation failure
    }

    // Start timer
    time_start();

    // Perform the operations from the pre-generated array
    for (i = 0; i < n_total_operations; i++) {
        switch (operations[i].type) {
            case OP_MEMBER:
                Member(operations[i].key, &head);
                break;
            case OP_INSERT:
                Insert(operations[i].key, &head);
                break;
            case OP_DELETE:
                Delete(operations[i].key, &head);
                break;
        }
    }

    // Stop timer
    elapsed_time = time_stop();

    printf("%.6f\n", elapsed_time);

    // Clean up
    FreeList(&head);
    free_operations(operations);

    return 0;
}
