
#include <stddef.h>

#include "hashmap.h"

static inline size_t int2int_hash(const unsigned long long key,
        const size_t table_size) {
    return (97 * key) % table_size;
}

int int2int_set(Int2IntHashTable_t * const ctx,
        const unsigned long long key, const size_t value) {

    size_t idx = int2int_hash(key, ctx->table_size);

    if (ctx->current_size == ctx->size) {
        return -1;
    }
    while (ctx->table[idx].status == USED) {
        if (ctx->table[idx].key == key) {
            ctx->table[idx].value = value;
            return 0;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    ctx->table[idx].status = USED;
    ctx->table[idx].key = key;
    ctx->table[idx].value = value;
    ctx->current_size += 1;
    return 0;
}

size_t int2int_get(const Int2IntHashTable_t * const ctx,
        const unsigned long long key) {

    size_t idx = int2int_hash(key, ctx->table_size);

    while (ctx->table[idx].status == USED) {
        if (ctx->table[idx].key == key) {
            return ctx->table[idx].value;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    return (size_t) -1;
}

int int2int_has(const Int2IntHashTable_t * const ctx,
        const unsigned long long key) {

    size_t idx = int2int_hash(key, ctx->table_size);

    while (ctx->table[idx].status == USED) {
        if (ctx->table[idx].key == key) {
            return 1;
        }
        idx = (idx + 1) % ctx->table_size;
    }
    return 0;
}
