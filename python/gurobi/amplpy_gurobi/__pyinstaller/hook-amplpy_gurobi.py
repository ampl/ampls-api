# PyInstaller hook for amplpy_gurobi (auto-discovered via pyinstaller40 entry points).
import os
from pathlib import Path

from PyInstaller.utils.hooks import collect_all

datas, binaries, hiddenimports = collect_all("amplpy_gurobi")

# collect_all may miss vendored solver shared libraries (package_data under libs/).
try:
    import amplpy_gurobi

    libs_root = Path(amplpy_gurobi.__file__).resolve().parent / "libs" / "gurobi" / "lib"
    if libs_root.is_dir():
        for arch_dir in sorted(libs_root.iterdir()):
            if not arch_dir.is_dir():
                continue
            dest = os.path.join("amplpy_gurobi", "libs", "gurobi", "lib", arch_dir.name)
            for pattern in ("*.dll", "*.so", "*.so.*", "*.dylib"):
                for lib in sorted(arch_dir.glob(pattern)):
                    binaries.append((str(lib), dest))
except ImportError:
    pass

hiddenimports += [
    "amplpy_gurobi_swig",
    "_amplpy_gurobi_swig",
]
