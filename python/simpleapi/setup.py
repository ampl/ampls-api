# -*- coding: utf-8 -*-
"""
AMPLPY-GUROBI
-------------

GUROBI driver for AMPL.

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


def make_relative_rpath(paths):
    if OSTYPE == 'Darwin':
        return '-Wl,'+','.join([
            '-rpath,@loader_path/' + path
            for path in paths
        ])
    elif OSTYPE == 'Linux':
        return '-Wl,'+','.join([
            '-rpath,$ORIGIN/' + path
            for path in paths
        ])
    else:
        return ''


def compile_args():
    if OSTYPE == 'Windows':
        return ['/TP', '/EHsc']
    elif OSTYPE == 'Linux':
        return []
    elif OSTYPE == 'Darwin':
        return []
    else:
        return []


def libdir():
    if OSTYPE == 'Darwin':
        assert x64 is True
        return 'macos64'
    elif OSTYPE == 'Linux':
        assert x64 is True
        return 'linux64'
    elif OSTYPE == 'Windows':
        assert x64 is True
        return 'win64'


setup(
    name='amplpy_gurobi',
    version='0.1.0b3',
    description='GUROBI extension for amplpy',
    long_description=__doc__,
    license='BSD-3',
    platforms='any',
    author='Filipe Brandão',
    author_email='fdabrandao@ampl.com',
    url='http://ampl.com/',
    download_url='https://github.com/ampl/amplpy',
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
        'Programming Language :: Python :: Implementation :: CPython',
    ],
    packages=['amplpy_gurobi'],
    ext_modules=[Extension(
        '_amplpy_gurobi_swig',
        library_dirs=[
            os.path.join('amplpy_gurobi', 'gurobi81', libdir()),
            os.path.join('amplpy_gurobi', 'amplgurobi', 'lib'),
        ],
        define_macros=[('SWIG', 1)],
        include_dirs=[
            os.path.join('amplpy_gurobi', 'gurobi81', 'include'),
            os.path.join('amplpy_gurobi', 'swig'),
            os.path.join('amplpy_gurobi', 'amplgurobi'),
            os.path.join('amplpy_gurobi', 'simpleapi', 'include'),
        ],
        libraries=['gurobi81', 'gurobidrv-lib'],
        extra_compile_args=compile_args(),
        extra_link_args=[make_relative_rpath([
            os.path.join('amplpy_gurobi', 'gurobi81', libdir()),
            os.path.join('amplpy_gurobi', 'amplgurobi', 'lib'),
        ])],
        sources=[
            os.path.join('amplpy_gurobi', 'swig',
                         'amplpy_gurobi_swig_wrap.cxx')
        ]+[
            os.path.join('amplpy_gurobi', 'simpleapi', fname)
            for fname in ls_dir(os.path.join('amplpy_gurobi', 'simpleapi'))
            if fname.endswith(('.c', '.cpp'))
        ]+[
            os.path.join('amplpy_gurobi', 'amplgurobi', fname)
            for fname in ls_dir(os.path.join('amplpy_gurobi', 'amplgurobi'))
            if fname.endswith(('.c', '.cpp'))
        ],
    )],
    package_data={'': ls_dir('amplpy_gurobi/')},
)