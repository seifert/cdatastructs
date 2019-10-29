
from setuptools import setup, find_packages

from cdatastructs import __version__ as VERSION


setup(
    name='shmdemo',
    description='cdatastructs - demo how to use hashmap in shared memory',
    version=VERSION,
    author='Jan Seifert',
    author_email='jan.seifert@fotkyzcest.net',
    license='BSD',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
    ],
    packages=find_packages(),
    zip_safe=True,
    install_requires=[
        'cdatastructs',
        'sysv_ipc',
    ],
    entry_points={
        'console_scripts': [
            'shmdemo = shmdemo.mainprocess:main',
        ]
    },
)
