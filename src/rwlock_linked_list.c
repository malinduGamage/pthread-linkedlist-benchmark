#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

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

// Global head pointer and read-write lock
struct list_node_s* head = NULL;
pthread_rwlock_t rwlock;

// Structure to pass multiple arguments to the thread function
struct thread_data {
    int* operation_array;
    long my_start;
    long my_count;
};

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
        return 0;
    } else {
        return 1;
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
        temp_p->data = value;
        temp_p->next = curr_p;
        if (pred_p == NULL) {
            *head_pp = temp_p;
        } else {
            pred_p->next = temp_p;
        }
        return 1;
    } else {
        return 0;
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
        if (pred_p == NULL) {
            *head_pp = curr_p->next;
            free(curr_p);
        } else {
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1;
    } else {
        return 0;
    }
}

// Function to free the entire linked list
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

int CountList(struct list_node_s* head) {
    int count = 0;
    struct list_node_s* curr = head;
    while (curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
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


// Thread work function
void* Thread_work(void* data_ptr) {
    struct thread_data* data = (struct thread_data*)data_ptr;
    long i;
    int value;

    for (i = 0; i < data->my_count; i++) {
        int op = data->operation_array[data->my_start + i];
        value = generate_random_value();

        switch (op) {
            case 0: // Member
                pthread_rwlock_rdlock(&rwlock);
                Member(value, &head);
                pthread_rwlock_unlock(&rwlock);
                break;
            case 1: // Insert
                pthread_rwlock_wrlock(&rwlock);
                do {
                    value = generate_random_value();
                } while (Insert(value, &head) == 0);
                pthread_rwlock_unlock(&rwlock);
                break;
            case 2: // Delete
                pthread_rwlock_wrlock(&rwlock);
                Delete(value, &head);
                pthread_rwlock_unlock(&rwlock);
                break;
        }
    }
    free(data);
    return NULL;
}

int main(int argc, char* argv[]) {
    long i;
    int value;
    clock_t start_time, end_time;
    double elapsed_time;
    pthread_t* thread_handles;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }
    long num_threads = strtol(argv[1], NULL, 10);
    if (num_threads <= 0 || num_threads > 8) {
        fprintf(stderr, "Number of threads must be between 1 and 8.\n");
        return 1;
    }
    thread_handles = malloc(num_threads * sizeof(pthread_t));

    srand(time(NULL));

    for (i = 0; i < N_INITIAL_NODES; i++) {
        do {
            value = generate_random_value();
        } while (Insert(value, &head) == 0);
    }

    pthread_rwlock_init(&rwlock, NULL);

    // Generate and shuffle the operations array once
    int* operation_order = GenerateRandomOperations();
    if (operation_order == NULL) {
        return 1;
    }

    long ops_per_thread = TOTAL_OPERATIONS / num_threads;
    long my_start, my_count;

    start_time = clock();

    for (i = 0; i < num_threads; i++) {
        my_start = i * ops_per_thread;
        my_count = (i == num_threads - 1) ? TOTAL_OPERATIONS - my_start : ops_per_thread;

        struct thread_data* data = malloc(sizeof(struct thread_data));
        data->operation_array = operation_order;
        data->my_start = my_start;
        data->my_count = my_count;
        
        pthread_create(&thread_handles[i], NULL, Thread_work, (void*)data);
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(thread_handles[i], NULL);
    }

    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Read-Write Lock Linked List (%ld threads): Elapsed time = %.6f seconds\n", num_threads, elapsed_time);
    printf("Final list count: %d\n", CountList(head));

    FreeList(&head);
    pthread_rwlock_destroy(&rwlock);
    free(thread_handles);
    free(operation_order);

    return 0;
}
