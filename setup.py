
from setuptools import setup, find_packages, Extension

from cdatastructs import __version__ as VERSION

_description = (
    'Simple data structures accessible from both Python and C. '
    'Data in structures are stored as a primitive C types, so in '
    'C you can compute data without Python overhead.'
)

try:
    _long_description = open('README.rst', 'rb').read().decode('utf-8')
    _long_description_content_type = 'text/x-rst'
except IOError:
    _long_description = _description
    _long_description_content_type = 'text/plain'

setup(
    name='cdatastructs',
    description=_description,
    long_description=_long_description,
    long_description_content_type=_long_description_content_type,
    version=VERSION,
    author='Jan Seifert',
    author_email='jan.seifert@fotkyzcest.net',
    license='BSD',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: C',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
    ],
    packages=find_packages(exclude=['tests*']),
    include_package_data=True,
    zip_safe=False,
    ext_modules=[
        Extension(
            name='cdatastructs.hashmap',
            sources=['cdatastructs/_hashmap.c', 'cdatastructs/hashmap.c'],
        ),
    ],
)
