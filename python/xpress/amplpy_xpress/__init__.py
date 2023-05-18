import sys
import os
import platform
BASEDIR = os.path.abspath(os.path.dirname(__file__))
sys.path.append(os.path.join(BASEDIR, 'swig'))


if platform.system() == 'Windows':
    import ctypes
    try:
        solverlib_path= os.path.join(BASEDIR, 'libs', 'xpress', 'lib', 'win64')
        for dll in ['xprs.dll', 'xprl.dll', 'xpress-lib.dll']:
            ctypes.CDLL(os.path.join(solverlib_path, dll))
    except Exception as e:
        print("Problem importing library:\n{}\n".format(e))



try:
    from .patch import *
except:
    raise

__version__ = '0.1.0'
