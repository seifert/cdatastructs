
#include <Python.h>

#include "hashmap.h"

/*
 * Int2Int class
 */

typedef struct {
    PyObject_HEAD
    Int2IntHashTable_t hashmap;
} Int2Int_t;

static void Int2Int_dealloc(Int2Int_t *self) {
    PyMem_RawFree(self->hashmap.table);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* Int2Int_new(PyTypeObject *type,
        PyObject *args, PyObject *kwds) {

    const Py_ssize_t size;
    Int2Int_t *self;
    size_t table_size;
    Int2IntItem_t *table;

    /* Parse arguments */
    if (!PyArg_ParseTuple(args, "n", &size)) {
        return NULL;
    }

    /* Create instance */
    self = (Int2Int_t*) type->tp_alloc(type, 0);
    if (!self) {
        return NULL;
    }

    /* Allocate hashtable memory */
    table_size = size * 2;
    table = (Int2IntItem_t*) PyMem_RawCalloc(table_size, sizeof(Int2IntItem_t));
    if (!table) {
        Py_TYPE(self)->tp_free((PyObject*) self);
        return PyErr_NoMemory();
    }

    /* Initialize object attributes */
    self->hashmap.size = size;
    self->hashmap.current_size = 0;
    self->hashmap.table_size = table_size;
    self->hashmap.table = table;

    return (PyObject*) self;
}

static PyObject* Int2Int_repr(Int2Int_t *self) {
    return PyUnicode_FromFormat("<%s: object at %p, size %zd (maxsize %zd)>",
            self->ob_base.ob_type->tp_name, self, self->hashmap.current_size,
            self->hashmap.size);
}

static Py_ssize_t Int2Int_len(PyObject *obj) {
    Int2Int_t *self = (Int2Int_t*) obj;
    return self->hashmap.current_size;
}

int Int2Int_contains(PyObject *obj, PyObject *key) {
    Int2Int_t *self = (Int2Int_t*) obj;
    unsigned long long c_key;

    c_key = PyLong_AsUnsignedLongLong(key);
    if (c_key == (unsigned long long) -1) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }

    return int2int_has(&self->hashmap, c_key);
}

static int Int2Int_setitem(PyObject *obj, PyObject *key, PyObject *value) {
    Int2Int_t *self = (Int2Int_t*) obj;
    unsigned long long c_key;
    size_t c_value;

    c_key = PyLong_AsUnsignedLongLong(key);
    if (c_key == (unsigned long long) -1) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }

    c_value = PyLong_AsSize_t(value);
    if (c_value == (size_t) -1) {
        PyErr_SetString(PyExc_TypeError, "'value' must be an integer");
        return -1;
    }

    if (int2int_set(&self->hashmap, c_key, c_value) == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Maximum size has been exceeded");
        return -1;
    }

    return 0;
}

static PyObject* Int2Int_getitem(PyObject *obj, PyObject *key) {
    Int2Int_t *self = (Int2Int_t*) obj;
    unsigned long long c_key;
    size_t c_value;

    c_key = PyLong_AsUnsignedLongLong(key);
    if (c_key == (unsigned long long) -1) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }

    c_value = int2int_get(&self->hashmap, c_key);
    if (c_value == (size_t) -1) {
        return PyErr_Format(PyExc_KeyError, "%llu", c_key);
    }

    return PyLong_FromSize_t(c_value);
}

static PyObject* Int2Int_get(Int2Int_t *self, PyObject *args, PyObject *kw) {
    static char *kwnames[] = {"key", "default", NULL};
    const unsigned long long key;
    Py_ssize_t default_value = -1;
    size_t value;

    if (!PyArg_ParseTupleAndKeywords(args, kw,
            "K|n", kwnames, &key, &default_value)) {
        return NULL;
    }

    value = int2int_get(&self->hashmap, key);
    if (value == (unsigned long long) -1) {
        if (default_value == -1) {
            Py_INCREF(Py_None);
            return Py_None;
        }
        return PyLong_FromSsize_t(default_value);
    }

    return PyLong_FromSize_t(value);
}

static PyObject* Int2Int_get_ptr(Int2Int_t *self) {
    return PyLong_FromVoidPtr(&self->hashmap);
}

static PySequenceMethods Int2Int_sequence_methods = {
    0,                                                  /* sq_length */
    0,                                                  /* sq_concat */
    0,                                                  /* sq_repeat */
    0,                                                  /* sq_item */
    0,                                                  /* sq_slice */
    0,                                                  /* sq_ass_item */
    0,                                                  /* sq_ass_slice */
    (objobjproc) Int2Int_contains,                      /* sq_contains */
    0,                                                  /* sq_inplace_concat */
    0,                                                  /* sq_inplace_repeat */
};

static PyMappingMethods Int2Int_mapping_methods = {
    (lenfunc) Int2Int_len,                              /* mp_length */
    (binaryfunc) Int2Int_getitem,                       /* mp_subscript */
    (objobjargproc) Int2Int_setitem,                    /* mp_ass_subscript */
};

static PyMethodDef Int2Int_methods[] = {
    {"get", (PyCFunction) Int2Int_get, METH_VARARGS | METH_KEYWORDS,
            "get(self, key, default=None, /)\n"
            "--\n"
            "\n"
            "Return value for key. If key does not exist, return default\n"
            "value, otherwise return None."},
    {"get_ptr", (PyCFunction) Int2Int_get_ptr, METH_NOARGS,
            "get_ptr(self, /)\n"
            "--\n"
            "\n"
            "Return pointer to internal Int2IntHashTable_t structure."},
    {NULL}
};

static PyTypeObject Int2Int_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "datastructs.hashmap.Int2Int",                      /* tp_name */
    sizeof(Int2Int_t),                                  /* tp_basicsize */
    0,                                                  /* tp_itemsize */
    (destructor) Int2Int_dealloc,                       /* tp_dealloc */
    0,                                                  /* tp_print */
    0,                                                  /* tp_getattr */
    0,                                                  /* tp_setattr */
    0,                                                  /* tp_compare */
    (reprfunc) Int2Int_repr,                            /* tp_repr */
    0,                                                  /* tp_as_number */
    &Int2Int_sequence_methods,                          /* tp_as_sequence */
    &Int2Int_mapping_methods,                           /* tp_as_mapping */
    0,                                                  /* tp_hash */
    0,                                                  /* tp_call */
    0,                                                  /* tp_str */
    0,                                                  /* tp_getattro */
    0,                                                  /* tp_setattro */
    0,                                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /* tp_flags */
    "Simple fixed-size hashmap which maps int key "     /* tp_doc */
    "to int value.\n"
    "\n"
    "Provides pointer to internal C structure and set/get/has functions,\n"
    "so hashmap is accesible from pure C. Easily fill data in Python and\n"
    "compute in C or Cython.\n",
    0,                                                  /* tp_traverse */
    0,                                                  /* tp_clear */
    0,                                                  /* tp_richcompare */
    0,                                                  /* tp_weaklistoffset */
    0,                                                  /* tp_iter */
    0,                                                  /* tp_iternext */
    Int2Int_methods,                                    /* tp_methods */
    0,                                                  /* tp_members */
    0,                                                  /* tp_getset */
    0,                                                  /* tp_base */
    0,                                                  /* tp_dict */
    0,                                                  /* tp_descr_get */
    0,                                                  /* tp_descr_set */
    0,                                                  /* tp_dictoffset */
    0,                                                  /* tp_init */
    0,                                                  /* tp_alloc */
    (newfunc) Int2Int_new,                              /* tp_new */
};

/*
 * hashmap module
 */

static PyModuleDef hashmapmodule = {
    PyModuleDef_HEAD_INIT,                              /* m_base */
    "hashmap",                                          /* m_name */
    "The hashmap module provides classes for "          /* m_doc */
    "key -> value mapping. Goal is \n"
    "make mapping accesible from both Python and C.",
    -1,                                                 /* m_size */
    0,                                                  /* m_methods */
    0,                                                  /* m_slots */
    0,                                                  /* m_traverse */
    0,                                                  /* m_clear */
    0                                                   /* m_free */
};

PyMODINIT_FUNC PyInit_hashmap(void) {
    PyObject *module;

    if (PyType_Ready(&Int2Int_type) < 0) {
        return NULL;
    }

    module = PyModule_Create(&hashmapmodule);
    if (module == NULL) {
        return NULL;
    }

    Py_INCREF(&Int2Int_type);
    PyModule_AddObject(module, "Int2Int", (PyObject*) &Int2Int_type);

    return module;
}
