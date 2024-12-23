import sys
import os
import platform
BASEDIR = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(BASEDIR, 'swig'))


if platform.system() == 'Windows':
    from glob import glob
    import ctypes
    try:
        paths = [
            os.path.join(BASEDIR, 'libs', 'cplex', 'lib', 'win64'),
            os.path.join(BASEDIR, 'libs', 'ampls', 'win64'),
        ]
        for path in paths:
            dllfile = glob(os.path.join(path, '*.dll'))
            for d in dllfile: ctypes.CDLL(os.path.join(path, d))
    except:
        pass


try:
    from .patch import *
except:
    raise

__version__ = '0.2.2'
