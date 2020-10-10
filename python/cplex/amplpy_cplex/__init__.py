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
    from amplpy_cplex_swig import *
except:
    raise


CPLEX_DRIVER = CPLEXDrv()


def export_cplex_model(self):
    self.eval('write gnlfile;')
    cplex_model = CPLEX_DRIVER.loadModel('nlfile.nl')
    return cplex_model


def import_cplex_solution(self, cplex_model):
    cplex_model.writeSol()
    self.eval('solution nlfile.sol;')


def cplex_patch(ampl_class):
    ampl_class.export_cplex_model = export_cplex_model
    ampl_class.import_cplex_solution = import_cplex_solution
