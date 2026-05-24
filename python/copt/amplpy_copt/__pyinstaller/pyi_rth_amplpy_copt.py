# PyInstaller runtime hook: make vendored Gurobi DLLs discoverable when frozen.
import os
import sys


def _platform_libdir():
    if sys.platform == "win32":
        return "win64"
    if sys.platform == "darwin":
        return "osx64"
    return "linux64"


if getattr(sys, "frozen", False):
    _base = getattr(sys, "_MEIPASS", os.path.dirname(os.path.abspath(sys.executable)))
    _libpath = os.path.join(
        _base, "amplpy_copt", "libs", "copt", "lib", _platform_libdir()
    )
    if os.path.isdir(_libpath):
        if sys.platform == "win32" and hasattr(os, "add_dll_directory"):
            os.add_dll_directory(_libpath)
        os.environ["PATH"] = _libpath + os.pathsep + os.environ.get("PATH", "")
