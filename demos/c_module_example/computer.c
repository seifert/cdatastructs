
#include <Python.h>

#include "hashmap.h"

static PyObject* sum(PyObject *self, PyObject *args) {
    Py_buffer ids;
    size_t ids_size;
    unsigned long long *p_ids;
    unsigned long long id2pos_addr;
    Int2IntHashTable_t *id2pos;
    Py_buffer a;
    double *p_a;
    Py_buffer b;
    double *p_b;
    Py_buffer res;
    double *p_res;
    unsigned long long current_id;
    size_t current_position;

    if (!PyArg_ParseTuple(args, "y*Ky*y*w*",
            &ids, &id2pos_addr, &a, &b, &res)) {
        return NULL;
    }

    /* Obtain pointers to structures */
    p_ids = (unsigned long long *) ids.buf;
    ids_size = ids.len / ids.itemsize;
    id2pos = (Int2IntHashTable_t *) id2pos_addr;
    p_a = (double *) a.buf;
    p_b = (double *) b.buf;
    p_res = (double *) res.buf;

    /* Calculate - there are only pure C types, no Python overhead */
    for (size_t i=0; i<ids_size; ++i) {
        current_id = p_ids[i];
        if (int2int_get(id2pos, current_id, &current_position) != 0) {
            goto error;
        }
        p_res[current_position] = p_a[current_position] + p_b[current_position];
    }

    /* Release buffers */
    PyBuffer_Release(&res);
    PyBuffer_Release(&b);
    PyBuffer_Release(&a);
    PyBuffer_Release(&ids);

    Py_RETURN_NONE;

error:
    /* Release buffers  and set exception */
    PyBuffer_Release(&res);
    PyBuffer_Release(&b);
    PyBuffer_Release(&a);
    PyBuffer_Release(&ids);

    PyErr_Format(PyExc_KeyError, "%llu", current_id);
    return NULL;
}

static PyMethodDef computer_methods[] = {
    {"sum", sum, METH_VARARGS,
            "sum(ids, int2int, array_a, array_b, array_result)"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef computer_module = {
    PyModuleDef_HEAD_INIT,
    "computer",
    NULL,
    -1,
    computer_methods
};

PyMODINIT_FUNC PyInit_computer(void) {
    return PyModule_Create(&computer_module);
}
