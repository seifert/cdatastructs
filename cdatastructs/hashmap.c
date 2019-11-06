
#include <stddef.h>

#include "hashmap.h"

static inline size_t u_long_long_hash(const unsigned long long key,
        const size_t table_size) {
    return (97 * key) % table_size;
}

/*
 * int2int
 */

int int2int_set(Int2IntHashTable_t * const ctx,
        const unsigned long long key, const size_t value) {

    Int2IntItem_t *table = (void*) ctx + sizeof(Int2IntHashTable_t);
    size_t idx = u_long_long_hash(key, ctx->table_size);

    for (size_t i=0; i<ctx->table_size; ++i) {
        if ((table[idx].status == USED) && (table[idx].key == key)) {
            table[idx].value = value;
            return 0;
        }
        if ((table[idx].status == EMPTY) || (table[idx].status == DELETED)) {
            table[idx].status = USED;
            table[idx].key = key;
            table[idx].value = value;
            ctx->current_size += 1;
            return 0;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    return -1;
}

int int2int_del(Int2IntHashTable_t * const ctx,
        const unsigned long long key) {

    Int2IntItem_t *table = (void*) ctx + sizeof(Int2IntHashTable_t);
    size_t idx = u_long_long_hash(key, ctx->table_size);

    for (size_t i=0; i<ctx->table_size; ++i) {
        if (table[idx].status == EMPTY) {
            break;
        }
        if ((table[idx].status == USED) && (table[idx].key == key)) {
            table[idx].status = DELETED;
            ctx->current_size -= 1;
            return 0;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    return -1;
}

int int2int_get(const Int2IntHashTable_t * const ctx,
        const unsigned long long key, size_t * const value) {

    size_t * p_value;

    if (int2int_ptr(ctx, key, &p_value) == 0) {
        *value = *p_value;
        return 0;
    }
    return -1;
}

int int2int_ptr(const Int2IntHashTable_t * const ctx,
        const unsigned long long key, size_t ** const value) {

    Int2IntItem_t *table = (void*) ctx + sizeof(Int2IntHashTable_t);
    size_t idx = u_long_long_hash(key, ctx->table_size);

    for (size_t i=0; i<ctx->table_size; ++i) {
        if (table[idx].status == EMPTY) {
            break;
        }
        if ((table[idx].status == USED) && (table[idx].key == key)) {
            *value = &(table[idx].value);
            return 0;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    return -1;
}

int int2int_has(const Int2IntHashTable_t * const ctx,
        const unsigned long long key) {

    size_t * value;

    return int2int_ptr(ctx, key, &value);
}

/*
 * int2float
 */

int int2float_set(Int2FloatHashTable_t * const ctx,
        const unsigned long long key, const double value) {

    Int2FloatItem_t *table = (void*) ctx + sizeof(Int2FloatHashTable_t);
    size_t idx = u_long_long_hash(key, ctx->table_size);

    for (size_t i=0; i<ctx->table_size; ++i) {
        if ((table[idx].status == USED) && (table[idx].key == key)) {
            table[idx].value = value;
            return 0;
        }
        if ((table[idx].status == EMPTY) || (table[idx].status == DELETED)) {
            table[idx].status = USED;
            table[idx].key = key;
            table[idx].value = value;
            ctx->current_size += 1;
            return 0;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    return -1;
}

int int2float_del(Int2FloatHashTable_t * const ctx,
        const unsigned long long key) {

    Int2FloatItem_t *table = (void*) ctx + sizeof(Int2FloatHashTable_t);
    size_t idx = u_long_long_hash(key, ctx->table_size);

    for (size_t i=0; i<ctx->table_size; ++i) {
        if (table[idx].status == EMPTY) {
            break;
        }
        if ((table[idx].status == USED) && (table[idx].key == key)) {
            table[idx].status = DELETED;
            ctx->current_size -= 1;
            return 0;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    return -1;
}

int int2float_get(const Int2FloatHashTable_t * const ctx,
        const unsigned long long key, double * const value) {

    double * p_value;

    if (int2float_ptr(ctx, key, &p_value) == 0) {
        *value = *p_value;
        return 0;
    }
    return -1;
}

int int2float_ptr(const Int2FloatHashTable_t * const ctx,
        const unsigned long long key, double ** const value) {

    Int2FloatItem_t *table = (void*) ctx + sizeof(Int2FloatHashTable_t);
    size_t idx = u_long_long_hash(key, ctx->table_size);

    for (size_t i=0; i<ctx->table_size; ++i) {
        if (table[idx].status == EMPTY) {
            break;
        }
        if ((table[idx].status == USED) && (table[idx].key == key)) {
            *value = &(table[idx].value);
            return 0;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    return -1;
}

int int2float_has(const Int2FloatHashTable_t * const ctx,
        const unsigned long long key) {

    double * value;

    return int2float_ptr(ctx, key, &value);
}
