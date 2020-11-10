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


def exportGurobiModel(self):
    global GUROBI_DRIVER
    import tempfile
    import shutil
    import os
    tmp = tempfile.mkdtemp()
    fname = os.path.join(tmp, 'model').replace('"', '""')
    try:
        self.option['auxfiles'] = 'c'
        self.eval('write "g{}";'.format(fname))
        model = GUROBI_DRIVER.loadModel(fname + '.nl')
        model._solfile = fname + '.sol'
        os.remove(fname + '.nl')
        return model
    except:
        shutil.rmtree(tmp)
        raise


def exportCplexModel(self):
    global CPLEX_DRIVER
    import tempfile
    import shutil
    import os
    tmp = tempfile.mkdtemp()
    fname = os.path.join(tmp, 'model').replace('"', '""')
    try:
        self.option['auxfiles'] = 'c'
        self.eval('write "g{}";'.format(fname))
        model = CPLEX_DRIVER.loadModel(fname + '.nl')
        model._solfile = fname + '.sol'
        os.remove(fname + '.nl')
        return model
    except:
        shutil.rmtree(tmp)
        raise


def exportModel(self, driver):
    if driver == 'gurobi':
        return self.exportGurobiModel()
    elif driver == 'cplex':
        return self.exportCplexModel()
    return None


def importSolution(self, model):
    import os
    model.writeSol()
    self.eval('solution "{}";'.format(model._solfile))
    os.remove(model._solfile)


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
