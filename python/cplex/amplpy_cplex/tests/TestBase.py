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


if __name__ == "__main__":
    unittest.main()
