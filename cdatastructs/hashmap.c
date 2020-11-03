
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

static inline size_t u_long_long_hash(const unsigned long long key,
        const size_t table_size) {
    return (97 * key) % table_size;
}

/*
 * int2int
 */

int int2int_new(const size_t size, Int2IntHashTable_t ** new_ctx) {
    size_t table_size = NEW_TABLE_SIZE(size);
    size_t memory_size = INT2INT_MEMORY_SIZE(table_size);
    Int2IntHashTable_t *hashmap = malloc(memory_size);

    if (NULL == hashmap) {
        return -1;
    }
    memset(hashmap, 0, memory_size);

    hashmap->size = size;
    hashmap->current_size = 0;
    hashmap->table_size = table_size;
    hashmap->readonly = false;

    *new_ctx = hashmap;

    return 0;
}

int int2int_set(Int2IntHashTable_t * ctx,
        const unsigned long long key, const size_t value,
        Int2IntHashTable_t ** new_ctx) {

    Int2IntItem_t *table = (Int2IntItem_t*) (
            (char*) ctx + sizeof(Int2IntHashTable_t));
    Int2IntHashTable_t *new_hashmap;
    Int2IntItem_t item;
    size_t idx;

    if (ctx->readonly) {
        return -1;
    }

    // Resize table if necessary
    if (NULL != new_ctx) {
        if (ctx->current_size == ctx->size) {
            if (int2int_new(ctx->size * 2, &new_hashmap)) {
                return -1;
            }

            for (size_t i=0; i<ctx->table_size; ++i) {
                item = table[i];
                if (item.status == USED) {
                    if (int2int_set(new_hashmap, item.key, item.value, NULL)) {
                        free(new_hashmap);
                        return -1;
                    }
                }
            }

            free(ctx);
            ctx = new_hashmap;
            table = (Int2IntItem_t*) ((char*) ctx + sizeof(Int2IntHashTable_t));
        }
        *new_ctx = ctx;
    }

    idx = u_long_long_hash(key, ctx->table_size);
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

    Int2IntItem_t *table = (Int2IntItem_t*) (
            (char*) ctx + sizeof(Int2IntHashTable_t));
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

    Int2IntItem_t *table = (Int2IntItem_t*) (
            (char*) ctx + sizeof(Int2IntHashTable_t));
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

int int2float_new(const size_t size, Int2FloatHashTable_t ** new_ctx) {
    size_t table_size = NEW_TABLE_SIZE(size);
    size_t memory_size = INT2FLOAT_MEMORY_SIZE(table_size);
    Int2FloatHashTable_t *hashmap = malloc(memory_size);

    if (NULL == hashmap) {
        return -1;
    }
    memset(hashmap, 0, memory_size);

    hashmap->size = size;
    hashmap->current_size = 0;
    hashmap->table_size = table_size;
    hashmap->readonly = false;

    *new_ctx = hashmap;

    return 0;
}

int int2float_set(Int2FloatHashTable_t * ctx,
        const unsigned long long key, const double value,
        Int2FloatHashTable_t ** new_ctx) {

    Int2FloatItem_t *table = (Int2FloatItem_t*) (
            (char*) ctx + sizeof(Int2FloatHashTable_t));
    Int2FloatHashTable_t *new_hashmap;
    Int2FloatItem_t item;
    size_t idx;

    if (ctx->readonly) {
        return -1;
    }

    // Resize table if necessary
    if (NULL != new_ctx) {
        if (ctx->current_size == ctx->size) {
            if (int2float_new(ctx->size * 2, &new_hashmap)) {
                return -1;
            }

            for (size_t i=0; i<ctx->table_size; ++i) {
                item = table[i];
                if (item.status == USED) {
                    if (int2float_set(new_hashmap, item.key,
                            item.value, NULL)) {
                        free(new_hashmap);
                        return -1;
                    }
                }
            }

            free(ctx);
            ctx = new_hashmap;
            table = (Int2FloatItem_t*) (
                    (char*) ctx + sizeof(Int2FloatHashTable_t));
        }
        *new_ctx = ctx;
    }

    idx = u_long_long_hash(key, ctx->table_size);
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

    Int2FloatItem_t *table = (Int2FloatItem_t*) (
            (char*) ctx + sizeof(Int2FloatHashTable_t));
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

    Int2FloatItem_t *table = (Int2FloatItem_t*) (
            (char*) ctx + sizeof(Int2FloatHashTable_t));
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
