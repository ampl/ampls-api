import sys
import os
import platform

BASEDIR = os.path.abspath(os.path.dirname(__file__))

sys.path.append(os.path.join(BASEDIR, "swig"))

if platform.system() == "Windows":
    sysdir = "win64"
elif platform.system() == "Linux":
    sysdir = "linux64"
elif platform.system() == "Darwin":
    sysdir = "osx64"
else:
    print(f"Platform not recognized: {platform.system()}")

LIBPATH = os.path.join(BASEDIR, "libs", "xpress", "lib", sysdir)
os.environ["PATH"] = LIBPATH + os.pathsep + os.environ["PATH"]


try:
    from .patch import *
except:
    raise

__version__ = "0.1.0"
