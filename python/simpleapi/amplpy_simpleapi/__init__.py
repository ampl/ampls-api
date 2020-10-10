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
            os.path.join(BASEDIR, 'gurobi81', 'win64'),
            os.path.join(BASEDIR, 'amplgurobi', 'bin'),
        ]
        for path in paths:
            dllfile = glob(os.path.join(path, '*.dll'))[0]
            ctypes.CDLL(os.path.join(path, dllfile))
    except:
        pass


from amplpy_gurobi_swig import *

import types

DRIVER = GurobiDrv()

def exportGurobiModel(self):
    self.eval('write gnlfile;')
    grb_model = DRIVER.loadModel('nlfile.nl')
    return grb_model


def importGurobiSolution(self, grb_model):
    grb_model.writeSol()
    self.eval('solution nlfile.sol;')


def patch(ampl_class):
    ampl_class.exportGurobiModel = exportGurobiModel
    ampl_class.importGurobiSolution = importGurobiSolution
