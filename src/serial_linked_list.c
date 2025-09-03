#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// You should change these values based on your lab sheet
#define N_INITIAL_NODES 1000
#define TOTAL_OPERATIONS 10000

// Define the fractions for each case
// Case 1: High Member
#define MEMBER_FRAC 0.99
#define INSERT_FRAC 0.005
#define DELETE_FRAC 0.005

/*
// Case 2: Balanced
#define MEMBER_FRAC 0.90
#define INSERT_FRAC 0.05
#define DELETE_FRAC 0.05
*/

/*
// Case 3: High Insert/Delete
#define MEMBER_FRAC 0.50
#define INSERT_FRAC 0.25
#define DELETE_FRAC 0.25
*/

// Linked list node structure
struct list_node_s {
    int data;
    struct list_node_s* next;
};

// Global head pointer
struct list_node_s* head = NULL;

// Function to generate a random number between 0 and 2^16 - 1
int generate_random_value() {
    return rand() % (1 << 16);
}

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

// Function to free the entire linked list
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

// Function to generate and shuffle an array of operations based on fractions
int* GenerateRandomOperations() {
    int* operations = malloc(TOTAL_OPERATIONS * sizeof(int));
    if (operations == NULL) {
        fprintf(stderr, "Error: Memory allocation for operations array failed.\n");
        return NULL;
    }

    int member_count = (int)(TOTAL_OPERATIONS * MEMBER_FRAC);
    int insert_count = (int)(TOTAL_OPERATIONS * INSERT_FRAC);
    int delete_count = (int)(TOTAL_OPERATIONS * DELETE_FRAC);

    //print counts
    printf("Operation counts - Member: %d, Insert: %d, Delete: %d\n", member_count, insert_count, delete_count);

    int i, j;

    // Fill the array with the correct number of each operation type
    for (i = 0; i < member_count; i++) {
        operations[i] = 0; // 0 for Member
    }
    for (j = 0; j < insert_count; j++) {
        operations[i + j] = 1; // 1 for Insert
    }
    i = i + j;
    printf("Insert count: %d\n", j);
    for (j = 0; j < delete_count; j++) {
        operations[i + j] = 2; // 2 for Delete
    }

    printf("Operation counts - Member: %d, Insert: %d, Delete: %d\n", member_count, insert_count, delete_count) ;
    //print array
    // for (i = 0; i < TOTAL_OPERATIONS; i++) {
    //     printf("%d ", operations[i]);
    // }
    // printf("\n");

    // Shuffle the operations array using Fisher-Yates algorithm
    for (i = TOTAL_OPERATIONS - 1; i > 0; i--) {
        j = rand() % (i + 1);
        int temp = operations[i];
        operations[i] = operations[j];
        operations[j] = temp;
    }

    return operations;
}

int main() {
    long i;
    int value;
    clock_t start_time, end_time;
    double elapsed_time;

    // Seed the random number generator
    srand(time(NULL));

    // Populate the initial linked list with unique, random values
    for (i = 0; i < N_INITIAL_NODES; i++) {
        do {
            value = generate_random_value();
        } while (Insert(value, &head) == 0);
    }

    printf("Initial list size: %d\n", CountList(head));

    // Generate and shuffle the operations array
    int* operation_order = GenerateRandomOperations();
    if (operation_order == NULL) {
        return 1; // Exit on allocation failure
    }

    // Start timer
    start_time = clock();

    // Perform the operations from the pre-generated array
    for (i = 0; i < TOTAL_OPERATIONS; i++) {
        switch (operation_order[i]) {
            case 0: // Member operation
                value = generate_random_value();
                Member(value, &head);
                break;
            case 1: // Insert operation
                // Keep generating new values until a successful Insert operation is performed
                do {
                    value = generate_random_value();
                } while (Insert(value, &head) == 0);
                break;
            case 2: // Delete operation
                value = generate_random_value();
                Delete(value, &head);
                break;
        }
    }

    // Stop timer
    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Final list size: %d\n", CountList(head));
    printf("Serial Linked List: Elapsed time = %.6f seconds\n", elapsed_time);

    // Clean up
    FreeList(&head);
    free(operation_order);

    return 0;
}
