#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "workload.h"
#include "utils.h"
#include "timing.h"

// Linked list node structure
struct list_node_s {
    int data;
    struct list_node_s* next;
};

// Global head pointer and mutex
struct list_node_s* head = NULL;
pthread_mutex_t mutex;

// Structure to pass multiple arguments to the thread function
struct thread_data {
    operation_t* operations;
    long my_start;
    long my_count;
};

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

// Thread work function
void* Thread_work(void* data_ptr) {
    struct thread_data* data = (struct thread_data*)data_ptr;
    long i;

    for (i = 0; i < data->my_count; i++) {
        operation_t op = data->operations[data->my_start + i];

        switch (op.type) {
            case OP_MEMBER:
                pthread_mutex_lock(&mutex);
                Member(op.key, &head);
                pthread_mutex_unlock(&mutex);
                break;
            case OP_INSERT:
                pthread_mutex_lock(&mutex);
                Insert(op.key, &head);
                pthread_mutex_unlock(&mutex);
                break;
            case OP_DELETE:
                pthread_mutex_lock(&mutex);
                Delete(op.key, &head);
                pthread_mutex_unlock(&mutex);
                break;
        }
    }
    free(data);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s <num_threads> <n_initial_nodes> <n_total_operations> <member_frac> <insert_frac> <delete_frac>\n", argv[0]);
        return 1;
    }

    long num_threads = strtol(argv[1], NULL, 10);
    int n_initial_nodes = atoi(argv[2]);
    int n_total_operations = atoi(argv[3]);
    double member_frac = atof(argv[4]);
    double insert_frac = atof(argv[5]);
    double delete_frac = atof(argv[6]);

    if (num_threads <= 0 || num_threads > 8) {
        fprintf(stderr, "Number of threads must be between 1 and 8.\n");
        return 1;
    }

    long i;
    int value;
    double elapsed_time;
    pthread_t* thread_handles;

    thread_handles = malloc(num_threads * sizeof(pthread_t));

    srand(time(NULL));

    for (i = 0; i < n_initial_nodes; i++) {
        do {
            value = generate_random_value();
        } while (Insert(value, &head) == 0);
    }

    pthread_mutex_init(&mutex, NULL);

    operation_t* operations = generate_operations(n_total_operations, member_frac, insert_frac, delete_frac);
    if (operations == NULL) {
        return 1;
    }
    
    long ops_per_thread = n_total_operations / num_threads;
    long my_start, my_count;

    time_start();

    for (i = 0; i < num_threads; i++) {
        my_start = i * ops_per_thread;
        my_count = (i == num_threads - 1) ? n_total_operations - my_start : ops_per_thread;

        struct thread_data* data = malloc(sizeof(struct thread_data));
        data->operations = operations;
        data->my_start = my_start;
        data->my_count = my_count;
        
        pthread_create(&thread_handles[i], NULL, Thread_work, (void*)data);
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(thread_handles[i], NULL);
    }

    elapsed_time = time_stop();

    printf("%.6f\n", elapsed_time);

    FreeList(&head);
    pthread_mutex_destroy(&mutex);
    free(thread_handles);
    free_operations(operations);

    return 0;
}