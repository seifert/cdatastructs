
cdef extern from "hashmap.h":

    ctypedef enum ItemStatus_e:
        EMPTY
        USED

    # int2int

    ctypedef struct Int2IntItem_t:
        unsigned long long key
        size_t value
        ItemStatus_e status

    ctypedef struct Int2IntHashTable_t:
        size_t size
        size_t current_size
        size_t table_size
        Int2IntItem_t * table

    cdef int int2int_set(
        Int2IntHashTable_t * const ctx,
        const unsigned long long key, const size_t value)

    cdef int int2int_get(
        const Int2IntHashTable_t * const ctx,
        const unsigned long long key, size_t * const value)

    cdef size_t * int2int_get_ptr(
        const Int2IntHashTable_t * const ctx,
        const unsigned long long key)

    cdef int int2int_has(
        const Int2IntHashTable_t * const ctx, const unsigned long long key)

    # int2float

    ctypedef struct Int2FloatItem_t:
        unsigned long long key
        double value
        ItemStatus_e status

    ctypedef struct Int2FloatHashTable_t:
        size_t size
        size_t current_size
        size_t table_size
        Int2FloatItem_t * table

    cdef int int2float_set(
        Int2FloatHashTable_t * const ctx,
        const unsigned long long key, const double value)

    cdef int int2float_get(
        const Int2FloatHashTable_t * const ctx,
        const unsigned long long key, double * const value)

    cdef double * int2float_get_ptr(
        const Int2FloatHashTable_t * const ctx,
        const unsigned long long key)

    cdef int int2float_has(
        const Int2FloatHashTable_t * const ctx,
        const unsigned long long key)
