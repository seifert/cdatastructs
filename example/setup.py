
import os.path

from distutils.core import setup, Extension

from datastructs import get_c_sources_dir


setup(
    name='datastructs-example',
    author='Jan Seifert',
    author_email='jan.seifert@fotkyzcest.net',
    license='BSD',
    packages=['example'],
    install_requires=[
        'datastructs',
    ],
    ext_modules=[
        Extension(
            'example.computer',
            include_dirs=[get_c_sources_dir()],
            sources=[
                'example/computer.c',
                os.path.join(get_c_sources_dir(), 'hashmap.c'),
            ],
        ),
    ],
)
