from grbpy_c import *
import types

drv = GurobiDrv()

def exportGurobiModel(self):
    self.eval('write gnlfile;')
    grb_model = drv.loadModel('nlfile.nl') 
    return grb_model


def importGurobiSolution(self, grb_model):
    grb_model.writeSol()
    self.eval('solution nlfile.sol;')


def patch(ampl_class):
    ampl_class.exportGurobiModel = exportGurobiModel
    ampl_class.importGurobiSolution = importGurobiSolution



