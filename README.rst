cdatastructs
============

Simple data structures accessible from both Python and C. Data in structures
are stored as a primitive C types, so in C you can compute data without Python
overhead.

Python code:

::

    >>> import array
    >>> import itertools
    >>> import math
    >>> import random

    >>> from cdatastructs.hashmap import Int2Int

    >>> from my_c_extension import calculate_data

	# Create instance of the Int2Int, it will be mapping from ID
	# to position in array
    >>> id2index = Int2Int()

    # Instance of the id2index is mapping, so you can use it same as dict
    >>> id2index[72351277] = 0
    >>> id2index[98092127498] = 1
    >>> id2index[126987499] = 2
    >>> id2index[36] = 3
    >>> id2index[980980] = 4
    >>> len(id2index)
    5
    >>> id2index[980980]
    4
    >>> id2index.buffer_ptr
    94691713534960

    # Prepare two arrays with numbers for calculate and one array for results
    >>> a = array.array('d', (random.random() for _ in id2index))
    >>> b = array.array('d', (random.random() for _ in id2index))
    >>> results = array.array('d', (math.nan for _ in id2index))

    # Calculate data for IDs 98092127498, 126987499 and 36
    >>> ids = array.array('Q', [98092127498, 126987499, 36])
    >>> calculate_data(id2index, ids, a, b, results)

	# Obtaint results
    >>> results[id2index[72351277]]
    nan
    >>> results[id2index[98092127498]]
    0.8163673050897404

C code:

::

    #include <hashmap.h>

    static PyObject * my_c_extension_compute_data(
            PyObject *id2index, PyObject *args) {

        Int2IntHashTable_t *id2position;
        unsigned long long *ids;
        size_t *ids_count;
        double *a;
        double *b;
        double *res;

        // Parse args, obtain pointers to buffers and cast them to C types
        ...

        // Calculate data, there are only pure C types, no Python overhead
        for (size_t i=0; i<ids_count; ++i) {
            unsigned long long id = ids[i];
            size_t *position;
            if (int2int_get(id2position, id, position) != 0) {
                goto error;
            }
            res[*position] = a[*position] + b[*position];
        }

        ...
        Py_RETURN_NONE;
    error:
        ...
        return NULL;
    }

See ``demos/`` for more examples and details.

License
-------

3-clause BSD
