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
            os.path.join(BASEDIR, 'libs', 'ampls', 'lib'),
        ]
        for path in paths:
            dllfile = glob(os.path.join(path, '*.dll'))[0]
            ctypes.CDLL(os.path.join(path, dllfile))
    except:
        pass


try:
    from .patch import *
except:
    raise
