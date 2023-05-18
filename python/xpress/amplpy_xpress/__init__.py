import sys
import os
import platform
BASEDIR = os.path.abspath(os.path.dirname(__file__))

sys.path.append(os.path.join(BASEDIR, 'swig'))

if platform.system() == 'Windows':
    sysdir = 'win64'
elif platform.system() == 'Linux':
    sysdir = 'linux64'
elif platform.system() == 'Darwin':
   sysdir = 'osx64'
else:
    print("Platform not recognized: {}", platform.system())

LIBPATH=os.path.join(BASEDIR, 'libs', 'xpress', 'lib', sysdir)
# Add path to help finding the license file
sys.path.insert(1,LIBPATH)
print("Addedd {}".format(LIBPATH))
LICFILE=os.path.join(LIBPATH, 'xpauth.xpr')
print("License file does ".format(LICFILE))
if not os.path.exists(LICFILE):
    print("not ")
print("exists.")

if platform.system() == 'Windows':
    import ctypes
    for dll in ['xprl.dll', 'xprs.dll','xpress-lib.dll']:
        try:
            ctypes.CDLL(os.path.join(LIBPATH, dll))
        except Exception as e:
            print("Problem importing library {}:\n{}\n".format(os.path.join(LIBPATH, dll), e))



try:
    from .patch import *
except:
    raise

__version__ = '0.1.0'
