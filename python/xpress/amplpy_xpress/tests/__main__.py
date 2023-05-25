#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
import os

from .test_multiple_models import TestMultipleModels
from .test_multiple_solutions import TestMultipleSolutions

if __name__ == "__main__":
    from amplpy import modules

    uuid = os.environ.get("AMPLKEY_UUID", None)
    if uuid is not None and uuid != "":
        modules.activate(uuid)

    unittest.main()
