
import os.path

from setuptools import setup, Extension

from cdatastructs import get_c_sources_dir, __version__ as VERSION


_author = 'Jan Seifert'
_author_email = 'jan.seifert@fotkyzcest.net'
_license = 'BSD'

setup(
    name='cdatastructs-c-module-example',
    description='cdatastructs - demo how to use hashmap for fast computing',
    version=VERSION,
    author=_author,
    author_email=_author_email,
    license=_license,
    packages=['c_module_example'],
    install_requires=[
        'cdatastructs',
    ],
    ext_modules=[
        Extension(
            'c_module_example.computer',
            include_dirs=[get_c_sources_dir()],
            sources=[
                'c_module_example/computer.c',
                os.path.join(get_c_sources_dir(), 'hashmap.c'),
            ],
        ),
    ],
    entry_points={
        'console_scripts': [
            'c-module-example = c_module_example.main:main',
        ],
    },
)

setup(
    name='cdatastructs-shmdemo',
    description='cdatastructs - demo how to use hashmap in shared memory',
    version=VERSION,
    author=_author,
    author_email=_author_email,
    license=_license,
    packages=['shmdemo'],
    zip_safe=True,
    install_requires=[
        'cdatastructs',
        'sysv_ipc',
    ],
    entry_points={
        'console_scripts': [
            'shmdemo = shmdemo.mainprocess:main',
        ],
    },
)
