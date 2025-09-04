

#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <stddef.h>


typedef enum {
    OP_MEMBER,
    OP_INSERT,
    OP_DELETE
} op_type_t;


typedef struct {
    op_type_t type;
    int key;
} operation_t;


operation_t *generate_operations(size_t m,
                                 double m_member_frac,
                                 double m_insert_frac,
                                 double m_delete_frac);


void free_operations(operation_t *ops);

#endif /* WORKLOAD_H */