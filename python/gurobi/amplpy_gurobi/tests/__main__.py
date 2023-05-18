#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import

import unittest
import os

from .test_callbacks import TestCallbacks

if __name__ == "__main__":
    from amplpy import modules

    uuid = os.environ.get("AMPLKEY_UUID", None)
    if uuid is not None and uuid != "":
        modules.activate(uuid)

    unittest.main()
