# -*- coding: utf-8 -*-
"""
AMPLPY-GUROBI
-------------

GUROBI driver for AMPL. This package uses AMPLS-API, which is an open source set of lightweight
interfaces between AMPL and solvers, which allow:

- Read in an AMPL model instance from an `NL` file
- Write out the solution as a `sol` file, ready to be imported by AMPL
- A choiche between:
   - Use of all the solver's capabilities, using its own C API functionalities
   - Use of a (provided) generic interface, that encapsulates the most common 
     functionalities of the solver interfaces, permitting hassle-free solver swap
- Usage of existing AMPL licenses, when used together with the AMPL drivers

The interfaces are available for multiple languages; the core is written in C++ and it is wrapped using
[swig](https://www.swig.org) to other target languages.
"""
from setuptools import setup, Extension
import platform
import os

OSTYPE = platform.system()
x64 = platform.architecture()[0] == '64bit'


def ls_dir(base_dir):
    """List files recursively."""
    return [
        os.path.join(dirpath.replace(base_dir, '', 1), f)
        for (dirpath, dirnames, files) in os.walk(base_dir)
        for f in files
    ]


def compile_args():
    if OSTYPE == 'Windows':
        return ['/TP', '/EHsc']
    elif OSTYPE == 'Linux':
        ignore_warnings = [
            '-Wno-stringop-truncation',
            '-Wno-catch-value',
            '-Wno-unused-variable',
        ]
        return ['-std=c++11'] + ignore_warnings
    elif OSTYPE == 'Darwin':
        ignore_warnings = [
            '-Wno-unused-variable',
        ]
        return ['-std=c++11', '-mmacosx-version-min=10.9'] + ignore_warnings
    else:
        return []


def libdir():
    if OSTYPE == 'Darwin':
        assert x64 is True
        return 'osx64'
    elif OSTYPE == 'Linux':
        assert x64 is True
        return 'linux64'
    elif OSTYPE == 'Windows':
        assert x64 is True
        return 'win64'


def link_args():
    rpaths = [
        os.path.join('amplpy_gurobi', 'libs', 'ampls', libdir()),
        os.path.join('amplpy_gurobi', 'libs', 'gurobi', 'lib', libdir()),
    ]
    if OSTYPE == 'Darwin':
        return ['-Wl,-rpath,@loader_path/' + rpath for rpath in rpaths]
    elif OSTYPE == 'Linux':
        return ['-Wl,-rpath,$ORIGIN/' + rpath for rpath in rpaths]
    else:
        # Return [] instead of [''] for Windows in order to avoid:
        #  cannot open input file '.obj' in build on distutils from Python 3.9
        # https://github.com/pypa/setuptools/issues/2417
        return []


setup(
    name='amplpy_gurobi',
    version='0.1.0b18',
    description='GUROBI extension for amplpy',
    long_description=__doc__,
    license='BSD-3',
    platforms='any',
    author='Filipe Brandão',
    author_email='fdabrandao@ampl.com',
    url='http://ampl.com/',
    download_url='https://github.com/ampl/ampls-api',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Environment :: Console',
        'Topic :: Software Development',
        'Topic :: Scientific/Engineering',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: BSD License',
        'Operating System :: POSIX',
        'Operating System :: Unix',
        'Operating System :: MacOS',
        'Operating System :: Microsoft :: Windows',
        'Programming Language :: C++',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: Implementation :: CPython',
    ],
    packages=['amplpy_gurobi'],
    ext_modules=[Extension(
        '_amplpy_gurobi_swig',
        library_dirs=[
            os.path.join('amplpy_gurobi', 'libs', 'gurobi', 'lib', libdir()),
            os.path.join('amplpy_gurobi', 'libs', 'ampls', libdir()),
        ],
        define_macros=[('SWIG', 1)],
        include_dirs=[
            os.path.join('amplpy_gurobi', 'libs', 'gurobi', 'include'),
            os.path.join('amplpy_gurobi', 'swig'),
            os.path.join('amplpy_gurobi', 'cpp', 'gurobi', 'include'),
            os.path.join('amplpy_gurobi', 'cpp', 'ampls', 'include'),
        ],
        libraries=['gurobi91', 'gurobi-lib'],
        extra_compile_args=compile_args(),
        extra_link_args=link_args(),
        sources=[
            os.path.join('amplpy_gurobi', 'swig',
                         'amplpy_gurobi_swig_wrap.cxx')
        ] + [
            os.path.join('amplpy_gurobi', 'cpp', 'ampls', 'src', fname)
            for fname in ls_dir(os.path.join('amplpy_gurobi', 'cpp', 'ampls', 'src'))
            if fname.endswith(('.c', '.cpp'))
        ] + [
            os.path.join('amplpy_gurobi', 'cpp', 'gurobi', 'src', fname)
            for fname in ls_dir(os.path.join('amplpy_gurobi', 'cpp', 'gurobi', 'src'))
            if fname.endswith(('.c', '.cpp'))
        ],
    )],
    package_data={'': ls_dir('amplpy_gurobi/')},
)
