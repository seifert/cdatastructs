"""
cdatastructs module provides simple data structures accessible from both
Python and C.
"""

__version__ = '0.2.0'


def get_c_sources_dir():
    """
    Return directory containig sources for building your own modules.
    """
    import os.path
    return os.path.abspath(os.path.dirname(__file__))
