
import ctypes
import operator
import pickle
import re

import pytest

from datastructs.hashmap import Int2Int, Int2Float


# Int2Int ---------------------------------------------------------------------

@pytest.fixture(scope='function')
def int2int_map():
    return Int2Int(8)


def test_int2int_new_fail_when_invalid_size_arg():
    with pytest.raises(TypeError) as exc_info:
        Int2Int('8')
    assert "'str' object cannot be interpreted as an integer" in str(exc_info)


def test_int2int_new_fail_when_invalid_default_arg():
    with pytest.raises(TypeError) as exc_info:
        Int2Int(8, default='8')
    assert "'default' must be an integer" in str(exc_info)


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
        r'\<datastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, used 0/8\>',
        repr(int2int_map))
    assert m is not None


def test_int2int_repr_when_not_empty(int2int_map):
    int2int_map[1] = 100
    m = re.match(
        r'\<datastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, used 1/8\>',
        repr(int2int_map))
    assert m is not None


def test_int2int_repr_when_empty_and_default_arg():
    int2int_map = Int2Int(8, default=100)
    m = re.match(
        r'\<datastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, '
        r'used 0/8, default 100\>',
        repr(int2int_map))
    assert m is not None


def test_int2int_repr_when_not_empty_and_default_arg():
    int2int_map = Int2Int(8, default=100)
    int2int_map[1] = 100
    m = re.match(
        r'\<datastructs.hashmap.Int2Int: object at 0x[0-9a-f]+, '
        r'used 1/8, default 100\>',
        repr(int2int_map))
    assert m is not None


def test_int2int_setitem_fail_when_invalid_key(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map['1'] = 100
    assert "'key' must be an integer" in str(exc_info)


def test_int2int_setitem_fail_when_invalid_value(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map[1] = '100'
    assert "'value' must be an integer" in str(exc_info)


def test_int2int_setitem_fail_when_size_exceeded(int2int_map):
    for i in range(8):
        int2int_map[i] = i + 100
    with pytest.raises(RuntimeError) as exc_info:
        int2int_map[9] = i + 100 + 1
    assert 'Maximum size has been exceeded' in str(exc_info)


def test_int2int_setitem_getitem(int2int_map):
    int2int_map[1] = 100
    assert isinstance(int2int_map[1], int)
    assert int2int_map[1] == 100


def test_int2int_setitem_twice(int2int_map):
    int2int_map[1] = 100
    int2int_map[1] = 200
    assert isinstance(int2int_map[1], int)
    assert int2int_map[1] == 200


def test_int2int_setitem_delitem(int2int_map):
    int2int_map[1] = 100
    with pytest.raises(NotImplementedError) as exc_info:
        del int2int_map[1]
    assert "can't delete item" in str(exc_info)


def test_int2int_getitem_key_does_not_exist_and_default_arg():
    int2int_map = Int2Int(8, 100)
    assert isinstance(int2int_map[1], int)
    assert int2int_map[1] == 100
    assert len(int2int_map) == 1


def test_int2int_getitem_key_does_not_exist_and_default_kwarg():
    int2int_map = Int2Int(8, default=100)
    assert isinstance(int2int_map[1], int)
    assert int2int_map[1] == 100
    assert len(int2int_map) == 1


def test_int2int_getitem_key_does_not_exist_and_default_arg_is_none():
    int2int_map = Int2Int(8, None)
    with pytest.raises(KeyError) as exc_info:
        int2int_map[1]
    assert "'1'" in str(exc_info)


def test_int2int_getitem_fail_when_invalid_key(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map['1']
    assert "'key' must be an integer" in str(exc_info)


def test_int2int_getitem_fail_when_key_does_not_exist(int2int_map):
    with pytest.raises(KeyError) as exc_info:
        int2int_map[1]
    assert "'1'" in str(exc_info)


def test_int2int_getitem_fail_when_size_exceeded():
    int2int_map = Int2Int(8, 0)
    for i in range(8):
        int2int_map[i] = i + 100
    with pytest.raises(RuntimeError) as exc_info:
        int2int_map[9]
    assert 'Maximum size has been exceeded' in str(exc_info)


def test_int2int_get(int2int_map):
    int2int_map[1] = 100
    assert isinstance(int2int_map.get(1), int)
    assert int2int_map.get(1) == 100


def test_int2int_get_when_key_does_not_exist(int2int_map):
    assert int2int_map.get(1) is None


def test_int2int_get_when_key_does_not_exist_and_default_arg(int2int_map):
    value = int2int_map.get(1, 100)
    assert isinstance(value, int)
    assert value == 100.0


def test_int2int_get_when_key_does_not_exist_and_default_kwarg(int2int_map):
    value = int2int_map.get(1, default=100)
    assert isinstance(value, int)
    assert value == 100.0


def test_int2int_get_fail_when_invalid_key(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map.get('1')
    exc_msg = str(exc_info)
    assert 'argument 1 must be int, not str' in exc_msg


def test_int2int_get_fail_when_invalid_default(int2int_map):
    with pytest.raises(TypeError) as exc_info:
        int2int_map.get(1, '1')
    exc_msg = str(exc_info)
    assert "'default' must be an integer" in exc_msg


def test_int2int_contains_when_key_exists(int2int_map):
    int2int_map[2541265478] = 100
    assert 2541265478 in int2int_map


def test_int2int_contains_when_key_does_not_exist(int2int_map):
    assert 2541265478 not in int2int_map


def test_int2int_get_ptr(int2int_map):
    int2int_map[1] = 100

    class Int2IntHashTable_t(ctypes.Structure):

        _fields_ = [
            ('size', ctypes.c_size_t),
            ('current_size', ctypes.c_size_t),
            ('table_size', ctypes.c_size_t),
            ('table', ctypes.c_void_p),
        ]

    t = Int2IntHashTable_t.from_address(int2int_map.get_ptr())
    assert t.size == 8
    assert t.current_size == 1


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
    a = Int2Int(8)
    b = Int2Int(8)
    assert a == b


def test_int2int_equal_when_same_keys_and_values():
    a = Int2Int(8)
    b = Int2Int(8)
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    assert a == b


def test_int2int_equal_with_dict_when_same_keys_and_values():
    a = Int2Int(8)
    b = {}
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    assert a == b


def test_int2int_not_equal_when_same_size_same_keys_but_different_values():
    a = Int2Int(8)
    b = Int2Int(8)
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    b[6] = 200
    assert a != b


def test_int2int_not_equal_when_same_size_same_values_but_different_keys():
    a = Int2Int(8)
    b = Int2Int(8)
    for i in range(1, 7, 1):
        a[i] = 100 + i
        b[i] = 100 + i
    a[7] = 200
    b[8] = 300
    assert a != b


def test_int2int_not_equal_when_different_size():
    a = Int2Int(8)
    b = Int2Int(8)
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


def test_int2int_pickle_dumps(int2int_map):
    for i in range(1, 7, 1):
        int2int_map[i] = 100 + i
    data = pickle.dumps(int2int_map)

    assert data == (
        b'\x80\x03cdatastructs.hashmap\nInt2Int\nq\x00K\x08N\x86q\x01Rq\x02K'
        b'\x06B\x80\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00e\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00f\x00\x00'
        b'\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00'
        b'\x00\x00\x00\x00\x00g\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00h\x00\x00'
        b'\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00'
        b'\x00\x00\x00\x00\x00i\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00j\x00\x00'
        b'\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00q\x03\x86q\x04b.'
    )


def test_int2int_pickle_loads(int2int_map):
    for i in range(1, 7, 1):
        int2int_map[i] = 100 + i

    new = pickle.loads(
        b'\x80\x03cdatastructs.hashmap\nInt2Int\nq\x00K\x08N\x86q\x01Rq\x02K'
        b'\x06B\x80\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00e\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00f\x00\x00'
        b'\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00'
        b'\x00\x00\x00\x00\x00g\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00h\x00\x00'
        b'\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00'
        b'\x00\x00\x00\x00\x00i\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00j\x00\x00'
        b'\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00q\x03\x86q\x04b.'
    )

    assert new == int2int_map
    assert all(isinstance(k, int) for k in new.keys())
    assert set(new.keys()) == {1, 2, 3, 4, 5, 6}
    assert all(isinstance(v, int) for v in new.values())
    assert set(new.values()) == {101, 102, 103, 104, 105, 106}


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
        r'\<datastructs.hashmap.Int2Float: object at 0x[0-9a-f]+, used 0/8\>',
        repr(int2float_map))
    assert m is not None


def test_int2float_repr_when_not_empty(int2float_map):
    int2float_map[1] = 100
    m = re.match(
        r'\<datastructs.hashmap.Int2Float: object at 0x[0-9a-f]+, used 1/8\>',
        repr(int2float_map))
    assert m is not None


def test_int2float_repr_when_empty_and_default_arg():
    int2float_map = Int2Float(8, default=100)
    m = re.match(
        r'\<datastructs.hashmap.Int2Float: object at 0x[0-9a-f]+, '
        r'used 0/8, default 100.0\>',
        repr(int2float_map))
    assert m is not None


def test_int2float_repr_when_not_empty_and_default_kwarg():
    int2float_map = Int2Float(8, default=100)
    int2float_map[1] = 100
    m = re.match(
        r'\<datastructs.hashmap.Int2Float: object at 0x[0-9a-f]+, '
        r'used 1/8, default 100.0\>',
        repr(int2float_map))
    assert m is not None


def test_int2float_setitem_fail_when_invalid_key(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map['1'] = 100
    assert "'key' must be an integer" in str(exc_info)


def test_int2float_setitem_fail_when_invalid_value(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map[1] = '100'
    assert "'value' must be a float" in str(exc_info)


def test_int2float_setitem_fail_when_size_exceeded(int2float_map):
    for i in range(8):
        int2float_map[i] = i + 100
    with pytest.raises(RuntimeError) as exc_info:
        int2float_map[9] = i + 100 + 1
    assert 'Maximum size has been exceeded' in str(exc_info)


def test_int2float_setitem_getitem(int2float_map):
    int2float_map[1] = 100
    assert isinstance(int2float_map[1], float)
    assert int2float_map[1] == 100.0


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


def test_int2float_getitem_fail_when_invalid_key(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map['1']
    assert "'key' must be an integer" in str(exc_info)


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


def test_int2float_get_fail_when_invalid_key(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map.get('1')
    exc_msg = str(exc_info)
    assert 'argument 1 must be int, not str' in exc_msg


def test_int2float_get_fail_when_invalid_default(int2float_map):
    with pytest.raises(TypeError) as exc_info:
        int2float_map.get(1, '1')
    exc_msg = str(exc_info)
    assert "'default' must be a float" in exc_msg


def test_int2float_contains_when_key_exists(int2float_map):
    int2float_map[2541265478] = 100
    assert 2541265478 in int2float_map


def test_int2float_contains_when_key_does_not_exist(int2float_map):
    assert 2541265478 not in int2float_map


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
        b'\x80\x03cdatastructs.hashmap\nInt2Float\nq\x00K\x08N\x86q\x01Rq\x02'
        b'K\x06B\x80\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@Y@\x01\x00\x00\x00\x00\x00'
        b'\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80Y@'
        b'\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\xc0Y@\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00Z@\x01\x00\x00\x00'
        b'\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'@Z@\x01\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x80Z@\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00q\x03\x86q\x04b.'
    )


def test_int2float_pickle_loads(int2float_map):
    for i in range(1, 7, 1):
        int2float_map[i] = 100 + i

    new = pickle.loads(
        b'\x80\x03cdatastructs.hashmap\nInt2Float\nq\x00K\x08N\x86q\x01Rq\x02'
        b'K\x06B\x80\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@Y@\x01\x00\x00\x00\x00\x00'
        b'\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80Y@'
        b'\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\xc0Y@\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00Z@\x01\x00\x00\x00'
        b'\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'@Z@\x01\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x80Z@\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00q\x03\x86q\x04b.'
    )

    assert new == int2float_map
    assert all(isinstance(k, int) for k in new.keys())
    assert set(new.keys()) == {1, 2, 3, 4, 5, 6}
    assert all(isinstance(v, float) for v in new.values())
    assert set(new.values()) == {101.0, 102.0, 103.0, 104.0, 105.0, 106.0}
