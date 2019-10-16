
#include <stdbool.h>
#include <stddef.h>

#include <Python.h>

#include "hashmap.h"

/******************************************************************************
 * Hashmap iterator - common                                                  *
 ******************************************************************************/

typedef enum {
    KEYS,
    VALUES,
    ITEMS
} HashmapIteratorType_e;

typedef struct {
    PyObject_HEAD
    HashmapIteratorType_e iterator_type;
    size_t current_position;
    PyObject *obj;
} HashmapIterator_t;

static void HashmapIterator_dealloc(HashmapIterator_t *self) {
    if (self->obj != NULL) {
        Py_DECREF(self->obj);
    }
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* HashmapIterator_iter(PyObject *self) {
    Py_INCREF(self);
    return self;
}

/******************************************************************************
 * Int2Int class                                                              *
 ******************************************************************************/

typedef struct {
    PyObject_HEAD
    Int2IntHashTable_t *hashmap;
    PyObject *default_value;
} Int2Int_t;

static PyTypeObject Int2Int_type;

/* Int2Int iterator */

static PyObject* Int2IntIterator_next(HashmapIterator_t *self) {
    Int2Int_t *obj = (Int2Int_t*) self->obj;
    PyObject *res = NULL;

    while (self->current_position < obj->hashmap->table_size) {
        Int2IntItem_t item = obj->hashmap->table[self->current_position];
        if (item.status == USED) {
            switch (self->iterator_type) {
            case KEYS:
                res = PyLong_FromUnsignedLongLong(item.key);
                break;
            case VALUES:
                res = PyLong_FromSize_t(item.value);
                break;
            case ITEMS:
                res = PyTuple_Pack(2, PyLong_FromUnsignedLongLong(item.key),
                        PyLong_FromSize_t(item.value));
                break;
            }
        }
        ++self->current_position;
        if (res != NULL) {
            break;
        }
    }

    return res;
}

static PyTypeObject Int2IntIterator_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cdatastructs.hashmap.Int2IntIterator",
    .tp_doc = "Iterator over hashmap",
    .tp_basicsize = sizeof(HashmapIterator_t),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_iter = (getiterfunc) HashmapIterator_iter,
    .tp_iternext = (iternextfunc) Int2IntIterator_next,
    .tp_dealloc = (destructor) HashmapIterator_dealloc
};

/* Int2Int */

static PyObject* Int2Int_new(PyTypeObject *type,
        PyObject *args, PyObject *kwds) {

    char *kwnames[] = {"size", "default", NULL};
    Py_ssize_t size;
    PyObject *default_value = NULL;
    Int2Int_t *self;
    bool growable;
    size_t table_size;
    size_t int2int_memory_size;
    void *int2int_memory;

    /* Parse arguments */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "n|O", kwnames,
            &size, &default_value)) {
        return NULL;
    }
    if (default_value != NULL) {
        if (default_value == Py_None) {
            default_value = NULL;
        }
        else {
            if (!PyLong_Check(default_value)) {
                PyErr_SetString(PyExc_TypeError,
                        "'default' must be an integer");
                return NULL;
            }
            Py_INCREF(default_value);
        }
    }

    /* Create instance */
    self = (Int2Int_t*) type->tp_alloc(type, 0);
    if (!self) {
        return NULL;
    }

    /* Allocate memory for Int2IntHashTable_t structure and hashtable. At
       the beginning of block of the memory Int2IntHashTable_t structure
       is placed, followed by hashtable (array of Int2IntItem_t). */
    growable = size ? false : true;
    size = growable ? 1 : size;
    table_size = (size * 1.2) + 1;
    int2int_memory_size = sizeof(Int2IntHashTable_t) +
            (table_size * sizeof(Int2IntItem_t));
    int2int_memory = PyMem_RawMalloc(int2int_memory_size);
    if (!int2int_memory) {
        Py_TYPE(self)->tp_free((PyObject*) self);
        return PyErr_NoMemory();
    }
    memset(int2int_memory, 0, int2int_memory_size);

    /* Initialize object attributes */
    self->default_value = default_value;
    self->hashmap = (Int2IntHashTable_t*) int2int_memory;
    self->hashmap->size = size;
    self->hashmap->growable = growable;
    self->hashmap->current_size = 0;
    self->hashmap->table_size = table_size;
    self->hashmap->table = int2int_memory + sizeof(Int2IntHashTable_t);

    return (PyObject*) self;
}

static void Int2Int_dealloc(Int2Int_t *self) {
    if (self->default_value != NULL) {
        Py_DECREF(self->default_value);
    }
    PyMem_RawFree(self->hashmap);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* Int2Int_repr(Int2Int_t *self) {
    if (self->default_value != NULL) {
        if (self->hashmap->growable) {
            return PyUnicode_FromFormat(
                    "<%s: object at %p, used %zd, default %A>",
                    self->ob_base.ob_type->tp_name, self,
                    self->hashmap->current_size, self->default_value);
        } else {
            return PyUnicode_FromFormat(
                    "<%s: object at %p, used %zd/%zd, default %A>",
                    self->ob_base.ob_type->tp_name, self,
                    self->hashmap->current_size, self->hashmap->size,
                    self->default_value);
        }
    }
    if (self->hashmap->growable) {
        return PyUnicode_FromFormat("<%s: object at %p, used %zd>",
                self->ob_base.ob_type->tp_name, self,
                self->hashmap->current_size);
    } else {
        return PyUnicode_FromFormat("<%s: object at %p, used %zd/%zd>",
                self->ob_base.ob_type->tp_name, self,
                self->hashmap->current_size, self->hashmap->size);
    }
}

static Py_ssize_t Int2Int_len(Int2Int_t *self) {
    return self->hashmap->current_size;
}

static PyObject* Int2Int_richcompare(Int2Int_t *self, PyObject *other, int op) {
    PyObject *res = Py_False;

    /* Check supported operators */
    switch (op) {
    case Py_LT:
        PyErr_SetString(PyExc_TypeError, "'<' is not supported");
        res = NULL;
    case Py_LE:
        PyErr_SetString(PyExc_TypeError, "'<=' is not supported");
        res = NULL;
    case Py_GT:
        PyErr_SetString(PyExc_TypeError, "'>' is not supported");
        res = NULL;
    case Py_GE:
        PyErr_SetString(PyExc_TypeError, "'>=' is not supported");
        res = NULL;
    case Py_EQ:
    case Py_NE:
        break;
    }

    if (res != NULL) {
        if (PyDict_Check(other) || (Py_TYPE(other) == &Int2Int_type)) {
            Py_ssize_t other_length = PyMapping_Size(other);

            if (other_length == (Py_ssize_t) self->hashmap->current_size) {
                res = Py_True;
                for (size_t i = 0; i < self->hashmap->table_size; ++i) {
                    Int2IntItem_t item = self->hashmap->table[i];

                    if (item.status == USED) {
                        PyObject *key = NULL;
                        PyObject *value = NULL;
                        size_t c_value;

                        key = PyLong_FromLongLong(item.key);
                        if (key != NULL) {
                            value = PyObject_GetItem(other, key);
                            if (value != NULL) {
                                c_value = PyLong_AsSize_t(value);
                                if ((c_value != (size_t) -1) &&
                                        PyErr_Occurred() != NULL) {
                                    /* PyLong to size_t conversion error */
                                    res = NULL;
                                }
                                else {
                                    /* Values for key are different */
                                    if (item.value != c_value) {
                                        res = Py_False;
                                    }
                                }
                            }
                            else {
                                /* other[key] error, if KeyError, objects are
                                   different, otherwise return with error. */
                                if (PyErr_GivenExceptionMatches(
                                        PyErr_Occurred(), PyExc_KeyError)) {
                                    PyErr_Clear();
                                    res = Py_False;
                                }
                                else {
                                    res = NULL;
                                }
                            }
                        }
                        else {
                            /* long long to PyLong conversion error */
                            res = NULL;
                        }

                        Py_XDECREF(key);
                        Py_XDECREF(value);

                        if (res != Py_True) {
                            break;
                        }
                    }
                }
            }
        }
        else {
            /* other object is not Int2Int or dict (or subtype) */
            PyErr_SetString(PyExc_TypeError,
                    "'other' is not either an Int2Int or a dict");
            res = NULL;
        }
    }

    if (res != NULL) {
        if (op == Py_NE) {
            res = (res == Py_True) ? Py_False : Py_True;
        }
        Py_INCREF(res);
    }

    return res;
}

int Int2Int_contains(Int2Int_t *self, PyObject *key) {
    unsigned long long c_key;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    return int2int_has(self->hashmap, c_key);
}

static int Int2Int_setitem(Int2Int_t *self, PyObject *key, PyObject *value) {
    unsigned long long c_key;
    size_t c_value;
    size_t new_size;
    size_t new_table_size;
    size_t new_int2int_memory_size;
    void *new_int2int_memory;
    Int2IntHashTable_t *new_hashmap;
    Int2IntItem_t item;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    if (value == NULL) {
        PyErr_SetString(PyExc_NotImplementedError, "can't delete item");
        return -1;
    }
    if (!PyLong_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "'value' must be an integer");
        return -1;
    }
    c_value = PyLong_AsSize_t(value);
    if ((c_value == (size_t) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    // Resize hashmap if necessary
    if (self->hashmap->growable &&
            (self->hashmap->current_size == self->hashmap->size)) {
        new_size = self->hashmap->size * 2;
        new_table_size = (new_size * 1.2) + 1;
        new_int2int_memory_size = sizeof(Int2IntHashTable_t) +
                (new_table_size * sizeof(Int2IntItem_t));
        new_int2int_memory = PyMem_RawMalloc(new_int2int_memory_size);
        if (!new_int2int_memory) {
            PyErr_NoMemory();
            return -1;
        }
        memset(new_int2int_memory, 0, new_int2int_memory_size);

        new_hashmap = (Int2IntHashTable_t*) new_int2int_memory;
        new_hashmap->size = new_size;
        new_hashmap->growable = self->hashmap->growable;
        new_hashmap->current_size = 0;
        new_hashmap->table_size = new_table_size;
        new_hashmap->table = new_int2int_memory + sizeof(Int2IntHashTable_t);

        for (size_t i=0; i<self->hashmap->table_size; ++i) {
            item = self->hashmap->table[i];
            if (item.status == USED) {
                if (int2int_set(new_hashmap, item.key, item.value) == -1) {
                    PyMem_RawFree(new_hashmap);
                    PyErr_SetString(PyExc_RuntimeError, "Can't resize buffer");
                    return -1;
                }
            }
        }

        PyMem_RawFree(self->hashmap);
        self->hashmap = new_hashmap;
    }
    // Put item
    if (int2int_set(self->hashmap, c_key, c_value) == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Maximum size has been exceeded");
        return -1;
    }

    return 0;
}

static PyObject* Int2Int_getitem(Int2Int_t *self, PyObject *key) {
    unsigned long long c_key;
    size_t c_value;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2int_get(self->hashmap, c_key, &c_value) == -1) {
        if (self->default_value != NULL) {
            c_value = PyLong_AsSize_t(self->default_value);
            if ((c_value == (size_t) -1) && (PyErr_Occurred() != NULL)) {
                return NULL;
            }
            if (int2int_set(self->hashmap, c_key, c_value) == -1) {
                PyErr_SetString(PyExc_RuntimeError,
                        "Maximum size has been exceeded");
                return NULL;
            }
            Py_INCREF(self->default_value);
            return self->default_value;
        }
        return PyErr_Format(PyExc_KeyError, "%llu", c_key);
    }

    return PyLong_FromSize_t(c_value);
}

static PyObject* Int2Int_iter(Int2Int_t *self) {
    HashmapIterator_t *iterator = PyObject_New(
            HashmapIterator_t, &Int2IntIterator_type);
    if (iterator != NULL) {
        Py_INCREF(self);
        iterator->iterator_type = KEYS;
        iterator->current_position = 0;
        iterator->obj = (PyObject*) self;
    }
    return (PyObject*) iterator;
}

static PyObject* Int2Int_reduce(Int2Int_t *self) {
    PyObject *res = PyTuple_New(3);
    PyObject *callable = PyObject_GetAttrString((PyObject *) self, "__class__");
    PyObject *args = PyTuple_New(2);
    PyObject *state = PyTuple_New(2);

    if ((res != NULL) && (callable != NULL) &&
            (args != NULL) && (state != NULL)) {
        /* state */
        PyTuple_SET_ITEM(state, 0,
                PyLong_FromSize_t(self->hashmap->current_size));
        PyTuple_SET_ITEM(state, 1,
                PyBytes_FromStringAndSize((const char *) self->hashmap->table,
                        sizeof(Int2IntItem_t) * self->hashmap->table_size));
        /* callable args */
        PyTuple_SET_ITEM(args, 0, PyLong_FromSize_t(self->hashmap->size));  // TODO
        if (self->default_value != NULL) {
            Py_INCREF(self->default_value);
            PyTuple_SET_ITEM(args, 1, self->default_value);
        }
        else {
            Py_INCREF(Py_None);
            PyTuple_SET_ITEM(args, 1, Py_None);
        }
        /* res */
        PyTuple_SET_ITEM(res, 0, callable);
        PyTuple_SET_ITEM(res, 1, args);
        PyTuple_SET_ITEM(res, 2, state);
    }
    else {
        Py_XDECREF(state);
        Py_XDECREF(args);
        Py_XDECREF(callable);
        Py_CLEAR(res);
    }

    return res;
}

static PyObject* Int2Int_setstate(Int2Int_t *self, PyObject *args) {
    PyObject *res = NULL;
    PyObject *state;
    PyObject *current_size;
    PyObject *table;
    size_t table_mem_size;
    Py_buffer table_buffer = { .obj = NULL };

    if (!PyArg_ParseTuple(args, "O", &state)) {
        goto cleanup;
    }
    if (!PyTuple_Check(state)) {
        goto cleanup;
    }

    /* self->hashmap->current_size */
    current_size = PyTuple_GetItem(state, 0);
    if (current_size == NULL) {
        goto cleanup;
    }
    self->hashmap->current_size = PyLong_AsSize_t(current_size);
    if (self->hashmap->current_size == (size_t) -1) {
        goto cleanup;
    }

    /* self->hashmap->table */
    table = PyTuple_GetItem(state, 1);
    if ((table == NULL) || (!PyBytes_Check(table))) {
        goto cleanup;
    }
    if (PyObject_GetBuffer(table, &table_buffer, 0) == -1) {
        goto cleanup;
    }
    table_mem_size = PyBytes_Size(table);
    if (table_mem_size != (self->hashmap->table_size * sizeof(Int2IntItem_t))) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid pickled state");
        goto cleanup;
    }
    memcpy((void *) self->hashmap->table, table_buffer.buf, table_mem_size);
    res = Py_None;

cleanup:
    if (table_buffer.obj != NULL) {
        PyBuffer_Release(&table_buffer);
    }

    Py_XINCREF(res);
    return res;
}

static PyObject* Int2Int_get(Int2Int_t *self, PyObject *args, PyObject *kwds) {
    char *kwnames[] = {"key", "default", NULL};
    PyObject * key;
    PyObject * default_value = NULL;
    unsigned long long c_key;
    size_t c_default_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwnames,
            &key, &default_value)) {
        return NULL;
    }

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2int_get(self->hashmap, c_key, &c_default_value) == -1) {
        if (default_value == NULL) {
            Py_INCREF(Py_None);
            return Py_None;
        }
        if (!PyLong_Check(default_value)) {
            PyErr_SetString(PyExc_TypeError, "'default' must be an integer");
            return NULL;
        }
        c_default_value = PyLong_AsSize_t(default_value);
        if ((c_default_value == (size_t) -1) && (PyErr_Occurred() != NULL)) {
            return NULL;
        }

        Py_INCREF(default_value);
        return default_value;
    }

    return PyLong_FromSize_t(c_default_value);
}

static PyObject* Int2Int_keys(Int2Int_t *self) {
    HashmapIterator_t *iterator = PyObject_New(
            HashmapIterator_t, &Int2IntIterator_type);
    if (iterator != NULL) {
        Py_INCREF(self);
        iterator->iterator_type = KEYS;
        iterator->current_position = 0;
        iterator->obj = (PyObject*) self;
    }
    return (PyObject*) iterator;
}

static PyObject* Int2Int_values(Int2Int_t *self) {
    HashmapIterator_t *iterator = PyObject_New(
            HashmapIterator_t, &Int2IntIterator_type);
    if (iterator != NULL) {
        Py_INCREF(self);
        iterator->iterator_type = VALUES;
        iterator->current_position = 0;
        iterator->obj = (PyObject*) self;
    }
    return (PyObject*) iterator;
}

static PyObject* Int2Int_items(Int2Int_t *self) {
    HashmapIterator_t *iterator = PyObject_New(
            HashmapIterator_t, &Int2IntIterator_type);
    if (iterator != NULL) {
        Py_INCREF(self);
        iterator->iterator_type = ITEMS;
        iterator->current_position = 0;
        iterator->obj = (PyObject*) self;
    }
    return (PyObject*) iterator;
}

static PyObject* Int2Int_get_ptr(Int2Int_t *self) {
    return PyLong_FromVoidPtr(self->hashmap);
}

static PyObject* Int2Int_get_max_size(Int2Int_t *self) {
    if (!self->hashmap->growable) {
        return PyLong_FromSize_t(self->hashmap->size);
    }
    else {
        Py_RETURN_NONE;
    }
}

static PyObject* Int2Int_get_growable(Int2Int_t *self) {
    if (self->hashmap->growable) {
    	Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
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
    {"keys", (PyCFunction) Int2Int_keys, METH_VARARGS,
            "keys(self, /)\n"
            "--\n"
            "\n"
            "Return an iterator over the hashmaps’s keys. Don't change\n"
            "hashmap during iteration, behavior is undefined!"},
    {"values", (PyCFunction) Int2Int_values, METH_VARARGS,
            "values(self, /)\n"
            "--\n"
            "\n"
            "Return an iterator over the hashmaps’s values. Don't change\n"
            "hashmap during iteration, behavior is undefined!"},
    {"items", (PyCFunction) Int2Int_items, METH_VARARGS,
            "items(self, /)\n"
            "--\n"
            "\n"
            "Return an iterator over the hashmaps’s (key, value) tuple\n"
            "pairs. Don't change hashmap during iteration, behavior is\n"
            "undefined!"},
    {"get_ptr", (PyCFunction) Int2Int_get_ptr, METH_NOARGS,
            "get_ptr(self, /)\n"
            "--\n"
            "\n"
            "Return pointer to internal Int2IntHashTable_t structure."},
    {"__reduce__", (PyCFunction) Int2Int_reduce, METH_NOARGS,
            "__setstate__(self, /)\n"
            "--\n"
            "\n"
            "Return state information for pickling."},
    {"__setstate__", (PyCFunction) Int2Int_setstate, METH_VARARGS,
            "__setstate__(self, state, /)\n"
            "--\n"
            "\n"
            "Restore object state when unpickling."},
    {NULL}
};

static PyGetSetDef Int2Int_getset[] = {
    {"max_size", (getter) Int2Int_get_max_size, NULL,
            "upperbound limit on the number of items that can be placed "
            "in the mapping\n", NULL},
    {"growable", (getter) Int2Int_get_growable, NULL,
            "flag that indicates that structure is either growable or has "
            "fixed size\n", NULL},
    {NULL}
};

static PyTypeObject Int2Int_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cdatastructs.hashmap.Int2Int",                     /* tp_name */
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
    (richcmpfunc) Int2Int_richcompare,                  /* tp_richcompare */
    0,                                                  /* tp_weaklistoffset */
    (getiterfunc) Int2Int_iter,                         /* tp_iter */
    0,                                                  /* tp_iternext */
    Int2Int_methods,                                    /* tp_methods */
    0,                                                  /* tp_members */
    Int2Int_getset,                                     /* tp_getset */
    0,                                                  /* tp_base */
    0,                                                  /* tp_dict */
    0,                                                  /* tp_descr_get */
    0,                                                  /* tp_descr_set */
    0,                                                  /* tp_dictoffset */
    0,                                                  /* tp_init */
    0,                                                  /* tp_alloc */
    (newfunc) Int2Int_new,                              /* tp_new */
};

/******************************************************************************
 * Int2Float class                                                            *
 ******************************************************************************/

typedef struct {
    PyObject_HEAD
    Int2FloatHashTable_t *hashmap;
    PyObject *default_value;
} Int2Float_t;

static PyTypeObject Int2Float_type;

/* Int2Float iterator */

static PyObject* Int2FloatIterator_next(HashmapIterator_t *self) {
    Int2Float_t *obj = (Int2Float_t*) self->obj;
    PyObject *res = NULL;

    while (self->current_position < obj->hashmap->table_size) {
        Int2FloatItem_t item = obj->hashmap->table[self->current_position];
        if (item.status == USED) {
            switch (self->iterator_type) {
            case KEYS:
                res = PyLong_FromUnsignedLongLong(item.key);
                break;
            case VALUES:
                res = PyFloat_FromDouble(item.value);
                break;
            case ITEMS:
                res = PyTuple_Pack(2, PyLong_FromUnsignedLongLong(item.key),
                        PyFloat_FromDouble(item.value));
                break;
            }
        }
        ++self->current_position;
        if (res != NULL) {
            break;
        }
    }

    return res;
}

static PyTypeObject Int2FloatIterator_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cdatastructs.hashmap.Int2FloatIterator",
    .tp_doc = "Iterator over hashmap",
    .tp_basicsize = sizeof(HashmapIterator_t),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_iter = (getiterfunc) HashmapIterator_iter,
    .tp_iternext = (iternextfunc) Int2FloatIterator_next,
    .tp_dealloc = (destructor) HashmapIterator_dealloc
};

/* Int2Float */

static PyObject* Int2Float_new(PyTypeObject *type,
        PyObject *args, PyObject *kwds) {

    char *kwnames[] = {"size", "default", NULL};
    const Py_ssize_t size;
    PyObject *default_value = NULL;
    Int2Float_t *self;
    size_t table_size;
    size_t int2float_memory_size;
    void *int2float_memory;

    /* Parse arguments */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "n|O", kwnames,
            &size, &default_value)) {
        return NULL;
    }
    if (default_value != NULL) {
        if (default_value == Py_None) {
            default_value = NULL;
        }
        else {
            if (PyLong_Check(default_value)) {
                double c_val = PyLong_AsDouble(default_value);
                if ((c_val == -1.0) && (PyErr_Occurred() != NULL)) {
                    return NULL;
                }
                default_value = PyFloat_FromDouble(c_val);
                if (default_value == NULL) {
                    return NULL;
                }
            }
            else {
                if (!PyFloat_Check(default_value)) {
                    PyErr_SetString(PyExc_TypeError,
                            "'default' must be a float");
                    return NULL;
                }
                Py_INCREF(default_value);
            }
        }
    }

    /* Create instance */
    self = (Int2Float_t*) type->tp_alloc(type, 0);
    if (!self) {
        return NULL;
    }

    /* Allocate memory for Int2FloatHashTable_t structure and hashtable. At
       the beginning of block of the memory Int2FloatHashTable_t structure
       is placed, followed by hashtable (array of Int2FloatItem_t). */
    table_size = (size * 1.2) + 1;;
    int2float_memory_size = sizeof(Int2FloatHashTable_t) +
            (table_size * sizeof(Int2FloatItem_t));
    int2float_memory = PyMem_RawMalloc(int2float_memory_size);
    if (!int2float_memory) {
        Py_TYPE(self)->tp_free((PyObject*) self);
        return PyErr_NoMemory();
    }
    memset(int2float_memory, 0, int2float_memory_size);

    /* Initialize object attributes */
    self->default_value = default_value;
    self->hashmap = (Int2FloatHashTable_t*) int2float_memory;
    self->hashmap->size = size;
    self->hashmap->current_size = 0;
    self->hashmap->table_size = table_size;
    self->hashmap->table = int2float_memory + sizeof(Int2FloatHashTable_t);

    return (PyObject*) self;
}

static void Int2Float_dealloc(Int2Float_t *self) {
    if (self->default_value != NULL) {
        Py_DECREF(self->default_value);
    }
    PyMem_RawFree(self->hashmap);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* Int2Float_repr(Int2Float_t *self) {
    if (self->default_value != NULL) {
        return PyUnicode_FromFormat(
                "<%s: object at %p, used %zd/%zd, default %A>",
                self->ob_base.ob_type->tp_name, self,
                self->hashmap->current_size, self->hashmap->size,
                self->default_value);
    }
    return PyUnicode_FromFormat("<%s: object at %p, used %zd/%zd>",
            self->ob_base.ob_type->tp_name, self,
            self->hashmap->current_size, self->hashmap->size);
}

static Py_ssize_t Int2Float_len(Int2Float_t *self) {
    return self->hashmap->current_size;
}

static PyObject* Int2Float_richcompare(Int2Float_t *self,
        PyObject *other, int op) {
    PyObject *res = Py_False;

    /* Check supported operators */
    switch (op) {
    case Py_LT:
        PyErr_SetString(PyExc_TypeError, "'<' is not supported");
        res = NULL;
    case Py_LE:
        PyErr_SetString(PyExc_TypeError, "'<=' is not supported");
        res = NULL;
    case Py_GT:
        PyErr_SetString(PyExc_TypeError, "'>' is not supported");
        res = NULL;
    case Py_GE:
        PyErr_SetString(PyExc_TypeError, "'>=' is not supported");
        res = NULL;
    case Py_EQ:
    case Py_NE:
        break;
    }

    if (res != NULL) {
        if (PyDict_Check(other) || (Py_TYPE(other) == &Int2Float_type)) {
            Py_ssize_t other_length = PyMapping_Size(other);

            if (other_length == (Py_ssize_t) self->hashmap->current_size) {
                res = Py_True;
                for (size_t i = 0; i < self->hashmap->table_size; ++i) {
                    Int2FloatItem_t item = self->hashmap->table[i];

                    if (item.status == USED) {
                        PyObject *key = NULL;
                        PyObject *value = NULL;
                        double c_value;

                        key = PyLong_FromLongLong(item.key);
                        if (key != NULL) {
                            value = PyObject_GetItem(other, key);
                            if (value != NULL) {
                                c_value = PyFloat_AsDouble(value);
                                if ((c_value == -1.0) &&
                                        PyErr_Occurred() != NULL) {
                                    /* PyLong to size_t conversion error */
                                    res = NULL;
                                }
                                else {
                                    /* Values for key are different */
                                    if (item.value != c_value) {
                                        res = Py_False;
                                    }
                                }
                            }
                            else {
                                /* other[key] error, if KeyError, objects are
                                   different, otherwise return with error. */
                                if (PyErr_GivenExceptionMatches(
                                        PyErr_Occurred(), PyExc_KeyError)) {
                                    PyErr_Clear();
                                    res = Py_False;
                                }
                                else {
                                    res = NULL;
                                }
                            }
                        }
                        else {
                            /* long long to PyLong conversion error */
                            res = NULL;
                        }

                        Py_XDECREF(key);
                        Py_XDECREF(value);

                        if (res != Py_True) {
                            break;
                        }
                    }
                }
            }
        }
        else {
            /* other object is not Int2Float or dict (or subtype) */
            PyErr_SetString(PyExc_TypeError,
                    "'other' is not either an Int2Float or a dict");
            res = NULL;
        }
    }

    if (res != NULL) {
        if (op == Py_NE) {
            res = (res == Py_True) ? Py_False : Py_True;
        }
        Py_INCREF(res);
    }

    return res;
}

int Int2Float_contains(Int2Float_t *self, PyObject *key) {
    unsigned long long c_key;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    return int2float_has(self->hashmap, c_key);
}

static int Int2Float_setitem(Int2Float_t *self,
        PyObject *key, PyObject *value) {
    unsigned long long c_key;
    double c_value;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    if (value == NULL) {
        PyErr_SetString(PyExc_NotImplementedError, "can't delete item");
        return -1;
    }
    c_value = PyFloat_AsDouble(value);
    if ((c_value == -1.0) && (PyErr_Occurred() != NULL)) {
        PyErr_SetString(PyExc_TypeError, "'value' must be a float");
        return -1;
    }

    if (int2float_set(self->hashmap, c_key, c_value) == -1) {
        PyErr_SetString(PyExc_RuntimeError, "Maximum size has been exceeded");
        return -1;
    }

    return 0;
}

static PyObject* Int2Float_getitem(Int2Float_t *self, PyObject *key) {
    unsigned long long c_key;
    double c_value;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2float_get(self->hashmap, c_key, &c_value) == -1) {
        if (self->default_value != NULL) {
            c_value = PyFloat_AsDouble(self->default_value);
            if ((c_value == -1.0) && (PyErr_Occurred() != NULL)) {
                return NULL;
            }
            if (int2float_set(self->hashmap, c_key, c_value) == -1) {
                PyErr_SetString(PyExc_RuntimeError,
                        "Maximum size has been exceeded");
                return NULL;
            }
            Py_INCREF(self->default_value);
            return self->default_value;
        }
        return PyErr_Format(PyExc_KeyError, "%llu", c_key);
    }

    return PyFloat_FromDouble(c_value);
}

static PyObject* Int2Float_iter(Int2Float_t *self) {
    HashmapIterator_t *iterator = PyObject_New(
            HashmapIterator_t, &Int2FloatIterator_type);
    if (iterator != NULL) {
        Py_INCREF(self);
        iterator->iterator_type = KEYS;
        iterator->current_position = 0;
        iterator->obj = (PyObject*) self;
    }
    return (PyObject*) iterator;
}

static PyObject* Int2Float_reduce(Int2Float_t *self) {
    PyObject *res = PyTuple_New(3);
    PyObject *callable = PyObject_GetAttrString((PyObject *) self, "__class__");
    PyObject *args = PyTuple_New(2);
    PyObject *state = PyTuple_New(2);

    if ((res != NULL) && (callable != NULL) &&
            (args != NULL) && (state != NULL)) {
        /* state */
        PyTuple_SET_ITEM(state, 0,
                PyLong_FromSize_t(self->hashmap->current_size));
        PyTuple_SET_ITEM(state, 1,
                PyBytes_FromStringAndSize((const char *) self->hashmap->table,
                        sizeof(Int2FloatItem_t) * self->hashmap->table_size));
        /* callable args */
        PyTuple_SET_ITEM(args, 0, PyLong_FromSize_t(self->hashmap->size));
        if (self->default_value != NULL) {
            Py_INCREF(self->default_value);
            PyTuple_SET_ITEM(args, 1, self->default_value);
        }
        else {
            Py_INCREF(Py_None);
            PyTuple_SET_ITEM(args, 1, Py_None);
        }
        /* res */
        PyTuple_SET_ITEM(res, 0, callable);
        PyTuple_SET_ITEM(res, 1, args);
        PyTuple_SET_ITEM(res, 2, state);
    }
    else {
        Py_XDECREF(state);
        Py_XDECREF(args);
        Py_XDECREF(callable);
        Py_CLEAR(res);
    }

    return res;
}

static PyObject* Int2Float_setstate(Int2Float_t *self, PyObject *args) {
    PyObject *res = NULL;
    PyObject *state;
    PyObject *current_size;
    PyObject *table;
    size_t table_mem_size;
    Py_buffer table_buffer = { .obj = NULL };

    if (!PyArg_ParseTuple(args, "O", &state)) {
        goto cleanup;
    }
    if (!PyTuple_Check(state)) {
        goto cleanup;
    }

    /* self->hashmap->current_size */
    current_size = PyTuple_GetItem(state, 0);
    if (current_size == NULL) {
        goto cleanup;
    }
    self->hashmap->current_size = PyLong_AsSize_t(current_size);
    if (self->hashmap->current_size == (size_t) -1) {
        goto cleanup;
    }

    /* self->hashmap->table */
    table = PyTuple_GetItem(state, 1);
    if ((table == NULL) || (!PyBytes_Check(table))) {
        goto cleanup;
    }
    if (PyObject_GetBuffer(table, &table_buffer, 0) == -1) {
        goto cleanup;
    }
    table_mem_size = PyBytes_Size(table);
    if (table_mem_size !=
            (self->hashmap->table_size * sizeof(Int2FloatItem_t))) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid pickled state");
        goto cleanup;
    }
    memcpy((void *) self->hashmap->table, table_buffer.buf, table_mem_size);
    res = Py_None;

cleanup:
    if (table_buffer.obj != NULL) {
        PyBuffer_Release(&table_buffer);
    }

    Py_XINCREF(res);
    return res;
}

static PyObject* Int2Float_get(Int2Float_t *self,
        PyObject *args, PyObject *kwds) {
    char *kwnames[] = {"key", "default", NULL};
    PyObject * key;
    PyObject * default_value = NULL;
    unsigned long long c_key;
    double value;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwnames,
            &key, &default_value)) {
        return NULL;
    }

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2float_get(self->hashmap, c_key, &value) == -1) {
        if (default_value == NULL) {
            Py_INCREF(Py_None);
            return Py_None;
        }
        if (PyLong_Check(default_value)) {
            double c_val = PyLong_AsDouble(default_value);
            if ((c_val == -1.0) && (PyErr_Occurred() != NULL)) {
                return NULL;
            }
            default_value = PyFloat_FromDouble(c_val);
            if (default_value == NULL) {
                return NULL;
            }
        }
        else {
            if (!PyFloat_Check(default_value)) {
                PyErr_SetString(PyExc_TypeError, "'default' must be a float");
                return NULL;
            }
            Py_INCREF(default_value);
        }
        return default_value;
    }

    return PyFloat_FromDouble(value);
}

static PyObject* Int2Float_keys(Int2Float_t *self) {
    HashmapIterator_t *iterator = PyObject_New(
            HashmapIterator_t, &Int2FloatIterator_type);
    if (iterator != NULL) {
        Py_INCREF(self);
        iterator->iterator_type = KEYS;
        iterator->current_position = 0;
        iterator->obj = (PyObject*) self;
    }
    return (PyObject*) iterator;
}

static PyObject* Int2Float_values(Int2Float_t *self) {
    HashmapIterator_t *iterator = PyObject_New(
            HashmapIterator_t, &Int2FloatIterator_type);
    if (iterator != NULL) {
        Py_INCREF(self);
        iterator->iterator_type = VALUES;
        iterator->current_position = 0;
        iterator->obj = (PyObject*) self;
    }
    return (PyObject*) iterator;
}

static PyObject* Int2Float_items(Int2Float_t *self) {
    HashmapIterator_t *iterator = PyObject_New(
            HashmapIterator_t, &Int2FloatIterator_type);
    if (iterator != NULL) {
        Py_INCREF(self);
        iterator->iterator_type = ITEMS;
        iterator->current_position = 0;
        iterator->obj = (PyObject*) self;
    }
    return (PyObject*) iterator;
}

static PyObject* Int2Float_get_ptr(Int2Float_t *self) {
    return PyLong_FromVoidPtr(self->hashmap);
}

static PyObject* Int2Float_get_max_size(Int2Float_t *self) {
    return PyLong_FromSize_t(self->hashmap->size);
}

static PySequenceMethods Int2Float_sequence_methods = {
    0,                                                  /* sq_length */
    0,                                                  /* sq_concat */
    0,                                                  /* sq_repeat */
    0,                                                  /* sq_item */
    0,                                                  /* sq_slice */
    0,                                                  /* sq_ass_item */
    0,                                                  /* sq_ass_slice */
    (objobjproc) Int2Float_contains,                    /* sq_contains */
    0,                                                  /* sq_inplace_concat */
    0,                                                  /* sq_inplace_repeat */
};

static PyMappingMethods Int2Float_mapping_methods = {
    (lenfunc) Int2Float_len,                            /* mp_length */
    (binaryfunc) Int2Float_getitem,                     /* mp_subscript */
    (objobjargproc) Int2Float_setitem,                  /* mp_ass_subscript */
};

static PyMethodDef Int2Float_methods[] = {
    {"get", (PyCFunction) Int2Float_get, METH_VARARGS | METH_KEYWORDS,
            "get(self, key, default=None, /)\n"
            "--\n"
            "\n"
            "Return value for key. If key does not exist, return default\n"
            "value, otherwise return None."},
    {"keys", (PyCFunction) Int2Float_keys, METH_VARARGS,
            "keys(self, /)\n"
            "--\n"
            "\n"
            "Return an iterator over the hashmaps’s keys. Don't change\n"
            "hashmap during iteration, behavior is undefined!"},
    {"values", (PyCFunction) Int2Float_values, METH_VARARGS,
            "values(self, /)\n"
            "--\n"
            "\n"
            "Return an iterator over the hashmaps’s values. Don't change\n"
            "hashmap during iteration, behavior is undefined!"},
    {"items", (PyCFunction) Int2Float_items, METH_VARARGS,
            "items(self, /)\n"
            "--\n"
            "\n"
            "Return an iterator over the hashmaps’s (key, value) tuple\n"
            "pairs. Don't change hashmap during iteration, behavior is\n"
            "undefined!"},
    {"get_ptr", (PyCFunction) Int2Float_get_ptr, METH_NOARGS,
            "get_ptr(self, /)\n"
            "--\n"
            "\n"
            "Return pointer to internal Int2FloatHashTable_t structure."},
    {"__reduce__", (PyCFunction) Int2Float_reduce, METH_NOARGS,
            "__setstate__(self, /)\n"
            "--\n"
            "\n"
            "Return state information for pickling."},
    {"__setstate__", (PyCFunction) Int2Float_setstate, METH_VARARGS,
            "__setstate__(self, state, /)\n"
            "--\n"
            "\n"
            "Restore object state when unpickling."},
    {NULL}
};

static PyGetSetDef Int2Float_getset[] = {
    {"max_size", (getter) Int2Float_get_max_size, NULL,
            "upperbound limit on the number of items that can be placed "
            "in the mapping\n", NULL},
    {NULL}
};

static PyTypeObject Int2Float_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cdatastructs.hashmap.Int2Float",                   /* tp_name */
    sizeof(Int2Float_t),                                /* tp_basicsize */
    0,                                                  /* tp_itemsize */
    (destructor) Int2Float_dealloc,                     /* tp_dealloc */
    0,                                                  /* tp_print */
    0,                                                  /* tp_getattr */
    0,                                                  /* tp_setattr */
    0,                                                  /* tp_compare */
    (reprfunc) Int2Float_repr,                          /* tp_repr */
    0,                                                  /* tp_as_number */
    &Int2Float_sequence_methods,                        /* tp_as_sequence */
    &Int2Float_mapping_methods,                         /* tp_as_mapping */
    0,                                                  /* tp_hash */
    0,                                                  /* tp_call */
    0,                                                  /* tp_str */
    0,                                                  /* tp_getattro */
    0,                                                  /* tp_setattro */
    0,                                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /* tp_flags */
    "Simple fixed-size hashmap which maps int key "     /* tp_doc */
    "to float value.\n"
    "\n"
    "Provides pointer to internal C structure and set/get/has functions,\n"
    "so hashmap is accesible from pure C. Easily fill data in Python and\n"
    "compute in C or Cython.\n",
    0,                                                  /* tp_traverse */
    0,                                                  /* tp_clear */
    (richcmpfunc) Int2Float_richcompare,                /* tp_richcompare */
    0,                                                  /* tp_weaklistoffset */
    (getiterfunc) Int2Float_iter,                       /* tp_iter */
    0,                                                  /* tp_iternext */
    Int2Float_methods,                                  /* tp_methods */
    0,                                                  /* tp_members */
    Int2Float_getset,                                   /* tp_getset */
    0,                                                  /* tp_base */
    0,                                                  /* tp_dict */
    0,                                                  /* tp_descr_get */
    0,                                                  /* tp_descr_set */
    0,                                                  /* tp_dictoffset */
    0,                                                  /* tp_init */
    0,                                                  /* tp_alloc */
    (newfunc) Int2Float_new,                            /* tp_new */
};

/******************************************************************************
 * hashmap module                                                             *
 ******************************************************************************/

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

    if ((PyType_Ready(&Int2IntIterator_type) < 0) ||
            (PyType_Ready(&Int2Int_type) < 0)) {
        return NULL;
    }

    if ((PyType_Ready(&Int2FloatIterator_type) < 0) ||
            (PyType_Ready(&Int2Float_type) < 0)) {
        return NULL;
    }

    module = PyModule_Create(&hashmapmodule);
    if (module == NULL) {
        return NULL;
    }

    Py_INCREF(&Int2Int_type);
    PyModule_AddObject(module, "Int2Int", (PyObject*) &Int2Int_type);

    Py_INCREF(&Int2Float_type);
    PyModule_AddObject(module, "Int2Float", (PyObject*) &Int2Float_type);

    return module;
}
