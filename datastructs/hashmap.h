
#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stddef.h>

/*
 * int2int
 */

typedef enum {
    EMPTY,
    USED
} ItemStatus_e;

typedef struct {
    unsigned long long key;
    size_t value;
    ItemStatus_e status;
} Int2IntItem_t;

typedef struct {
    size_t size;
    size_t current_size;
    size_t table_size;
    Int2IntItem_t *table;
} Int2IntHashTable_t;

int int2int_set(Int2IntHashTable_t * const ctx,
        const unsigned long long key, const size_t value);

size_t int2int_get(const Int2IntHashTable_t * const ctx,
        const unsigned long long key);

int int2int_has(const Int2IntHashTable_t * const ctx,
        const unsigned long long key);

#endif /* HASHMAP_H_ */
