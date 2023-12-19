#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
import os


class TestBase(unittest.TestCase):
    def setUp(self):
        print("Method:", self._testMethodName)
        os.chdir(os.path.dirname(__file__) or os.curdir)
        self._data_dir = os.path.join(os.curdir, "data")

    def tearDown(self):
        pass

class TestExamples(TestBase):
    def test_all(self):
        print(f"Execute {dir()}")


def get_filenames(directory, extension):
    filtered_files = []
    EXCLUSIONS = ["tsp_helpers", "callback_first_gurobi", "grb_callbacks_stopping",
                  " test_tsp_simple_cuts_gurobi"]
    for filename in os.listdir(directory):
        if not os.path.isdir(os.path.join(directory, filename)):
            if filename.endswith(extension):
                base_filename, _ = os.path.splitext(filename)
                if base_filename not in EXCLUSIONS:
                    filtered_files.append(base_filename)

    return filtered_files

def def_func(fn):
    fdef= f"""def test_{fn}(self):
            import {fn}"""
    fassign=f"TestExamples.test_{fn}=test_{fn}"
    exec(fdef)
    exec(fassign)


fns=get_filenames(os.curdir, "py")
for f in fns:
    def_func(f)

if __name__ == "__main__":
    unittest.main()
