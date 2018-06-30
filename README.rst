datastructs
===========

Simple data structures accessible from both Python and C. Data in structures
are stored as a primitive C types, so in C you can compute data without Python
overhead.

Python code:

::

    >>> from datastructs.hashmap import Int2Int

    >>> id2index = Int2Int(1000)

    >>> id2index[72351277] = 0
    >>> id2index[98092127498] = 1
    >>> id2index[126987499] = 2
    >>> id2index[36] = 3
    >>> id2index[980980] = 4
    ...
    >>> id2index[9534875] = 999
    >>> len(id2index)
    1000
    >>> id2index[980980]
    4
    >>> id2index.get_ptr()
    140329067037264


    >>> import array
    >>> from my_c_extension import some_func

    >>> a = array.array('d', (random.random() for i in range(1000)))
    >>> b = array.array('d', (random.random() for i in range(1000)))
    >>> c = array.array('d', 0 for i in range(1000)))
    >>> ids = array.array('Q', [98092127498, 980980, 36])

    >>> c[id2index[72351277]]
    0.0
    >>> c[id2index[98092127498]]
    0.0

    >>> some_func(id2index, ids, a, b, c)

    >>> c[id2index[72351277]]
    0.0
    >>> c[id2index[98092127498]]
    0.8163673050897404

C code:

::

    #include <hashmap.h>

    static void compute_data(Int2IntHashTable_t * id2idx,
            unsigned long long * ids, size_t ids_count,
            double * x, double * y, double * z) {

        size_t idx;

    	for (size_t i=0; i<ids_count; ++i) {
    		idx = int2int_get(id2idx, ids[i]);
    		z[idx] = x[idx] + y[idx];
    	}
    }

    static PyObject * my_c_extension_some_func(PyObject * id2index,
            PyObject * allowed_ids, PyObject * a, PyObject * b,
            PyObject * result) {

        Int2IntHashTable_t *id2idx;
        unsigned long long *ids;
        size_t *ids_count;
        double *x;
        double *y;
        double *z;

        // There is only one Python overhead - acquire pointers to buffers
        // which contain data and cast them to C types.

        compute_data(id2idx, ids, ids_count, x, y, z);

        Py_RETURN_NONE;
    }
