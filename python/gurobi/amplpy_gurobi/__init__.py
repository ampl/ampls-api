import sys
import os
import platform
BASEDIR = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(BASEDIR, 'swig'))


if platform.system() == 'Windows':
    import ctypes
    try:
        solverlib_path= os.path.join(BASEDIR, 'libs', 'gurobi', 'lib', 'win64')
        ctypes.CDLL(os.path.join(solverlib_path, 'gurobi100.dll'))
        ctypes.CDLL(os.path.join(solverlib_path, 'gurobi-lib.dll'))
    except Exception as e:
        print("Problem importing library:\n{}\n".format(e))


try:
    from .patch import *
except:
    raise

__version__ = '0.1.0'
