
import ctypes
import logging

LOG_FORMAT = "%(asctime)s %(levelname)-7s [%(process)d] %(name)-7s %(message)s"


def configure_logging():
    logging.basicConfig(level=logging.INFO, format=LOG_FORMAT)


class StopProcess(Exception):
    pass


class Int2IntHashTable_t(ctypes.Structure):

    _fields_ = [
        ('size', ctypes.c_size_t),
        ('current_size', ctypes.c_size_t),
        ('table_size', ctypes.c_size_t),
        ('readonly', ctypes.c_bool),
    ]
