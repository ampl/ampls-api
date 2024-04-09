#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from test_callbacks import TestCallbacks
from test_multiple_models import TestMultipleModels
from test_multiple_solutions import TestMultipleSolutions
from test_options import TestOptions
from test_tsp import Test_Tsp

if __name__ == "__main__":
    from amplpy import modules

    uuid = os.environ.get("AMPLKEY_UUID", None)
    if uuid is not None and uuid != "":
        modules.activate(uuid)

    unittest.main()
