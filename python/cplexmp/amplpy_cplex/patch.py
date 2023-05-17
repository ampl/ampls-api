import types


def setCallback(self, cb):
    def run(self):
        try:
            return self._run()
        except Exception as e:
            print('Error:', e)
            return 0

    cb._run = cb.run
    cb.run = types.MethodType(run, cb)
    super(type(cb), cb).__init__()
    self._setCallback(cb)


if __package__ == 'amplpy_cplex':
    from amplpy_cplex_swig import *
    CPLEX_DRIVER = CPLEXDrv()

    def exportCplexModel(self, options=None):
        global CPLEX_DRIVER
        import tempfile
        import shutil
        import os
        tmp = tempfile.mkdtemp()
        fname = os.path.join(tmp, 'model').replace('"', '""')
        try:
            if options:
                CPLEX_DRIVER.setOptions(options)
            self.option['auxfiles'] = 'c'
            self.eval('write "g{}";'.format(fname))
            model = CPLEX_DRIVER.loadModel(fname + '.nl')
            model._solfile = fname + '.sol'
            os.remove(fname + '.nl')
            model._setCallback = model.setCallback
            model.setCallback = types.MethodType(setCallback, model)
            return model
        except:
            shutil.rmtree(tmp)
            raise

    try:
        from amplpy import AMPL
        AMPL.exportCplexModel = exportCplexModel
    except:
        pass

if __package__ == 'amplpy_gurobi':
    from amplpy_gurobi_swig import *
    GUROBI_DRIVER = GurobiDrv()

    def exportGurobiModel(self, options=None):
        global GUROBI_DRIVER
        import tempfile
        import shutil
        import os
        tmp = tempfile.mkdtemp()
        fname = os.path.join(tmp, 'model').replace('"', '""')
        try:
            if options:
                GUROBI_DRIVER.setOptions(options)
            self.option['auxfiles'] = 'c'
            self.eval('write "g{}";'.format(fname))
            model = GUROBI_DRIVER.loadModel(fname + '.nl')
            model._solfile = fname + '.sol'
            os.remove(fname + '.nl')
            model._setCallback = model.setCallback
            model.setCallback = types.MethodType(setCallback, model)
            return model
        except:
            shutil.rmtree(tmp)
            raise

    try:
        from amplpy import AMPL
        AMPL.exportGurobiModel = exportGurobiModel
    except:
        pass


def exportModel(self, driver, options=None):
    if driver == 'gurobi':
        return self.exportGurobiModel(options)
    elif driver == 'cplex':
        return self.exportCplexModel(options)


def importSolution(self, model):
    if isinstance(model, dict):
        self.eval(''.join(
            'let {} := {};'.format(name, value)
            for name, value in model.items()
        ))
        return
    import os
    model.writeSol()
    self.eval('solution "{}";'.format(model._solfile))
    os.remove(model._solfile)


try:
    from amplpy import AMPL
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
    indices = varname[varname.find('[')+1: -1].split(',')
    for i in range(len(indices)):
        indices[i] = indices[i].strip(' ')
        if indices[i][0] in ('\'', '\"'):
            indices[i] = indices[i][1:-1]
        else:
            try:
                indices[i] = int(indices[i])
            except:
                pass
    return tuple([varname[:varname.find('[')]] + indices)
