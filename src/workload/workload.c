

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "workload.h"

operation_t *generate_operations(size_t m,
                                 double m_member_frac,
                                 double m_insert_frac,
                                 double m_delete_frac)
{
    if (m == 0) {
        return NULL;
    }
    operation_t *ops = (operation_t *)malloc(m * sizeof(operation_t));
    if (!ops) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t m_member = (size_t)((double)m * m_member_frac);
    size_t m_insert = (size_t)((double)m * m_insert_frac);
    size_t m_delete = (size_t)((double)m * m_delete_frac);
    size_t assigned = m_member + m_insert + m_delete;
    if (assigned < m) {
        m_member += m - assigned;
    }
    // Fill the array with the appropriate counts for each operation
    size_t index = 0;
    for (size_t i = 0; i < m_member; i++, index++) {
        ops[index].type = OP_MEMBER;
    }
    for (size_t i = 0; i < m_insert; i++, index++) {
        ops[index].type = OP_INSERT;
    }
    for (size_t i = 0; i < m_delete; i++, index++) {
        ops[index].type = OP_DELETE;
    }
    // Assign random keys.  Keys are always set before shuffling to
    // avoid biasing key distribution by operation type.
    for (size_t i = 0; i < m; i++) {
        ops[i].key = rand() & 0xFFFF; // 0–65535 inclusive
    }
   
    for (size_t i = m - 1; i > 0; i--) {
        size_t j = (size_t)rand() % (i + 1);
        op_type_t tmp = ops[i].type;
        ops[i].type = ops[j].type;
        ops[j].type = tmp;
    }
    return ops;
}

void free_operations(operation_t *ops) {
    free(ops);
}