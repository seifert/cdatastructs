
#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stddef.h>

typedef enum {
    EMPTY,
    USED
} ItemStatus_e;

/*
 * int2int
 */

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

int int2int_get(const Int2IntHashTable_t * const ctx,
        const unsigned long long key, size_t * const value);

int int2int_has(const Int2IntHashTable_t * const ctx,
        const unsigned long long key);

/*
 * int2float
 */

typedef struct {
    unsigned long long key;
    double value;
    ItemStatus_e status;
} Int2FloatItem_t;

typedef struct {
    size_t size;
    size_t current_size;
    size_t table_size;
    Int2FloatItem_t *table;
} Int2FloatHashTable_t;

int int2float_set(Int2FloatHashTable_t * const ctx,
        const unsigned long long key, const double value);

int int2float_get(const Int2FloatHashTable_t * const ctx,
        const unsigned long long key, double * const value);

int int2float_has(const Int2FloatHashTable_t * const ctx,
        const unsigned long long key);

#endif /* HASHMAP_H_ */
