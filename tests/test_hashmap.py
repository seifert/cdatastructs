
import ctypes
import re

import pytest

from datastructs.hashmap import Int2Int


@pytest.fixture(scope='function')
def int2int_map():
    return Int2Int(8)


def test_new_fail_when_invalid_size_arg():
    with pytest.raises(TypeError) as exc_info:
        Int2Int('8')
    assert "'str' object cannot be interpreted as an integer" in str(exc_info)


def test_new_fail_when_invalid_default_arg():
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
    assert int2int_map[1] == 100


def test_int2int_setitem_twice(int2int_map):
    int2int_map[1] = 100
    int2int_map[1] = 200
    assert int2int_map[1] == 200


def test_int2int_setitem_delitem(int2int_map):
    int2int_map[1] = 100
    with pytest.raises(NotImplementedError) as exc_info:
        del int2int_map[1]
    assert "can't delete item" in str(exc_info)


def test_int2int_getitem_key_does_not_exist_and_default_arg():
    int2int_map = Int2Int(8, 100)
    assert int2int_map[1] == 100
    assert len(int2int_map) == 1


def test_int2int_getitem_key_does_not_exist_and_default_kwarg():
    int2int_map = Int2Int(8, default=100)
    assert int2int_map[1] == 100
    assert len(int2int_map) == 1


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
    assert int2int_map.get(1) == 100


def test_int2int_get_when_key_does_not_exist(int2int_map):
    assert int2int_map.get(1) is None


def test_int2int_get_when_key_does_not_exist_and_default_arg(int2int_map):
    assert int2int_map.get(1, 100) == 100


def test_int2int_get_when_key_does_not_exist_and_default_kwarg(int2int_map):
    assert int2int_map.get(1, default=100) == 100


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
