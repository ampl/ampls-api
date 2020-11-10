try:
    from amplpy_gurobi_swig import *
    GUROBI_DRIVER = GurobiDrv()
except:
    pass

try:
    from amplpy_cplex_swig import *
    CPLEX_DRIVER = CPLEXDrv()
except:
    pass

try:
    from amplpy_ampls_swig import *
    GUROBI_DRIVER = GurobiDrv()
    CPLEX_DRIVER = CPLEXDrv()
except:
    pass


def exportGurobiModel(self):
    global GUROBI_DRIVER
    self.option['auxfiles'] = 'c'
    self.eval('write gnlfileexport;')
    model = GUROBI_DRIVER.loadModel('nlfileexport.nl')
    return model


def exportCplexModel(self):
    global CPLEX_DRIVER
    self.option['auxfiles'] = 'c'
    self.eval('write gnlfileexport;')
    model = CPLEX_DRIVER.loadModel('nlfileexport.nl')
    return model


def exportModel(self, driver):
    if driver == "gurobi":
        return self.exportGurobiModel()
    elif driver == "cplex":
        return self.exportCplexModel()
    return None


def importSolution(self, mod):
    mod.writeSol()
    self.eval('solution nlfileexport.sol;')


try:
    from amplpy import AMPL
    AMPL.exportGurobiModel = exportGurobiModel
    AMPL.exportCplexModel = exportCplexModel
    AMPL.exportModel = exportModel
    AMPL.importSolution = importSolution
except:
    pass


def tuple2var(varname, *args):
    def val(index):
        try:
            _ = float(index)
            return str(index)
        except ValueError:
            return '\'{}\''.format(index)
    if len(args) == 0:
        return varname
    return '{}[{}]'.format(varname, ','.join(val(x) for x in args))


def var2tuple(varname):
    return tuple([varname[:varname.find('[')]] + varname[varname.find('[')+1: -1].split(','))
