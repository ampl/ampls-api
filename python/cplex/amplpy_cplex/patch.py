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

def _do_export(self, drv):
    import tempfile
    import shutil
    import os
    tmp = tempfile.mkdtemp()
    fname = os.path.join(tmp, 'model').replace('"', '""')
    try:
        self.option['auxfiles'] = 'c'
        self.eval('write "g{}";'.format(fname))
        model = drv.loadModel(fname + '.nl')
        model._solfile = fname + '.sol'
        os.remove(fname + '.nl')
        model._setCallback = model.setCallback
        model.setCallback = types.MethodType(setCallback, model)
        return model
    except:
        shutil.rmtree(tmp)
        raise



if __package__ == 'amplpy_copt':
    from amplpy_copt_swig import *
    COPT_DRIVER = COPTDrv()
    def exportCoptModel(self, options=None):
        global COPT_DRIVER
        return _do_export(self, COPT_DRIVER)

    try:
        from amplpy import AMPL
        AMPL.export_copt_model = exportCoptModel
    except:
        pass


if __package__ == 'amplpy_cplex':
    from amplpy_cplex_swig import *
    CPLEX_DRIVER = CPLEXDrv()
    def exportCplexModel(self, options=None):
        global CPLEX_DRIVER
        return _do_export(self, CPLEX_DRIVER)

    try:
        from amplpy import AMPL
        AMPL.export_cplex_model = exportCplexModel
    except:
        pass


if __package__ == 'amplpy_gurobi':
    from amplpy_gurobi_swig import *
    GUROBI_DRIVER = GurobiDrv()

    def exportGurobiModel(self, options=None):
        global GUROBI_DRIVER
        return _do_export(self, GUROBI_DRIVER)

    try:
        from amplpy import AMPL
        AMPL.export_gurobi_model = exportGurobiModel
    except:
        pass


if __package__ == 'amplpy_xpress':
    from amplpy_xpress_swig import *
    XPRESS_DRIVER = XPRESSDrv()

    def exportXpressModel(self, options=None):
        global XPRESS_DRIVER
        return _do_export(self, XPRESS_DRIVER)

    try:
        from amplpy import AMPL
        AMPL.export_xpress_model = exportXpressModel
    except:
        pass


def exportModel(self, driver):
    if driver == 'gurobi':
        return self.export_gurobi_model()
    elif driver == 'cplex':
        return self.export_cplex_model()
    elif driver == 'copt':
        return self.export_copt_model()
    elif driver == 'xpress':
        return self.export_xpress_model()


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
    AMPL.export_model = exportModel
    AMPL.import_solution = importSolution
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
