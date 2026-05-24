# PyInstaller hook registration (see https://pyinstaller.org/en/stable/hooks.html)
from pathlib import Path

_DIR = Path(__file__).resolve().parent


def get_hook_dirs():
    """Return directories searched for hook-amplpy_copt.py."""
    return [str(_DIR)]


def get_rthooks():
    """Return runtime hooks executed before the packaged application main script."""
    return [str(_DIR / "pyi_rth_amplpy_copt.py")]
