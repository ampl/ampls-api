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

if platform.system() == "Windows":
    import ctypes

    LIBPATH = os.path.join(BASEDIR, "libs", "copt", "lib", "win64")
    for dll in ["copt.dll", "copt-lib.dll"]:
        try:
            ctypes.CDLL(os.path.join(LIBPATH, dll))
        except Exception as e:
            print(
                "Problem importing library {}:\n{}\n".format(
                    os.path.join(LIBPATH, dll), e
                )
            )


try:
    from .patch import *
except:
    raise

__version__ = "0.2.1"
