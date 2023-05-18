import sys
import os
import platform
BASEDIR = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(BASEDIR, 'swig'))


if platform.system() == 'Windows':
    import ctypes
    solverlib_path= os.path.join(BASEDIR, 'libs', 'cplex', 'lib', 'win64')
    for dll in ['cplex2211.dll', 'cplexmp-lib.dll']:
        try:
            ctypes.CDLL(os.path.join(solverlib_path, dll))
        except Exception as e:
            print("Problem importing library {}:\n{}\n".format(os.path.join(solverlib_path, dll), e))
try:
    from .patch import *
except:
    raise

__version__ = '0.1.0'
