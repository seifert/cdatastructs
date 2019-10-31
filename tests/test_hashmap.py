
import ctypes
import operator
import pickle
import re

import pytest

from cdatastructs.hashmap import Int2Int, Int2Float


# Int2Int ---------------------------------------------------------------------

@pytest.fixture(scope='function')
def int2int_map():
    return Int2Int()


def test_int2int_new_fail_when_invalid_default_kwarg_type():
    with pytest.raises(TypeError, match="'default' must be positive int"):
        Int2Int(default='8')


def test_int2int_new_fail_when_negative_default_kwarg():
    with pytest.raises(TypeError, match="'default' must be positive int"):
        Int2Int(default=-1)


def test_int2int_len_when_empty(int2int_map):
    assert len(int2int_map) == 0


def test_int2int_len_when_not_empty(int2int_map):
    int2int_map[1] = 100
    assert len(int2int_map) == 1


def test_int2int_bool_when_empty(int2int_map):
    assert bool(int2int_map) is False


def test_int2int_bool_when_not_empty(int2int_map):
    int2int_map[1] = 100
    assert bool(int2int_map) is True


def test_int2int_repr_when_empty(int2int_map):
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, used 0\>',
        repr(int2int_map))
    assert m is not None


def test_int2int_repr_when_not_empty(int2int_map):
    int2int_map[1] = 100
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, used 1\>',
        repr(int2int_map))
    assert m is not None


def test_int2int_repr_when_empty_and_default_kwarg():
    int2int_map = Int2Int(default=100)
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, '
        r'used 0, default 100\>',
        repr(int2int_map))
    assert m is not None


def test_int2int_repr_when_not_empty_and_default_kwarg():
    int2int_map = Int2Int(default=100)
    int2int_map[1] = 100
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, '
        r'used 1, default 100\>',
        repr(int2int_map))
    assert m is not None


def test_int2int_repr_when_readonly(int2int_map):
    int2int_map[1] = 100
    int2int_map.make_readonly()
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, '
        r'used 1, read-only\>',
        repr(int2int_map))
    print(repr(int2int_map))
    assert m is not None


def test_int2int_setitem_fail_when_invalid_key_type(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map['1'] = 100
    assert "'key' must be an integer" in str(exc_info)


def test_int2int_setitem_fail_when_negative_key(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map[-1] = 100
    assert "can't convert negative int to unsigned" in str(exc_info)


def test_int2int_setitem_fail_when_key_is_too_big(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map[2 ** 64] = 100
    assert "int too big to convert" in str(exc_info)


def test_int2int_setitem_fail_when_invalid_value_type(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map[1] = '100'
    assert "'value' must be an integer" in str(exc_info)


def test_int2int_setitem_fail_when_negative_value(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map[1] = -1
    assert "can't convert negative value to size_t" in str(exc_info)


def test_int2int_setitem_fail_when_value_is_too_big(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map[1] = 2 ** 64
    assert "Python int too large to convert to C size_t" in str(exc_info)


def test_int2int_setitem_fail_when_readonly(int2int_map):
    int2int_map[1] = 100
    int2int_map.make_readonly()
    with pytest.raises(RuntimeError) as exc_info:
        int2int_map[2] = 200
    assert 'Instance is read-only' in str(exc_info)


@pytest.mark.parametrize('key', [0, (2 ** 64) - 1])
def test_int2int_setitem_getitem(int2int_map, key):
    int2int_map[key] = 100
    assert len(int2int_map) == 1
    assert isinstance(int2int_map[key], int)
    assert int2int_map[key] == 100


def test_int2int_setitem_twice(int2int_map):
    int2int_map[1] = 100
    int2int_map[1] = 200
    assert len(int2int_map) == 1
    assert isinstance(int2int_map[1], int)
    assert int2int_map[1] == 200


def test_int2int_setitem_more_items(int2int_map):

    class Int2IntItem_t(ctypes.Structure):

        _fields_ = [
            ('key', ctypes.c_ulonglong),
            ('value', ctypes.c_size_t),
            ('status', ctypes.c_int),
        ]

    class Int2IntHashTable_t(ctypes.Structure):

        _fields_ = [
            ('size', ctypes.c_size_t),
            ('current_size', ctypes.c_size_t),
            ('table_size', ctypes.c_size_t),
            ('readonly', ctypes.c_bool),
        ]

    t = Int2IntHashTable_t.from_address(int2int_map.buffer_ptr)

    assert int2int_map.buffer_size == (
        ctypes.sizeof(Int2IntHashTable_t)
        + (ctypes.sizeof(Int2IntItem_t) * t.table_size))
    assert t.size == 8
    assert t.current_size == 0

    for i in range(16):
        int2int_map[i] = i + 100

    t = Int2IntHashTable_t.from_address(int2int_map.buffer_ptr)

    assert int2int_map.buffer_size == (
        ctypes.sizeof(Int2IntHashTable_t)
        + (ctypes.sizeof(Int2IntItem_t) * t.table_size))
    assert len(int2int_map) == 16
    assert set(int2int_map.items()) == {
        (0, 100), (1, 101), (2, 102), (3, 103),
        (4, 104), (5, 105), (6, 106), (7, 107),
        (8, 108), (9, 109), (10, 110), (11, 111),
        (12, 112), (13, 113), (14, 114), (15, 115),
    }
    assert t.size == 16
    assert t.current_size == 16


def test_int2int_delitem(int2int_map):
    int2int_map[1] = 101
    int2int_map[2] = 102
    int2int_map[3] = 103
    int2int_map[4] = 104
    int2int_map[5] = 105
    int2int_map[6] = 106
    int2int_map[7] = 107
    int2int_map[8] = 108

    assert len(int2int_map) == 8
    del int2int_map[1]
    assert len(int2int_map) == 7
    del int2int_map[2]
    assert len(int2int_map) == 6
    del int2int_map[3]
    assert len(int2int_map) == 5
    del int2int_map[4]
    assert len(int2int_map) == 4
    del int2int_map[5]
    assert len(int2int_map) == 3
    del int2int_map[6]
    assert len(int2int_map) == 2
    del int2int_map[7]
    assert len(int2int_map) == 1
    del int2int_map[8]
    assert len(int2int_map) == 0


def test_int2int_delitem_fail_when_key_does_not_exist(int2int_map):
    int2int_map[2] = 102
    with pytest.raises(KeyError, match='1'):
        del int2int_map[1]


def test_int2int_getitem_more_keys_does_not_exist_and_default_kwarg():

    class Int2IntItem_t(ctypes.Structure):

        _fields_ = [
            ('key', ctypes.c_ulonglong),
            ('value', ctypes.c_size_t),
            ('status', ctypes.c_int),
        ]

    class Int2IntHashTable_t(ctypes.Structure):

        _fields_ = [
            ('size', ctypes.c_size_t),
            ('current_size', ctypes.c_size_t),
            ('table_size', ctypes.c_size_t),
            ('readonly', ctypes.c_bool),
        ]

    int2int_map = Int2Int(default=100)
    t = Int2IntHashTable_t.from_address(int2int_map.buffer_ptr)

    assert int2int_map.buffer_size == (
        ctypes.sizeof(Int2IntHashTable_t)
        + (ctypes.sizeof(Int2IntItem_t) * t.table_size))
    assert t.size == 8
    assert t.current_size == 0

    for i in range(16):
        int2int_map[i]

    t = Int2IntHashTable_t.from_address(int2int_map.buffer_ptr)

    assert int2int_map.buffer_size == (
        ctypes.sizeof(Int2IntHashTable_t)
        + (ctypes.sizeof(Int2IntItem_t) * t.table_size))
    assert len(int2int_map) == 16
    assert set(int2int_map.items()) == {
        (0, 100), (1, 100), (2, 100), (3, 100),
        (4, 100), (5, 100), (6, 100), (7, 100),
        (8, 100), (9, 100), (10, 100), (11, 100),
        (12, 100), (13, 100), (14, 100), (15, 100),
    }
    assert t.size == 16
    assert t.current_size == 16


def test_int2int_getitem_key_does_not_exist_and_default_kwarg():
    int2int_map = Int2Int(default=100)
    assert isinstance(int2int_map[1], int)
    assert int2int_map[1] == 100
    assert len(int2int_map) == 1


def test_int2int_getitem_key_does_not_exist_and_default_kwarg_is_none():
    int2int_map = Int2Int(default=None)
    with pytest.raises(KeyError) as exc_info:
        int2int_map[1]
    assert "'1'" in str(exc_info)


def test_int2int_getitem_fail_when_invalid_key_type(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map['1']
    assert "'key' must be an integer" in str(exc_info)


def test_int2int_getitem_fail_when_negative_key(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map[-1]
    assert "can't convert negative int to unsigned" in str(exc_info)


def test_int2int_getitem_fail_when_key_is_too_big(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map[2 ** 64]
    assert "int too big to convert" in str(exc_info)


def test_int2int_getitem_fail_when_key_does_not_exist(int2int_map):
    with pytest.raises(KeyError) as exc_info:
        int2int_map[1]
    assert "'1'" in str(exc_info)


def test_int2int_get(int2int_map):
    int2int_map[1] = 100
    value = int2int_map.get(1)
    assert value == 100
    assert isinstance(value, int)


def test_int2int_get_when_key_does_not_exist(int2int_map):
    assert int2int_map.get(1) is None


@pytest.mark.parametrize('default', [100, None])
def test_int2int_get_when_key_does_not_exist_and_default_arg(
        int2int_map, default):
    value = int2int_map.get(1, default)
    assert isinstance(value, type(default))
    assert (value == default) or (value is default)


def test_int2int_get_fail_when_invalid_default_type(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map.get(1, '1')
    exc_msg = str(exc_info)
    assert "'default' must be int or None" in exc_msg


def test_int2int_get_fail_when_invalid_key_type(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map.get('1')
    exc_msg = str(exc_info)
    assert "'key' must be an integer" in exc_msg


def test_int2int_get_fail_when_negative_key(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map.get(-1)
    assert "can't convert negative int to unsigned" in str(exc_info)


def test_int2int_get_fail_when_key_is_too_big(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map.get(2 ** 64)
    assert "int too big to convert" in str(exc_info)


def test_int2int_get_fail_when_invalid_default_type(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map.get(1, '1')
    exc_msg = str(exc_info)
    assert "'default' must be positive int" in exc_msg


def test_int2int_get_fail_when_negative_default(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map.get(1, -1)
    assert "can't convert negative value to size_t" in str(exc_info)


def test_int2int_get_fail_when_default_is_too_big(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        int2int_map.get(1, 2 ** 64)
    assert "Python int too large to convert to C size_t" in str(exc_info)


def test_int2int_contains_when_key_exists(int2int_map):
    int2int_map[2541265478] = 100
    assert 2541265478 in int2int_map


def test_int2int_contains_when_key_does_not_exist(int2int_map):
    assert 2541265478 not in int2int_map


def test_int2int_contains_fail_when_invalid_key_type(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        bool('1' in int2int_map)
    exc_msg = str(exc_info)
    assert "'key' must be an integer" in exc_msg


def test_int2int_contains_fail_when_negative_key(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        bool(-1 in int2int_map)
    assert "can't convert negative int to unsigned" in str(exc_info)


def test_int2int_contains_fail_when_key_is_too_big(int2int_map):
    with pytest.raises(OverflowError) as exc_info:
        bool((2 ** 64) in int2int_map)
    assert "int too big to convert" in str(exc_info)


def test_int2int_from_ptr(int2int_map):
    int2int_map[1] = 101
    int2int_map[2] = 102

    int2int_map.make_readonly()

    with pytest.raises(RuntimeError) as exc_info:
        int2int_map[3] = 103
    assert "Instance is read-only" in str(exc_info)

    addr = int2int_map.buffer_ptr
    int2int_new_map = Int2Int.from_ptr(addr)

    assert int2int_new_map[1] == 101
    assert int2int_new_map[2] == 102

    with pytest.raises(RuntimeError) as exc_info:
        int2int_new_map[3] = 103
    assert "Instance is read-only" in str(exc_info)


def test_int2int_from_ptr_fail_when_not_readonly(int2int_map):
    with pytest.raises(RuntimeError) as exc_info:
        addr = int2int_map.buffer_ptr
        Int2Int.from_ptr(addr)
    assert "Instance must be read-only" in str(exc_info)


def test_int2int_iter_when_empty(int2int_map):
    assert set(int2int_map) == set()


def test_int2int_iter(int2int_map):
    for i in range(1, 7, 1):
        int2int_map[i] = 100 + i
    assert all(isinstance(k, int) for k in int2int_map.keys())
    assert set(int2int_map) == {1, 2, 3, 4, 5, 6}


def test_int2int_keys_when_empty(int2int_map):
    assert set(int2int_map.keys()) == set()


def test_int2int_keys(int2int_map):
    for i in range(1, 7, 1):
        int2int_map[i] = 100 + i
    assert all(isinstance(k, int) for k in int2int_map.keys())
    assert set(int2int_map.keys()) == {1, 2, 3, 4, 5, 6}


def test_int2int_values_when_empty(int2int_map):
    assert set(int2int_map.values()) == set()


def test_int2int_values(int2int_map):
    for i in range(1, 7, 1):
        int2int_map[i] = 100 + i
    assert all(isinstance(v, int) for v in int2int_map.values())
    assert set(int2int_map.values()) == {101, 102, 103, 104, 105, 106}


def test_int2int_items_when_empty(int2int_map):
    assert set(int2int_map.items()) == set()


def test_int2int_items(int2int_map):
    for i in range(1, 7, 1):
        int2int_map[i] = 100 + i
    assert all(isinstance(k, int) for k, unused_v in int2int_map.items())
    assert all(isinstance(v, int) for unused_k, v in int2int_map.items())
    assert set(int2int_map.items()) == {
        (1, 101), (2, 102), (3, 103), (4, 104), (5, 105), (6, 106)}


def test_int2int_equal_when_empty():
    a = Int2Int()
    b = Int2Int()
    assert a == b


def test_int2int_equal_when_same_keys_and_values():
    a = Int2Int()
    b = Int2Int()
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    assert a == b


def test_int2int_equal_with_dict_when_same_keys_and_values():
    a = Int2Int()
    b = {}
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    assert a == b


def test_int2int_not_equal_when_same_size_same_keys_but_different_values():
    a = Int2Int()
    b = Int2Int()
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    b[6] = 200
    assert a != b


def test_int2int_not_equal_when_same_size_same_values_but_different_keys():
    a = Int2Int()
    b = Int2Int()
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    a[7] = 200
    b[8] = 300
    assert a != b


def test_int2int_not_equal_when_different_size():
    a = Int2Int()
    b = Int2Int()
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    b[7] = 107
    assert a != b


@pytest.mark.parametrize(
    'other', [1, 'a', [1, 2]])
def test_int2int_eq_fail_when_invalid_other_object(int2int_map, other):
    with pytest.raises(TypeError) as exc_info:
        assert int2int_map == other
    assert "'other' is not either an Int2Int or a dict" in str(exc_info)


@pytest.mark.parametrize(
    'op', [operator.lt, operator.le, operator.gt, operator.ge])
def test_int2int_eq_fail_when_invalid_operator(int2int_map, op):
    with pytest.raises(TypeError) as exc_info:
        op(int2int_map, {})
    assert 'is not supported' in str(exc_info)


@pytest.mark.parametrize(
    'args, exc, msg',
    [
        [(-1, 8, 0, 10, False, b'\0'*24*10), TypeError, "'default' must be"],
        [('1', 8, 0, 10, False, b'\0'*24*10), TypeError, "'default' must be"],
        [(None, 8, 9, 10, False, b'\0'*24*10), ValueError, "Inconsistent"],
        [(None, 11, 0, 10, False, b'\0'*24*10), ValueError, "Inconsistent"],
        [(None, 8, 0, 10, False, b'\0'*24*9), ValueError, "Inconsistent"],
        [(None, 8, 0, 10, False, b'\0'*24*11), ValueError, "Inconsistent"],
    ])
def test_int2int_from_raw_data_fail_when_inconsistent_args(args, exc, msg):
    with pytest.raises(exc, match=msg):
        Int2Int._from_raw_data(*args)


def test_int2int_pickle_dumps_loads(int2int_map):
    for i in (1, 2, 3, 4, 5, 6):
        int2int_map[i] = 100 + i

    data = pickle.dumps(int2int_map)
    new = pickle.loads(data)

    with pytest.raises(KeyError):
        int2int_map[7]
    with pytest.raises(KeyError):
        new[7]
    assert new == int2int_map
    assert set(new.keys()) == {1, 2, 3, 4, 5, 6}
    assert set(new.values()) == {101, 102, 103, 104, 105, 106}


def test_int2int_pickle_dumps_loads_when_default():
    int2int_map = Int2Int(default=0)

    for i in (1, 2, 3, 4, 5):
        int2int_map[i] = 100 + i

    data = pickle.dumps(int2int_map)
    new = pickle.loads(data)

    assert int2int_map[6] == 0
    assert new[6] == 0
    assert new == int2int_map
    assert set(new.keys()) == {1, 2, 3, 4, 5, 6}
    assert set(new.values()) == {101, 102, 103, 104, 105, 0}


def test_int2int_readonly_flag_is_false(int2int_map):
    assert int2int_map.readonly is False


def test_int2int_readonly_flag_is_true(int2int_map):
    int2int_map.make_readonly()
    assert int2int_map.readonly is True


# Int2Float -------------------------------------------------------------------

@pytest.fixture(scope='function')
def int2float_map():
    return Int2Float(8)


def test_int2float_new_fail_when_invalid_size_arg():
    with pytest.raises(TypeError) as exc_info:
        Int2Float('8')
    assert "'str' object cannot be interpreted as an integer" in str(exc_info)


def test_int2float_new_fail_when_invalid_default_arg():
    with pytest.raises(TypeError) as exc_info:
        Int2Float(8, default='8')
    assert "'default' must be a float" in str(exc_info)


def test_int2float_len_when_empty(int2float_map):
    assert len(int2float_map) == 0


def test_int2float_len_when_not_empty(int2float_map):
    int2float_map[1] = 100
    assert len(int2float_map) == 1


def test_int2float_bool_when_empty(int2float_map):
    assert bool(int2float_map) is False


def test_int2float_bool_when_not_empty(int2float_map):
    int2float_map[1] = 100
    assert bool(int2float_map) is True


def test_int2float_repr_when_empty(int2float_map):
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Float: object at 0x[0-9a-f]+, used 0/8\>',
        repr(int2float_map))
    assert m is not None


def test_int2float_repr_when_not_empty(int2float_map):
    int2float_map[1] = 100
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Float: object at 0x[0-9a-f]+, used 1/8\>',
        repr(int2float_map))
    assert m is not None


def test_int2float_repr_when_empty_and_default_arg():
    int2float_map = Int2Float(8, default=100)
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Float: object at 0x[0-9a-f]+, '
        r'used 0/8, default 100.0\>',
        repr(int2float_map))
    assert m is not None


def test_int2float_repr_when_not_empty_and_default_kwarg():
    int2float_map = Int2Float(8, default=100)
    int2float_map[1] = 100
    m = re.match(
        r'\<cdatastructs.hashmap.Int2Float: object at 0x[0-9a-f]+, '
        r'used 1/8, default 100.0\>',
        repr(int2float_map))
    assert m is not None


def test_int2float_setitem_fail_when_invalid_key_type(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map['1'] = 100
    assert "'key' must be an integer" in str(exc_info)


def test_int2float_setitem_fail_when_negative_key(int2float_map):
    with pytest.raises(OverflowError) as exc_info:
        int2float_map[-1] = 100
    assert "can't convert negative int to unsigned" in str(exc_info)


def test_int2float_setitem_fail_when_key_is_too_big(int2float_map):
    with pytest.raises(OverflowError) as exc_info:
        int2float_map[2 ** 64] = 100
    assert "int too big to convert" in str(exc_info)


def test_int2float_setitem_fail_when_invalid_value_type(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map[1] = '100'
    assert "'value' must be a float" in str(exc_info)


def test_int2float_setitem_fail_when_size_exceeded(int2float_map):
    for i in range(8):
        int2float_map[i] = i + 100
    with pytest.raises(RuntimeError) as exc_info:
        int2float_map[9] = i + 100 + 1
    assert 'Maximum size has been exceeded' in str(exc_info)


@pytest.mark.parametrize('key', [0, (2 ** 64) - 1])
def test_int2float_setitem_getitem(int2float_map, key):
    int2float_map[key] = 100
    assert isinstance(int2float_map[key], float)
    assert int2float_map[key] == 100.0


def test_int2float_setitem_twice(int2float_map):
    int2float_map[1] = 100
    int2float_map[1] = 200
    assert isinstance(int2float_map[1], float)
    assert int2float_map[1] == 200.0


def test_int2float_setitem_delitem(int2float_map):
    int2float_map[1] = 100
    with pytest.raises(NotImplementedError) as exc_info:
        del int2float_map[1]
    assert "can't delete item" in str(exc_info)


def test_int2float_getitem_key_does_not_exist_and_default_arg():
    int2float_map = Int2Float(8, 100)
    assert isinstance(int2float_map[1], float)
    assert int2float_map[1] == 100.0
    assert len(int2float_map) == 1


def test_int2float_getitem_key_does_not_exist_and_default_kwarg():
    int2float_map = Int2Float(8, default=100)
    assert isinstance(int2float_map[1], float)
    assert int2float_map[1] == 100.0
    assert len(int2float_map) == 1


def test_int2float_getitem_key_does_not_exist_and_default_arg_is_none():
    int2float_map = Int2Float(8, None)
    with pytest.raises(KeyError) as exc_info:
        int2float_map[1]
    assert "'1'" in str(exc_info)


def test_int2float_getitem_fail_when_invalid_key_type(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map['1']
    assert "'key' must be an integer" in str(exc_info)


def test_int2float_getitem_fail_when_negative_key(int2float_map):
    with pytest.raises(OverflowError) as exc_info:
        int2float_map[-1]
    assert "can't convert negative int to unsigned" in str(exc_info)


def test_int2float_getitem_fail_when_key_is_too_big(int2float_map):
    with pytest.raises(OverflowError) as exc_info:
        int2float_map[2 ** 64]
    assert "int too big to convert" in str(exc_info)


def test_int2float_getitem_fail_when_key_does_not_exist(int2float_map):
    with pytest.raises(KeyError) as exc_info:
        int2float_map[1]
    assert "'1'" in str(exc_info)


def test_int2float_getitem_fail_when_size_exceeded():
    int2float_map = Int2Float(8, 0)
    for i in range(8):
        int2float_map[i] = i + 100
    with pytest.raises(RuntimeError) as exc_info:
        int2float_map[9]
    assert 'Maximum size has been exceeded' in str(exc_info)


def test_int2float_get(int2float_map):
    int2float_map[1] = 100
    assert isinstance(int2float_map.get(1), float)
    assert int2float_map.get(1) == 100.0


def test_int2float_get_when_key_does_not_exist(int2float_map):
    assert int2float_map.get(1) is None


def test_int2float_get_when_key_does_not_exist_and_default_arg(int2float_map):
    value = int2float_map.get(1, 100)
    assert isinstance(value, float)
    assert value == 100.0


def test_int2float_get_when_key_does_not_exist_and_default_kwarg(
        int2float_map):
    value = int2float_map.get(1, default=100)
    assert isinstance(value, float)
    assert value == 100.0


def test_int2float_get_fail_when_invalid_key_type(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map.get('1')
    exc_msg = str(exc_info)
    assert "'key' must be an integer" in exc_msg


def test_int2float_get_fail_when_negative_key(int2float_map):
    with pytest.raises(OverflowError) as exc_info:
        int2float_map.get(-1)
    assert "can't convert negative int to unsigned" in str(exc_info)


def test_int2float_get_fail_when_key_is_too_big(int2float_map):
    with pytest.raises(OverflowError) as exc_info:
        int2float_map.get(2 ** 64)
    assert "int too big to convert" in str(exc_info)


def test_int2float_get_fail_when_invalid_default_type(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map.get(1, '1')
    exc_msg = str(exc_info)
    assert "'default' must be a float" in exc_msg


def test_int2float_contains_when_key_exists(int2float_map):
    int2float_map[2541265478] = 100
    assert 2541265478 in int2float_map


def test_int2float_contains_when_key_does_not_exist(int2float_map):
    assert 2541265478 not in int2float_map


def test_int2float_contains_fail_when_invalid_key_type(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        bool('1' in int2float_map)
    exc_msg = str(exc_info)
    assert "'key' must be an integer" in exc_msg


def test_int2float_contains_fail_when_negative_key(int2float_map):
    with pytest.raises(OverflowError) as exc_info:
        bool(-1 in int2float_map)
    assert "can't convert negative int to unsigned" in str(exc_info)


def test_int2float_contains_fail_when_key_is_too_big(int2float_map):
    with pytest.raises(OverflowError) as exc_info:
        bool((2 ** 64) in int2float_map)
    assert "int too big to convert" in str(exc_info)


def test_int2float_get_ptr(int2float_map):
    int2float_map[1] = 100

    class Int2FloatHashTable_t(ctypes.Structure):

        _fields_ = [
            ('size', ctypes.c_size_t),
            ('current_size', ctypes.c_size_t),
            ('table_size', ctypes.c_size_t),
            ('table', ctypes.c_void_p),
        ]

    t = Int2FloatHashTable_t.from_address(int2float_map.get_ptr())
    assert t.size == 8
    assert t.current_size == 1


def test_int2float_iter_when_empty(int2float_map):
    assert set(int2float_map) == set()


def test_int2float_iter(int2float_map):
    for i in range(1, 7, 1):
        int2float_map[i] = 100 + i
    assert all(isinstance(k, int) for k in int2float_map.keys())
    assert set(int2float_map) == {1, 2, 3, 4, 5, 6}


def test_int2float_keys_when_empty(int2float_map):
    assert set(int2float_map.keys()) == set()


def test_int2float_keys(int2float_map):
    for i in range(1, 7, 1):
        int2float_map[i] = 100 + i
    assert all(isinstance(k, int) for k in int2float_map.keys())
    assert set(int2float_map.keys()) == {1, 2, 3, 4, 5, 6}


def test_int2float_values_when_empty(int2float_map):
    assert set(int2float_map.values()) == set()


def test_int2float_values(int2float_map):
    for i in range(1, 7, 1):
        int2float_map[i] = 100 + i
    expected = {101.0, 102.0, 103.0, 104.0, 105.0, 106.0}
    assert all(isinstance(v, float) for v in int2float_map.values())
    assert set(int2float_map.values()) == expected


def test_int2float_items_when_empty(int2float_map):
    assert set(int2float_map.items()) == set()


def test_int2float_items(int2float_map):
    for i in range(1, 7, 1):
        int2float_map[i] = 100 + i
    assert all(isinstance(k, int) for k, unused_v in int2float_map.items())
    assert all(isinstance(v, float) for unused_k, v in int2float_map.items())
    assert set(int2float_map.items()) == {
        (1, 101.0), (2, 102.0), (3, 103.0), (4, 104.0), (5, 105.0), (6, 106.0)}


def test_int2float_equal_when_empty():
    a = Int2Float(8)
    b = Int2Float(8)
    assert a == b


def test_int2float_equal_when_same_keys_and_values():
    a = Int2Float(8)
    b = Int2Float(8)
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    assert a == b


def test_int2float_equal_with_dict_when_same_keys_and_values():
    a = Int2Float(8)
    b = {}
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    assert a == b


def test_int2float_not_equal_when_same_size_same_keys_but_different_values():
    a = Int2Float(8)
    b = Int2Float(8)
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    b[6] = 200
    assert a != b


def test_int2float_not_equal_when_same_size_same_values_but_different_keys():
    a = Int2Float(8)
    b = Int2Float(8)
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    a[7] = 200
    b[8] = 300
    assert a != b


def test_int2float_not_equal_when_different_size():
    a = Int2Float(8)
    b = Int2Float(8)
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    b[7] = 107
    assert a != b


@pytest.mark.parametrize(
    'other', [1, 'a', [1, 2]])
def test_int2float_eq_fail_when_invalid_other_object(int2float_map, other):
    with pytest.raises(TypeError) as exc_info:
        assert int2float_map == other
    assert "'other' is not either an Int2Float or a dict" in str(exc_info)


@pytest.mark.parametrize(
    'op', [operator.lt, operator.le, operator.gt, operator.ge])
def test_int2float_eq_fail_when_invalid_operator(int2float_map, op):
    with pytest.raises(TypeError) as exc_info:
        op(int2float_map, {})
    assert 'is not supported' in str(exc_info)


def test_int2float_pickle_dumps(int2float_map):
    for i in range(1, 7, 1):
        int2float_map[i] = 100 + i
    data = pickle.dumps(int2float_map)
    import pprint
    pprint.pprint(data)

    assert data == (
        b'\x80\x03ccdatastructs.hashmap\nInt2Float\nq\x00K\x08N\x86q\x01Rq\x02'
        b'K\x06C\xf0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\xc0Y@\x01\x00\x00\x00\x00\x00\x00\x00'
        b'\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80Z@\x01\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80Y@\x01\x00\x00\x00'
        b'\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'@Z@\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@Y@\x01\x00\x00\x00'
        b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00Z@\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'q\x03\x86q\x04b.'
    )


def test_int2float_pickle_loads(int2float_map):
    for i in range(1, 7, 1):
        int2float_map[i] = 100 + i

    new = pickle.loads(
        b'\x80\x03ccdatastructs.hashmap\nInt2Float\nq\x00K\x08N\x86q\x01Rq\x02'
        b'K\x06C\xf0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\xc0Y@\x01\x00\x00\x00\x00\x00\x00\x00'
        b'\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80Z@\x01\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80Y@\x01\x00\x00\x00'
        b'\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'@Z@\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@Y@\x01\x00\x00\x00'
        b'\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00Z@\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'q\x03\x86q\x04b.'
    )

    assert new == int2float_map
    assert all(isinstance(k, int) for k in new.keys())
    assert set(new.keys()) == {1, 2, 3, 4, 5, 6}
    assert all(isinstance(v, float) for v in new.values())
    assert set(new.values()) == {101.0, 102.0, 103.0, 104.0, 105.0, 106.0}


def test_int2float_max_size(int2float_map):
    assert int2float_map.max_size == 8
