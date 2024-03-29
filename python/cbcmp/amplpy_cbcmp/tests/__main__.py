#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import

import unittest
import os

try:
    from .test_callbacks import TestCallbacks
    from .test_multiple_models import TestMultipleModels
    from .test_multiple_solutions import TestMultipleSolutions
except:
    from test_callbacks import TestCallbacks
    from test_multiple_models import TestMultipleModels
    from test_multiple_solutions import TestMultipleSolutions

if __name__ == "__main__":
    from amplpy import modules

    uuid = os.environ.get("AMPLKEY_UUID", None)
    if uuid is not None and uuid != "":
        modules.activate(uuid)

    unittest.main()
