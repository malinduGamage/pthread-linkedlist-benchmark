#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdatomic.h>
#include "workload.h"
#include "utils.h"
#include "timing.h"

// Linked list node structure
struct list_node_s {
    int data;
    struct list_node_s* next;
};

// Shared data structure for the threads
struct rw_lock_data {
    struct list_node_s* head;
    pthread_rwlock_t rwlock;
    long m;
    long m_member;
    long m_insert;
    long m_delete;
    // Counters for operations
    _Atomic int tot_ops;
    _Atomic int member_ops;
    _Atomic int insert_ops;
    _Atomic int delete_ops;
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

int CountList(struct list_node_s* head) {
    int count = 0;
    struct list_node_s* curr = head;
    while (curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
}

void* Thread_work(void* data_ptr) {
    struct rw_lock_data* data = (struct rw_lock_data*)data_ptr;
    unsigned int seed = (unsigned int) time(NULL) ^ (unsigned int) pthread_self();

    while (atomic_load(&data->tot_ops) < data->m) {
        int op_type = rand_r(&seed) % 3;
        int value = rand_r(&seed) % (1 << 16);

        if (op_type == 0) { // Member
            if (atomic_load(&data->member_ops) < data->m_member) {
                pthread_rwlock_rdlock(&data->rwlock);
                Member(value, &data->head);
                pthread_rwlock_unlock(&data->rwlock);
                atomic_fetch_add(&data->member_ops, 1);
                atomic_fetch_add(&data->tot_ops, 1);
            }
        } else if (op_type == 1) { // Insert
            if (atomic_load(&data->insert_ops) < data->m_insert) {
                pthread_rwlock_wrlock(&data->rwlock);
                Insert(value, &data->head);
                pthread_rwlock_unlock(&data->rwlock);
                atomic_fetch_add(&data->insert_ops, 1);
                atomic_fetch_add(&data->tot_ops, 1);
            }
        } else { // Delete
            if (atomic_load(&data->delete_ops) < data->m_delete) {
                pthread_rwlock_wrlock(&data->rwlock);
                Delete(value, &data->head);
                pthread_rwlock_unlock(&data->rwlock);
                atomic_fetch_add(&data->delete_ops, 1);
                atomic_fetch_add(&data->tot_ops, 1);
            }
        }
    }
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

    struct rw_lock_data shared_data;
    shared_data.head = NULL;
    shared_data.m = n_total_operations;
    shared_data.m_member = (long)(n_total_operations * member_frac);
    shared_data.m_insert = (long)(n_total_operations * insert_frac);
    shared_data.m_delete = (long)(n_total_operations * delete_frac);
    shared_data.tot_ops = 0;
    shared_data.member_ops = 0;
    shared_data.insert_ops = 0;
    shared_data.delete_ops = 0;

    srand(time(NULL));
    int inserted_count = 0;
    while (inserted_count < n_initial_nodes) {
        int value = generate_random_value();
        if (Insert(value, &shared_data.head)) {
            inserted_count++;
        }
    }

    pthread_rwlock_init(&shared_data.rwlock, NULL);
    pthread_t* thread_handles = malloc(num_threads * sizeof(pthread_t));

    time_start();

    long i;
    for (i = 0; i < num_threads; i++) {
        pthread_create(&thread_handles[i], NULL, Thread_work, (void*)&shared_data);
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(thread_handles[i], NULL);
    }

    double elapsed_time = time_stop();
    printf("%.6f\n", elapsed_time);

    FreeList(&shared_data.head);
    pthread_rwlock_destroy(&shared_data.rwlock);
    free(thread_handles);

    return 0;
}
