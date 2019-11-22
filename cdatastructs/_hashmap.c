
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>
#include <stddef.h>

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
    Int2IntItem_t *table;
    PyObject *default_value;
    bool release_memory;
} Int2Int_t;

static PyTypeObject Int2Int_type;

/* Int2Int iterator */

static PyObject* Int2IntIterator_next(HashmapIterator_t *self) {
    Int2Int_t *obj = (Int2Int_t*) self->obj;
    PyObject *res = NULL;

    while (self->current_position < obj->hashmap->table_size) {
        Int2IntItem_t item = obj->table[self->current_position];
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

static int Int2Int_update_from_initializer(Int2Int_t *self,
        PyObject *initializer);

static PyObject* Int2Int_new(PyTypeObject *cls,
        PyObject *args, PyObject *kwds) {

    char *kwnames[] = {"initializer", "default", "prealloc_size", NULL};
    PyObject *initializer = NULL;
    PyObject *default_value = Py_None;
    unsigned int prealloc_size = INT2INT_INITIAL_SIZE;
    Int2Int_t *self = NULL;

    /* Parse arguments */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O$OI", kwnames,
            &initializer, &default_value, &prealloc_size)) {
        goto error;
    }
    /* Validate arguments */
    if ((default_value != Py_None)
            && (!PyLong_Check(default_value) ||
                    ((PyLong_AsSize_t(default_value) == (size_t) -1)
                            && PyErr_Occurred()))) {
        PyErr_SetString(PyExc_TypeError, "'default' must be positive int");
        goto error;
    }

    /* Create instance */
    if (NULL == (self = (Int2Int_t*) cls->tp_alloc(cls, 0))) {
        goto error;
    }

    /* Allocate memory for Int2IntHashTable_t structure and hashtable. At
       the beginning of block of the memory Int2IntHashTable_t structure
       is placed, followed by hashtable (array of Int2IntItem_t). */
    if (int2int_new(prealloc_size, &self->hashmap)) {
        PyErr_NoMemory();
        goto error;
    }
    /* Initialize object attributes */
    self->release_memory = true;
    self->default_value = default_value;
    self->table = (void*) self->hashmap + sizeof(Int2IntHashTable_t);

    if ((NULL != initializer) &&
            (Int2Int_update_from_initializer(self, initializer) != 0)) {
        goto error;
    }

    Py_INCREF(self->default_value);
    return (PyObject*) self;

error:
    if (NULL != self) {
        cls->tp_free((PyObject*) self);
    }

    return NULL;
}

static void Int2Int_dealloc(Int2Int_t *self) {
    Py_DECREF(self->default_value);
    if (self->release_memory && (NULL != self->hashmap)) {
        free(self->hashmap);
    }
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* Int2Int_repr(Int2Int_t *self) {
    if (self->hashmap->readonly) {
        return PyUnicode_FromFormat("<%s: object at %p, used %zd, read-only>",
                self->ob_base.ob_type->tp_name, self,
                self->hashmap->current_size);
    } else {
        if (self->default_value == Py_None) {
            return PyUnicode_FromFormat(
                    "<%s: object at %p, used %zd>",
                    self->ob_base.ob_type->tp_name, self,
                    self->hashmap->current_size);
        } else {
            return PyUnicode_FromFormat(
                    "<%s: object at %p, used %zd, default %A>",
                    self->ob_base.ob_type->tp_name, self,
                    self->hashmap->current_size, self->default_value);
        }
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
        break;
    case Py_LE:
        PyErr_SetString(PyExc_TypeError, "'<=' is not supported");
        res = NULL;
        break;
    case Py_GT:
        PyErr_SetString(PyExc_TypeError, "'>' is not supported");
        res = NULL;
        break;
    case Py_GE:
        PyErr_SetString(PyExc_TypeError, "'>=' is not supported");
        res = NULL;
        break;
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
                    Int2IntItem_t item = self->table[i];

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

static int Int2Int_contains(Int2Int_t *self, PyObject *key) {
    unsigned long long c_key;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    return int2int_has(self->hashmap, c_key) == -1 ? 0 : 1;
}

static int Int2Int_setitem(Int2Int_t *self, PyObject *key, PyObject *value) {
    unsigned long long c_key;
    size_t c_value;
    Int2IntHashTable_t * new_hashmap;

    if (self->hashmap->readonly) {
        PyErr_SetString(PyExc_RuntimeError, "Instance is read-only");
        return -1;
    }
    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    if (value == NULL) {
        /* Delete item */
        if (int2int_del(self->hashmap, c_key) == -1) {
            Py_INCREF(key);
            PyErr_SetObject(PyExc_KeyError, key);
            return -1;
        }
    }
    else {
        /* Set new or update existing item */
        if (!PyLong_Check(value)) {
            PyErr_SetString(PyExc_TypeError, "'value' must be an integer");
            return -1;
        }
        c_value = PyLong_AsSize_t(value);
        if ((c_value == (size_t) -1) && (PyErr_Occurred() != NULL)) {
            return -1;
        }

        if (int2int_set(self->hashmap, c_key, c_value, &new_hashmap)) {
            PyErr_NoMemory();
            return -1;
        }
        if (new_hashmap != self->hashmap) {
            self->hashmap = new_hashmap;
            self->table = (void*) new_hashmap + sizeof(Int2IntHashTable_t);
        }
    }

    return 0;
}

static PyObject* Int2Int_getitem(Int2Int_t *self, PyObject *key) {
    unsigned long long c_key;
    size_t c_value;
    Int2IntHashTable_t * new_hashmap;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (NULL != PyErr_Occurred())) {
        return NULL;
    }

    if (int2int_get(self->hashmap, c_key, &c_value) == -1) {
        if (self->default_value != Py_None) {
            c_value = PyLong_AsSize_t(self->default_value);
            if ((c_value == (size_t) -1) && (NULL != PyErr_Occurred())) {
                return NULL;
            }
            if (int2int_set(self->hashmap, c_key, c_value, &new_hashmap)) {
                PyErr_NoMemory();
                return NULL;
            }
            if (new_hashmap != self->hashmap) {
                self->hashmap = new_hashmap;
                self->table = (void*) new_hashmap + sizeof(Int2IntHashTable_t);
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

static PyObject* Int2Int_get(Int2Int_t *self, PyObject *args, PyObject *kwds) {
    PyObject * key;
    PyObject * default_value = Py_None;
    unsigned long long c_key;
    size_t value;

    if (!PyArg_ParseTuple(args, "O|O", &key, &default_value)) {
        return NULL;
    }
    /* key argument */
    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2int_get(self->hashmap, c_key, &value) == -1) {
        if (default_value != Py_None) {
            if (!PyLong_Check(default_value)) {
                PyErr_SetString(PyExc_TypeError,
                        "'default' must be positive int");
                return NULL;
            }
            if ((PyLong_AsSize_t(default_value) == (size_t) -1)
                    && PyErr_Occurred()) {
                return NULL;
            }
        }

        Py_INCREF(default_value);
        return default_value;
    }

    return PyLong_FromSize_t(value);
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

static PyObject* Int2Int_pop(Int2Int_t *self, PyObject *args) {
    PyObject * key;
    PyObject * default_value = NULL;
    unsigned long long c_key;
    size_t value;

    if (!PyArg_ParseTuple(args, "O|O", &key, &default_value)) {
        return NULL;
    }
    /* key argument */
    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2int_get(self->hashmap, c_key, &value) == -1) {
        if (NULL != default_value) {
            if ((!PyLong_Check(default_value)) && (default_value != Py_None)) {
                PyErr_SetString(PyExc_TypeError,
                        "'default' must be positive int or None");
                return NULL;
            }
            if ((default_value != Py_None) &&
                    (PyLong_AsSize_t(default_value) == (size_t) -1) &&
                    PyErr_Occurred()) {
                return NULL;
            }
            Py_INCREF(default_value);
            return default_value;
        }
        PyErr_SetObject(PyExc_KeyError, key);
        return NULL;
    }

    int2int_del(self->hashmap, c_key);
    return PyLong_FromSize_t(value);
}

static PyObject* Int2Int_popitem(Int2Int_t *self) {
    Int2IntItem_t * item;
    PyObject * res = NULL;
    PyObject * key = NULL;
    PyObject * value = NULL;

    for (size_t i=0; i<self->hashmap->table_size; ++i) {
        item = &(self->table[i]);
        if (USED == item->status) {
            if (NULL == (key = PyLong_FromUnsignedLongLong(item->key))) {
                goto error;
            }
            if (NULL == (value = PyLong_FromSize_t(item->value))) {
                goto error;
            }
            if (NULL == (res = PyTuple_New(2))) {
                goto error;
            }
            PyTuple_SET_ITEM(res, 0, key);
            PyTuple_SET_ITEM(res, 1, value);

            item->status = DELETED;
            self->hashmap->current_size -= 1;

            return res;
        }
    }
    PyErr_SetString(PyExc_KeyError, "popitem(): mapping is empty");

error:
    Py_XDECREF(res);
    Py_XDECREF(key);
    Py_XDECREF(value);

    return NULL;
}

static PyObject* Int2Int_clear(Int2Int_t *self) {
    for (size_t i=0; i<self->hashmap->table_size; ++i) {
        self->table[i].status = EMPTY;
    }
    self->hashmap->current_size = 0;

    Py_RETURN_NONE;
}

static int Int2Int_update_from_initializer(Int2Int_t *self,
        PyObject *initializer) {
    PyObject * pairs = NULL;
    PyObject * pairs_it = NULL;
    PyObject * pair = NULL;
    PyObject * item = NULL;
    int res = -1;

    /* No 'other' argument */
    if (NULL == initializer) {
        res = 0;
        goto cleanup;
    }

    /* 'other' is mapping */
    if (NULL != (pairs = PyMapping_Items(initializer))) {
        if (NULL != (pairs_it = PyObject_GetIter(pairs))) {
            while (NULL != (pair = PyIter_Next(pairs_it))) {
                if (Int2Int_setitem(self, PyTuple_GET_ITEM(pair, 0),
                        PyTuple_GET_ITEM(pair, 1))) {
                    goto cleanup;
                }
                Py_CLEAR(pair);
            }
            if (PyErr_Occurred()) {
                goto cleanup;
            }
            res = 0;
            goto cleanup;
        }
    }
    else {
        PyErr_Clear();
    }

    /* 'other' is iterator */
    if (NULL != (pairs_it = PyObject_GetIter(initializer))) {
        while (NULL != (item = PyIter_Next(pairs_it))) {
            /* Check pair */
            if (NULL == (pair = PySequence_Tuple(item))) {
                goto error;
            }
            if (PySequence_Size(pair) != 2) {
                goto error;
            }
            if (Int2Int_setitem(self, PyTuple_GET_ITEM(pair, 0),
                    PyTuple_GET_ITEM(pair, 1))) {
                goto cleanup;
            }
            Py_CLEAR(item);
            Py_CLEAR(pair);
        }
        if (PyErr_Occurred()) {
            goto cleanup;
        }
        res = 0;
        goto cleanup;
    }

error:
    PyErr_SetString(PyExc_TypeError,
            "'initializer' must be mapping or iterator "
            "over pairs (key, value)");

cleanup:
    Py_XDECREF(pairs);
    Py_XDECREF(pairs_it);
    Py_XDECREF(pair);
    Py_XDECREF(item);

    return res;
}

static PyObject* Int2Int_update(Int2Int_t *self, PyObject *args) {
    PyObject * initializer = NULL;

    if (!PyArg_ParseTuple(args, "|O", &initializer)) {
        return NULL;
    }
    if (Int2Int_update_from_initializer(self, initializer) != 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* Int2Int_setdefault(Int2Int_t *self, PyObject *args) {
    PyObject * key;
    PyObject * default_value = NULL;
    unsigned long long c_key;
    size_t c_value;

    if (!PyArg_ParseTuple(args, "O|O", &key, &default_value)) {
        return NULL;
    }
    /* key argument */
    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2int_get(self->hashmap, c_key, &c_value) == -1) {
        if (Int2Int_setitem(self, key, default_value) == -1) {
            return NULL;
        }
        Py_INCREF(default_value);
        return default_value;
    }

    return PyLong_FromSize_t(c_value);
}

static PyObject* Int2Int_reduce(Int2Int_t *self) {
    PyObject *res = NULL;
    PyObject *args = NULL;
    PyObject *callable = NULL;
    PyObject *readonly;
    PyObject *data;

    if (NULL == (res = PyTuple_New(2))) {
        goto error;
    }
    if (NULL == (args = PyTuple_New(6))) {
        goto error;
    }
    if (NULL == (callable = PyObject_GetAttrString(
            (PyObject *) self, "_from_raw_data"))) {
        goto error;
    }

    readonly = self->hashmap->readonly ? Py_True : Py_False;
    if (NULL == (data = PyBytes_FromStringAndSize(
            (const char *) self->table,
            self->hashmap->table_size * sizeof(Int2IntItem_t)))) {
        goto error;
    }

    Py_INCREF(self->default_value);
    Py_INCREF(readonly);

    PyTuple_SET_ITEM(args, 0, self->default_value);
    PyTuple_SET_ITEM(args, 1, PyLong_FromSize_t(self->hashmap->size));
    PyTuple_SET_ITEM(args, 2, PyLong_FromSize_t(self->hashmap->current_size));
    PyTuple_SET_ITEM(args, 3, PyLong_FromSize_t(self->hashmap->table_size));
    PyTuple_SET_ITEM(args, 4, readonly);
    PyTuple_SET_ITEM(args, 5, data);

    PyTuple_SET_ITEM(res, 0, callable);
    PyTuple_SET_ITEM(res, 1, args);

    return res;

error:
    Py_XDECREF(res);
    Py_XDECREF(args);
    Py_XDECREF(callable);

    return NULL;
}

static PyObject* Int2Int_from_raw_data(PyTypeObject *cls, PyObject *args) {
    PyObject *default_value = Py_None;
    size_t size;
    size_t current_size;
    size_t table_size;
    int readonly;
    Py_buffer buffer = { .obj = NULL };
    size_t int2int_memory_size;
    Int2Int_t *self = NULL;
    PyObject *res = NULL;

    /* Parse arguments */
    if (!PyArg_ParseTuple(args, "Onnnpy*", &default_value, &size,
            &current_size, &table_size, &readonly, &buffer)) {
        goto error;
    }
    /* Validate arguments */
    if ((default_value != Py_None)
            && (!PyLong_Check(default_value) ||
                    ((PyLong_AsSize_t(default_value) == (size_t) -1)
                            && PyErr_Occurred()))) {
        PyErr_SetString(PyExc_TypeError, "'default' must be positive int");
        goto error;
    }
    if ((size < current_size) || (table_size < size)
            || ((size_t) buffer.len != (table_size * sizeof(Int2IntItem_t)))) {
        PyErr_SetString(PyExc_ValueError, "Inconsistent argument's values");
        goto error;
    }

    /* Create instance */
    if (NULL == (self = (Int2Int_t*) cls->tp_alloc(cls, 0))) {
        goto error;
    }

    /* Allocate memory for Int2IntHashTable_t structure and hashtable. At
       the beginning of block of the memory Int2IntHashTable_t structure
       is placed, followed by hashtable (array of Int2IntItem_t). */
    int2int_memory_size = INT2INT_MEMORY_SIZE(table_size);
    if (NULL == (self->hashmap = malloc(int2int_memory_size))) {
        PyErr_NoMemory();
        goto error;
    }
    memset(self->hashmap, 0, int2int_memory_size);

    self->release_memory = true;
    self->default_value = default_value;
    self->hashmap->size = size;
    self->hashmap->current_size = current_size;
    self->hashmap->table_size = table_size;
    self->hashmap->readonly = readonly;
    self->table = (void*) self->hashmap + sizeof(Int2IntHashTable_t);
    memcpy((void *) self->table, buffer.buf, buffer.len);

    Py_INCREF(self->default_value);
    res = (PyObject*) self;
    goto cleanup;

error:
    if (NULL != self) {
        cls->tp_free((PyObject*) self);
    }
cleanup:
    if (NULL != buffer.obj) {
        PyBuffer_Release(&buffer);
    }

    return res;
}

static PyObject* Int2Int_from_ptr(PyTypeObject *cls, PyObject *args) {
    const Py_ssize_t addr;
    Int2IntHashTable_t *other;
    Int2Int_t *self;

    /* Parse addr */
    if (!PyArg_ParseTuple(args, "n", &addr)) {
        return NULL;
    }

    other = (Int2IntHashTable_t*) addr;
    /* Check readonly flag */
    if (!other->readonly) {
        PyErr_SetString(PyExc_RuntimeError, "Instance must be read-only");
        return NULL;
    }

    /* Create instance */
    self = (Int2Int_t*) cls->tp_alloc(cls, 0);
    if (!self) {
        return NULL;
    }
    self->default_value = Py_None;
    self->release_memory = false;
    self->hashmap = other;
    self->table = (void*) self->hashmap + sizeof(Int2IntHashTable_t);

    Py_INCREF(self->default_value);
    return (PyObject*) self;
}

static PyObject* Int2Int_make_readonly(Int2Int_t *self) {
    self->hashmap->readonly = true;
    Py_RETURN_NONE;
}

static PyObject* Int2Int_get_readonly(Int2Int_t *self) {
    if (self->hashmap->readonly) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

static PyObject* Int2Int_get_buffer_ptr(Int2Int_t *self) {
    return PyLong_FromVoidPtr(self->hashmap);
}

static PyObject* Int2Int_get_buffer_size(Int2Int_t *self) {
    return PyLong_FromSize_t(INT2INT_MEMORY_SIZE(self->hashmap->table_size));
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
    {"get", (PyCFunction) Int2Int_get, METH_VARARGS,
            "get(self, key, default=None, /)\n"
            "--\n"
            "\n"
            "Return value for key. If key does not exist, return default\n"
            "value, otherwise return None. default must be int or None."},
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
    {"pop", (PyCFunction) Int2Int_pop, METH_VARARGS,
            "pop(self, key, default, /)\n"
            "--\n"
            "\n"
            "Return value for key and remove this value from structure.\n"
            "If key does not exist, return default value, otherwise raise\n"
            "KeyError exception. default must be int or None."},
    {"popitem", (PyCFunction) Int2Int_popitem, METH_NOARGS,
            "popitem(self, /)\n"
            "--\n"
            "\n"
            "Return arbitrary (key, value) pair from structure and remove\n"
            "this item."},
    {"clear", (PyCFunction) Int2Int_clear, METH_NOARGS,
            "clear(self, /)\n"
            "--\n"
            "\n"
            "Remove all items from structure."},
    {"update", (PyCFunction) Int2Int_update, METH_VARARGS,
            "update(self, initializer, /)\n"
            "--\n"
            "\n"
            "Update mapping with keys and values from initializer,\n"
            "overwrite existing values. initializer can be either\n"
            "iterable with (key, value) pairs or mapping."},
    {"setdefault", (PyCFunction) Int2Int_setdefault, METH_VARARGS,
            "setdefault(self, key, default, /)\n"
            "--\n"
            "\n"
            "Return value for key. If key does not exist, insert new key\n"
            "with value default and return this value. If default is not\n"
            "specified, raise KeyError exception. default must be int.\n"},
    {"from_ptr", (PyCFunction) Int2Int_from_ptr, METH_VARARGS | METH_CLASS,
            "from_ptr(self, addr, /)\n"
            "--\n"
            "\n"
            "Return instance created from address pointed to existing\n"
            "Int2Int memory block."},
    {"make_readonly", (PyCFunction) Int2Int_make_readonly, METH_NOARGS,
            "make_readonly(self, /)\n"
            "--\n"
            "\n"
            "Make Int2Int structure as a read-only."},
    {"__reduce__", (PyCFunction) Int2Int_reduce, METH_NOARGS,
            "__reduce__(self, /)\n"
            "--\n"
            "\n"
            "Return reduced value of the instance."},
    {"_from_raw_data", (PyCFunction) Int2Int_from_raw_data,
            METH_VARARGS | METH_CLASS,
            "_from_raw_data(self, ..., /)\n"
            "--\n"
            "\n"
            "Return instance created from raw data.\n"
            "\n"
            "It is protected method used when object is unpickled, do not \n"
            "call this method yourself!"},
    {NULL}
};

static PyGetSetDef Int2Int_getset[] = {
    {"readonly", (getter) Int2Int_get_readonly, NULL,
            "Flag that indicates that instance is read-only.", NULL},
    {"buffer_ptr", (getter) Int2Int_get_buffer_ptr, NULL,
            "Address of the internal buffer.", NULL},
    {"buffer_size", (getter) Int2Int_get_buffer_size, NULL,
            "Size of the internal buffer in bytes.", NULL},
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
    "Int2Int(self, initializer, default=None, /)\n"     /* tp_doc */
    "--\n"
    "\n"
    "Simple fixed-size hashmap which maps int key to int value. Provides\n"
    "pointer to internal C structure and set/del/get/has functions, so\n"
    "hashmap is accesible from pure C. Easily fill data in Python and\n"
    "compute in C or Cython.\n"
    "\n"
    "If initializer is specified, instance will be filled from this\n"
    "initializer. It can be either iterable with (key, value) pairs or\n"
    "mapping. If default is specified, value of the default will be\n"
    "returned when key does not exist and will be stored into mapping.",
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
    Int2FloatItem_t *table;
    PyObject *default_value;
    bool release_memory;
} Int2Float_t;

static PyTypeObject Int2Float_type;

/* Int2Float iterator */

static PyObject* Int2FloatIterator_next(HashmapIterator_t *self) {
    Int2Float_t *obj = (Int2Float_t*) self->obj;
    PyObject *res = NULL;

    while (self->current_position < obj->hashmap->table_size) {
        Int2FloatItem_t item = obj->table[self->current_position];
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

static int Int2Float_update_from_initializer(Int2Float_t *self,
        PyObject *initializer);

static PyObject* Int2Float_new(PyTypeObject *cls,
        PyObject *args, PyObject *kwds) {

    char *kwnames[] = {"initializer", "default", "prealloc_size", NULL};
    PyObject *initializer = NULL;
    PyObject *default_value = NULL;
    unsigned int prealloc_size = INT2FLOAT_INITIAL_SIZE;
    Int2Float_t *self = NULL;

    /* Parse arguments */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O$OI", kwnames,
            &initializer, &default_value, &prealloc_size)) {
        goto error;
    }
    /* Validate arguments */
    if ((NULL != default_value) && (Py_None != default_value)) {
        if (PyLong_Check(default_value)) {
            if (NULL == (default_value = PyNumber_Float(default_value))) {
                goto error;
            }
        } else {
            if (!PyFloat_Check(default_value)) {
                PyErr_SetString(PyExc_TypeError, "'default' must be a float");
                goto error;
            }
            Py_INCREF(default_value);
        }
    } else {
        if (NULL == default_value) {
            default_value = Py_None;
        }
        Py_INCREF(default_value);
    }

    /* Create instance */
    if (NULL == (self = (Int2Float_t*) cls->tp_alloc(cls, 0))) {
        goto error;
    }

    /* Allocate memory for Int2FloatHashTable_t structure and hashtable. At
       the beginning of block of the memory Int2FloatHashTable_t structure
       is placed, followed by hashtable (array of Int2FloatItem_t). */
    if (int2float_new(prealloc_size, &self->hashmap)) {
        PyErr_NoMemory();
        goto error;
    }

    /* Initialize object attributes */
    self->release_memory = true;
    self->default_value = default_value;
    self->table = (void*) self->hashmap + sizeof(Int2FloatHashTable_t);

    if ((NULL != initializer) &&
            (Int2Float_update_from_initializer(self, initializer) != 0)) {
        goto error;
    }

    return (PyObject*) self;

error:
    Py_XDECREF(default_value);
    if (NULL != self) {
        cls->tp_free((PyObject*) self);
    }

    return NULL;
}

static void Int2Float_dealloc(Int2Float_t *self) {
    Py_DECREF(self->default_value);
    if (self->release_memory && (NULL != self->hashmap)) {
        free(self->hashmap);
    }
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* Int2Float_repr(Int2Float_t *self) {
    if (self->hashmap->readonly) {
        return PyUnicode_FromFormat("<%s: object at %p, used %zd, read-only>",
                self->ob_base.ob_type->tp_name, self,
                self->hashmap->current_size);
    } else {
        if (self->default_value == Py_None) {
            return PyUnicode_FromFormat(
                    "<%s: object at %p, used %zd>",
                    self->ob_base.ob_type->tp_name, self,
                    self->hashmap->current_size);
        } else {
            return PyUnicode_FromFormat(
                    "<%s: object at %p, used %zd, default %A>",
                    self->ob_base.ob_type->tp_name, self,
                    self->hashmap->current_size, self->default_value);
        }
    }
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
        break;
    case Py_LE:
        PyErr_SetString(PyExc_TypeError, "'<=' is not supported");
        res = NULL;
        break;
    case Py_GT:
        PyErr_SetString(PyExc_TypeError, "'>' is not supported");
        res = NULL;
        break;
    case Py_GE:
        PyErr_SetString(PyExc_TypeError, "'>=' is not supported");
        res = NULL;
        break;
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
                    Int2FloatItem_t item = self->table[i];

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

static int Int2Float_contains(Int2Float_t *self, PyObject *key) {
    unsigned long long c_key;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    return int2float_has(self->hashmap, c_key) == -1 ? 0 : 1;
}

static int Int2Float_setitem(Int2Float_t *self,
        PyObject *key, PyObject *value) {
    unsigned long long c_key;
    double c_value;
    Int2FloatHashTable_t * new_hashmap;

    if (self->hashmap->readonly) {
        PyErr_SetString(PyExc_RuntimeError, "Instance is read-only");
        return -1;
    }
    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return -1;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return -1;
    }

    if (value == NULL) {
        /* Delete item */
        if (int2float_del(self->hashmap, c_key) == -1) {
            Py_INCREF(key);
            PyErr_SetObject(PyExc_KeyError, key);
            return -1;
        }
    }
    else {
        /* Set new or update existing item */
        if (PyLong_Check(value)) {
            c_value = PyLong_AsDouble(value);
            if ((c_value == -1.0) && (PyErr_Occurred() != NULL)) {
                return -1;
            }
        } else {
            if (!PyFloat_Check(value)) {
                PyErr_SetString(PyExc_TypeError, "'value' must be a float");
                return -1;
            }
            c_value = PyFloat_AsDouble(value);
            if ((c_value == -1.0) && (PyErr_Occurred() != NULL)) {
                return -1;
            }
        }
        if (int2float_set(self->hashmap, c_key, c_value, &new_hashmap)) {
            PyErr_NoMemory();
            return -1;
        }
        if (new_hashmap != self->hashmap) {
            self->hashmap = new_hashmap;
            self->table = (void*) new_hashmap + sizeof(Int2FloatHashTable_t);
        }
    }

    return 0;
}

static PyObject* Int2Float_getitem(Int2Float_t *self, PyObject *key) {
    unsigned long long c_key;
    double c_value;
    Int2FloatHashTable_t * new_hashmap;

    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (NULL != PyErr_Occurred())) {
        return NULL;
    }

    if (int2float_get(self->hashmap, c_key, &c_value) == -1) {
        if (self->default_value != Py_None) {
            c_value = PyFloat_AsDouble(self->default_value);
            if ((c_value == -1.0) && (NULL != PyErr_Occurred())) {
                return NULL;
            }
            if (int2float_set(self->hashmap, c_key, c_value, &new_hashmap)) {
                PyErr_NoMemory();
                return NULL;
            }
            if (new_hashmap != self->hashmap) {
                self->hashmap = new_hashmap;
                self->table = (void*) new_hashmap
                        + sizeof(Int2FloatHashTable_t);
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

static PyObject* Int2Float_get(Int2Float_t *self,
        PyObject *args, PyObject *kwds) {
    PyObject * key;
    PyObject * default_value = Py_None;
    unsigned long long c_key;
    double value;

    if (!PyArg_ParseTuple(args, "O|O", &key, &default_value)) {
        return NULL;
    }

    /* key argument */
    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2float_get(self->hashmap, c_key, &value) == -1) {
        if (default_value != Py_None) {
            if (PyLong_Check(default_value)) {
                if (NULL == (default_value = PyNumber_Float(default_value))) {
                    return NULL;
                }
            } else {
                if (!PyFloat_Check(default_value)) {
                    PyErr_SetString(PyExc_TypeError,
                            "'default' must be a float");
                    return NULL;
                }
                Py_INCREF(default_value);
            }
        } else {
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

static PyObject* Int2Float_pop(Int2Float_t *self, PyObject *args) {
    PyObject * key;
    PyObject * default_value = NULL;
    unsigned long long c_key;
    double value;

    if (!PyArg_ParseTuple(args, "O|O", &key, &default_value)) {
        return NULL;
    }
    /* key argument */
    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2float_get(self->hashmap, c_key, &value) == -1) {
        if (NULL == default_value) {
            PyErr_SetObject(PyExc_KeyError, key);
            return NULL;
        }
        if (default_value == Py_None) {
            Py_INCREF(default_value);
        }
        else {
            if (PyLong_Check(default_value)) {
                if (NULL == (default_value = PyNumber_Float(default_value))) {
                    return NULL;
                }
            } else {
                if (!PyFloat_Check(default_value)) {
                    PyErr_SetString(PyExc_TypeError,
                            "'default' must be float or None");
                    return NULL;
                }
                Py_INCREF(default_value);
            }
        }
        return default_value;
    }

    int2float_del(self->hashmap, c_key);
    return PyFloat_FromDouble(value);
}

static PyObject* Int2Float_popitem(Int2Float_t *self) {
    Int2FloatItem_t * item;
    PyObject * res = NULL;
    PyObject * key = NULL;
    PyObject * value = NULL;

    for (size_t i=0; i<self->hashmap->table_size; ++i) {
        item = &(self->table[i]);
        if (USED == item->status) {
            if (NULL == (key = PyLong_FromUnsignedLongLong(item->key))) {
                goto error;
            }
            if (NULL == (value = PyFloat_FromDouble(item->value))) {
                goto error;
            }
            if (NULL == (res = PyTuple_New(2))) {
                goto error;
            }
            PyTuple_SET_ITEM(res, 0, key);
            PyTuple_SET_ITEM(res, 1, value);

            item->status = DELETED;
            self->hashmap->current_size -= 1;

            return res;
        }
    }
    PyErr_SetString(PyExc_KeyError, "popitem(): mapping is empty");

error:
    Py_XDECREF(res);
    Py_XDECREF(key);
    Py_XDECREF(value);

    return NULL;
}

static PyObject* Int2Float_clear(Int2Float_t *self) {
    for (size_t i=0; i<self->hashmap->table_size; ++i) {
        self->table[i].status = EMPTY;
    }
    self->hashmap->current_size = 0;

    Py_RETURN_NONE;
}

int Int2Float_update_from_initializer(Int2Float_t *self,
        PyObject *initializer) {
    PyObject * pairs = NULL;
    PyObject * pairs_it = NULL;
    PyObject * pair = NULL;
    PyObject * item = NULL;
    int res = -1;

    /* No 'other' argument */
    if (NULL == initializer) {
        res = 0;
        goto cleanup;
    }

    /* 'other' is mapping */
    if (NULL != (pairs = PyMapping_Items(initializer))) {
        if (NULL != (pairs_it = PyObject_GetIter(pairs))) {
            while (NULL != (pair = PyIter_Next(pairs_it))) {
                if (Int2Float_setitem(self, PyTuple_GET_ITEM(pair, 0),
                        PyTuple_GET_ITEM(pair, 1))) {
                    goto cleanup;
                }
                Py_CLEAR(pair);
            }
            if (PyErr_Occurred()) {
                goto cleanup;
            }
            res = 0;
            goto cleanup;
        }
    }
    else {
        PyErr_Clear();
    }

    /* 'other' is iterator */
    if (NULL != (pairs_it = PyObject_GetIter(initializer))) {
        while (NULL != (item = PyIter_Next(pairs_it))) {
            /* Check pair */
            if (NULL == (pair = PySequence_Tuple(item))) {
                goto error;
            }
            if (PySequence_Size(pair) != 2) {
                goto error;
            }
            if (Int2Float_setitem(self, PyTuple_GET_ITEM(pair, 0),
                    PyTuple_GET_ITEM(pair, 1))) {
                goto cleanup;
            }
            Py_CLEAR(item);
            Py_CLEAR(pair);
        }
        if (PyErr_Occurred()) {
            goto cleanup;
        }
        res = 0;
        goto cleanup;
    }

error:
    PyErr_SetString(PyExc_TypeError,
            "'initializer' must be mapping or iterator "
            "over pairs (key, value)");

cleanup:
    Py_XDECREF(pairs);
    Py_XDECREF(pairs_it);
    Py_XDECREF(pair);
    Py_XDECREF(item);

    return res;
}

static PyObject* Int2Float_update(Int2Float_t *self, PyObject *args) {
    PyObject * initializer = NULL;

    if (!PyArg_ParseTuple(args, "|O", &initializer)) {
        return NULL;
    }
    if (Int2Float_update_from_initializer(self, initializer) != 0) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* Int2Float_setdefault(Int2Float_t *self, PyObject *args) {
    PyObject * key;
    PyObject * default_value = NULL;
    unsigned long long c_key;
    double c_value;

    if (!PyArg_ParseTuple(args, "O|O", &key, &default_value)) {
        return NULL;
    }
    /* key argument */
    if (!PyLong_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "'key' must be an integer");
        return NULL;
    }
    c_key = PyLong_AsUnsignedLongLong(key);
    if ((c_key == (unsigned long long) -1) && (PyErr_Occurred() != NULL)) {
        return NULL;
    }

    if (int2float_get(self->hashmap, c_key, &c_value) == -1) {
        if (Int2Float_setitem(self, key, default_value) == -1) {
            return NULL;
        }
        Py_INCREF(default_value);
        return default_value;
    }

    return PyFloat_FromDouble(c_value);
}

static PyObject* Int2Float_reduce(Int2Float_t *self) {
    PyObject *res = NULL;
    PyObject *args = NULL;
    PyObject *callable = NULL;
    PyObject *readonly;
    PyObject *data;

    if (NULL == (res = PyTuple_New(2))) {
        goto error;
    }
    if (NULL == (args = PyTuple_New(6))) {
        goto error;
    }
    if (NULL == (callable = PyObject_GetAttrString(
            (PyObject *) self, "_from_raw_data"))) {
        goto error;
    }

    readonly = self->hashmap->readonly ? Py_True : Py_False;
    if (NULL == (data = PyBytes_FromStringAndSize(
            (const char *) self->table,
            self->hashmap->table_size * sizeof(Int2FloatItem_t)))) {
        goto error;
    }

    Py_INCREF(self->default_value);
    Py_INCREF(readonly);

    PyTuple_SET_ITEM(args, 0, self->default_value);
    PyTuple_SET_ITEM(args, 1, PyLong_FromSize_t(self->hashmap->size));
    PyTuple_SET_ITEM(args, 2, PyLong_FromSize_t(self->hashmap->current_size));
    PyTuple_SET_ITEM(args, 3, PyLong_FromSize_t(self->hashmap->table_size));
    PyTuple_SET_ITEM(args, 4, readonly);
    PyTuple_SET_ITEM(args, 5, data);

    PyTuple_SET_ITEM(res, 0, callable);
    PyTuple_SET_ITEM(res, 1, args);

    return res;

error:
    Py_XDECREF(res);
    Py_XDECREF(args);
    Py_XDECREF(callable);

    return NULL;
}

static PyObject* Int2Float_from_raw_data(PyTypeObject *cls, PyObject *args) {
    PyObject *default_value = Py_None;
    size_t size;
    size_t current_size;
    size_t table_size;
    int readonly;
    Py_buffer buffer = { .obj = NULL };
    size_t int2float_memory_size;
    Int2Float_t *self = NULL;
    PyObject *res = NULL;

    /* Parse arguments */
    if (!PyArg_ParseTuple(args, "Onnnpy*", &default_value, &size,
            &current_size, &table_size, &readonly, &buffer)) {
        goto error;
    }
    /* Validate arguments */
    if ((default_value != Py_None)
            && (!PyFloat_Check(default_value) ||
                    ((PyFloat_AsDouble(default_value) == -1.0)
                            && PyErr_Occurred()))) {
        PyErr_SetString(PyExc_TypeError, "'default' must be float");
        goto error;
    }
    if ((size < current_size) || (table_size < size)
            || ((size_t) buffer.len !=
                    (table_size * sizeof(Int2FloatItem_t)))) {
        PyErr_SetString(PyExc_ValueError, "Inconsistent argument's values");
        goto error;
    }

    /* Create instance */
    if (NULL == (self = (Int2Float_t*) cls->tp_alloc(cls, 0))) {
        goto error;
    }

    /* Allocate memory for Int2FloatHashTable_t structure and hashtable. At
       the beginning of block of the memory Int2FloatHashTable_t structure
       is placed, followed by hashtable (array of Int2FloatItem_t). */
    int2float_memory_size = INT2FLOAT_MEMORY_SIZE(table_size);
    if (NULL == (self->hashmap = malloc(int2float_memory_size))) {
        PyErr_NoMemory();
        goto error;
    }
    memset(self->hashmap, 0, int2float_memory_size);

    self->release_memory = true;
    self->default_value = default_value;
    self->hashmap->size = size;
    self->hashmap->current_size = current_size;
    self->hashmap->table_size = table_size;
    self->hashmap->readonly = readonly;
    self->table = (void*) self->hashmap + sizeof(Int2FloatHashTable_t);
    memcpy((void *) self->table, buffer.buf, buffer.len);

    Py_INCREF(self->default_value);
    res = (PyObject*) self;
    goto cleanup;

error:
    if (NULL != self) {
        cls->tp_free((PyObject*) self);
    }
cleanup:
    if (NULL != buffer.obj) {
        PyBuffer_Release(&buffer);
    }

    return res;
}

static PyObject* Int2Float_from_ptr(PyTypeObject *cls, PyObject *args) {
    const Py_ssize_t addr;
    Int2FloatHashTable_t *other;
    Int2Float_t *self;

    /* Parse addr */
    if (!PyArg_ParseTuple(args, "n", &addr)) {
        return NULL;
    }

    other = (Int2FloatHashTable_t*) addr;
    /* Check readonly flag */
    if (!other->readonly) {
        PyErr_SetString(PyExc_RuntimeError, "Instance must be read-only");
        return NULL;
    }

    /* Create instance */
    self = (Int2Float_t*) cls->tp_alloc(cls, 0);
    if (!self) {
        return NULL;
    }
    self->default_value = Py_None;
    self->release_memory = false;
    self->hashmap = other;
    self->table = (void*) self->hashmap + sizeof(Int2FloatHashTable_t);

    Py_INCREF(self->default_value);
    return (PyObject*) self;
}

static PyObject* Int2Float_make_readonly(Int2Float_t *self) {
    self->hashmap->readonly = true;
    Py_RETURN_NONE;
}

static PyObject* Int2Float_get_readonly(Int2Float_t *self) {
    if (self->hashmap->readonly) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

static PyObject* Int2Float_get_buffer_ptr(Int2Float_t *self) {
    return PyLong_FromVoidPtr(self->hashmap);
}

static PyObject* Int2Float_get_buffer_size(Int2Float_t *self) {
    return PyLong_FromSize_t(INT2FLOAT_MEMORY_SIZE(self->hashmap->table_size));
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
    {"get", (PyCFunction) Int2Float_get, METH_VARARGS,
            "get(self, key, default=None, /)\n"
            "--\n"
            "\n"
            "Return value for key. If key does not exist, return default\n"
            "value, otherwise return None. default must be float or None."},
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
    {"pop", (PyCFunction) Int2Float_pop, METH_VARARGS,
            "pop(self, key, default, /)\n"
             "--\n"
             "\n"
            "Return value for key and remove this value from structure.\n"
            "If key does not exist, return default value, otherwise raise\n"
            "KeyError exception. default must be float or None."},
    {"popitem", (PyCFunction) Int2Float_popitem, METH_NOARGS,
            "popitem(self, /)\n"
            "--\n"
            "\n"
            "Return arbitrary (key, value) pair from structure and remove\n"
            "this item."},
    {"clear", (PyCFunction) Int2Float_clear, METH_NOARGS,
            "clear(self, /)\n"
            "--\n"
            "\n"
            "Remove all items from structure."},
    {"update", (PyCFunction) Int2Float_update, METH_VARARGS,
            "update(self, initializer, /)\n"
            "--\n"
            "\n"
            "Update mapping with keys and values from initializer,\n"
            "overwrite existing values. initializer can be either\n"
            "iterable with (key, value) pairs or mapping."},
    {"setdefault", (PyCFunction) Int2Float_setdefault, METH_VARARGS,
            "setdefault(self, key, default, /)\n"
            "--\n"
            "\n"
            "Return value for key. If key does not exist, insert new key\n"
            "with value default and return this value. If default is not\n"
            "specified, raise KeyError exception. default must be float.\n"},
    {"from_ptr", (PyCFunction) Int2Float_from_ptr, METH_VARARGS | METH_CLASS,
            "from_ptr(self, addr, /)\n"
            "--\n"
            "\n"
            "Return instance created from address pointed to existing\n"
            "Int2Float memory block."},
    {"make_readonly", (PyCFunction) Int2Float_make_readonly, METH_NOARGS,
            "make_readonly(self, /)\n"
            "--\n"
            "\n"
            "Make Int2Float structure as a read-only."},
    {"__reduce__", (PyCFunction) Int2Float_reduce, METH_NOARGS,
            "__setstate__(self, /)\n"
            "--\n"
            "\n"
            "Return reduced value of the instance."},
    {"_from_raw_data", (PyCFunction) Int2Float_from_raw_data,
            METH_VARARGS | METH_CLASS,
            "_from_raw_data(self, ..., /)\n"
            "--\n"
            "\n"
            "Return instance created from raw data.\n"
            "\n"
            "It is protected method used when object is unpickled, do not \n"
            "call this method yourself!"},
    {NULL}
};

static PyGetSetDef Int2Float_getset[] = {
    {"readonly", (getter) Int2Float_get_readonly, NULL,
            "Flag that indicates that instance is read-only.", NULL},
    {"buffer_ptr", (getter) Int2Float_get_buffer_ptr, NULL,
            "Address of the internal buffer.", NULL},
    {"buffer_size", (getter) Int2Float_get_buffer_size, NULL,
            "Size of the internal buffer in bytes.", NULL},
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
    "Int2Float(self, initializer, default=None, /)\n"   /* tp_doc */
    "--\n"
    "\n"
    "Simple fixed-size hashmap which maps int key to float value. Provides\n"
    "pointer to internal C structure and set/del/get/has functions, so\n"
    "hashmap is accesible from pure C. Easily fill data in Python and\n"
    "compute in C or Cython.\n"
    "\n"
    "If initializer is specified, instance will be filled from this\n"
    "initializer. It can be either iterable with (key, value) pairs or\n"
    "mapping. If default is specified, value of the default will be\n"
    "returned when key does not exist and will be stored into mapping.",
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
    PyObject *abc_module = NULL;
    PyObject *mutable_mapping = NULL;
    PyObject *all = NULL;
    PyObject *module = NULL;

    /* Obtain MutableMapping from collections.abc */
    if (NULL == (abc_module = PyImport_ImportModule("collections.abc"))) {
        goto error;
    }
    if (NULL == (mutable_mapping = PyObject_GetAttrString(
            abc_module, "MutableMapping"))) {
        goto error;
    }

    /* Initialize types */
    if (PyType_Ready(&Int2IntIterator_type)
            || PyType_Ready(&Int2Int_type)
            || PyType_Ready(&Int2FloatIterator_type)
            || PyType_Ready(&Int2Float_type)) {
        goto error;
    }

    /* Create module object */
    if (NULL == (module = PyModule_Create(&hashmapmodule))) {
        goto error;
    }
    /* Create __all__ attribute */
    if (NULL == (all = PyList_New(2))) {
        goto error;
    }
    PyList_SET_ITEM(all, 0, PyUnicode_FromString("Int2Int"));
    PyList_SET_ITEM(all, 1, PyUnicode_FromString("Int2Float"));
    /* Add objects onto module */
    Py_INCREF(&Int2Int_type);
    if (PyModule_AddObject(module, "Int2Int", (PyObject*) &Int2Int_type)) {
        Py_DECREF(&Int2Int_type);
        goto error;
    }
    Py_INCREF(&Int2Float_type);
    if (PyModule_AddObject(module, "Int2Float", (PyObject*) &Int2Float_type)) {
        Py_DECREF(&Int2Float_type);
        goto error;
    }
    if (PyModule_AddObject(module, "__all__", all)) {
        goto error;
    }

    /* Register types into collections.abs */
    if (NULL == PyObject_CallMethod(
            mutable_mapping, "register", "O", (PyObject*) &Int2Int_type)) {
        goto error;
    }
    if (NULL == PyObject_CallMethod(
            mutable_mapping, "register", "O", (PyObject*) &Int2Float_type)) {
        goto error;
    }

    return module;

error:
    Py_XDECREF(abc_module);
    Py_XDECREF(mutable_mapping);
    Py_XDECREF(module);
    Py_XDECREF(all);

    return NULL;
}
