
#include <Python.h>
#include "hashmap.h"

static PyObject* sum(PyObject *self, PyObject *args) {
    Py_buffer ids;
    unsigned long long m_addr;
    Py_buffer a;
    Py_buffer b;
    Py_buffer res;

    if (!PyArg_ParseTuple(args, "y*Ky*y*w*", &ids, &m_addr, &a, &b, &res)) {
        return NULL;
    }

    unsigned long long * c_ids = (unsigned long long *) ids.buf;
    Py_ssize_t c_ids_size = ids.len / ids.itemsize;
    Int2IntHashTable_t * m = (Int2IntHashTable_t *) m_addr;
    double *c_a = (double *) a.buf;
    double *c_b = (double *) b.buf;
    double *c_res = (double *) res.buf;

    /* Compute - only pure C types, no Python overhead */
    for (Py_ssize_t i=0; i<c_ids_size; ++i) {
        size_t idx = int2int_get(m, c_ids[i]);
        c_res[idx] = c_a[idx] + c_b[idx];
    }
    /* Compute - end */

    PyBuffer_Release(&res);
    PyBuffer_Release(&b);
    PyBuffer_Release(&a);
    PyBuffer_Release(&ids);

    Py_RETURN_NONE;
}

static PyMethodDef computer_methods[] = {
    {"sum", sum, METH_VARARGS, "sum(int2int, a, b, res)"},
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
