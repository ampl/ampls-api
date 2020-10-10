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
            os.path.join(BASEDIR, 'libs', 'gurobi', 'lib', 'win64'),
            os.path.join(BASEDIR, 'libs', 'ampls', 'lib'),
        ]
        for path in paths:
            dllfile = glob(os.path.join(path, '*.dll'))[0]
            ctypes.CDLL(os.path.join(path, dllfile))
    except:
        pass


try:
    from amplpy_gurobi_swig import *
except:
    raise


GUROBI_DRIVER = GurobiDrv()


def export_gurobi_model(self):
    self.eval('write gnlfile;')
    grb_model = GUROBI_DRIVER.loadModel('nlfile.nl')
    return grb_model


def import_gurobi_solution(self, grb_model):
    grb_model.writeSol()
    self.eval('solution nlfile.sol;')


def gurobi_patch(ampl_class):
    ampl_class.export_gurobi_model = export_gurobi_model
    ampl_class.import_gurobi_solution = import_gurobi_solution
