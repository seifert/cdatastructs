
#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    EMPTY,
    USED,
    DELETED
} ItemStatus_e;

#define NEW_TABLE_SIZE(ncount) (((ncount) * 1.2) + 1)

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
    bool readonly;
} Int2IntHashTable_t;

#define INT2INT_INITIAL_SIZE 8

#define INT2INT_MEMORY_SIZE(ncount) (sizeof(Int2IntHashTable_t) \
        + ((ncount) * sizeof(Int2IntItem_t)))

int int2int_new(const size_t size, Int2IntHashTable_t ** new_ctx);

int int2int_set(Int2IntHashTable_t * ctx,
        const unsigned long long key, const size_t value,
        Int2IntHashTable_t ** new_ctx);

int int2int_del(Int2IntHashTable_t * const ctx,
        const unsigned long long key);

int int2int_get(const Int2IntHashTable_t * const ctx,
        const unsigned long long key, size_t * const value);

int int2int_ptr(const Int2IntHashTable_t * const ctx,
        const unsigned long long key, size_t ** const value);

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
    bool readonly;
} Int2FloatHashTable_t;

#define INT2FLOAT_INITIAL_SIZE 8

#define INT2FLOAT_MEMORY_SIZE(ncount) (sizeof(Int2FloatHashTable_t) \
        + ((ncount) * sizeof(Int2FloatItem_t)))

int int2float_new(const size_t size, Int2FloatHashTable_t ** new_ctx);

int int2float_set(Int2FloatHashTable_t * ctx,
        const unsigned long long key, const double value,
        Int2FloatHashTable_t ** new_ctx);

int int2float_del(Int2FloatHashTable_t * const ctx,
        const unsigned long long key);

int int2float_get(const Int2FloatHashTable_t * const ctx,
        const unsigned long long key, double * const value);

int int2float_ptr(const Int2FloatHashTable_t * const ctx,
        const unsigned long long key, double ** const value);

int int2float_has(const Int2FloatHashTable_t * const ctx,
        const unsigned long long key);

#endif /* HASHMAP_H_ */
