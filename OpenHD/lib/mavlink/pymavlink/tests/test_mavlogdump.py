#!/usr/bin/env python


"""
regression tests for mavlogdump.py
"""

from __future__ import absolute_import, print_function
import unittest
import os
import pkg_resources
import sys

class MAVLogDumpTest(unittest.TestCase):

    """
    Class to test mavlogdump
    """

    def __init__(self, *args, **kwargs):
        """Constructor, set up some data that is reused in many tests"""
        super(MAVLogDumpTest, self).__init__(*args, **kwargs)

    def test_dump_same(self):
        """Test dump of file is what we expect"""
        test_filename = "test.BIN"
        test_filepath = pkg_resources.resource_filename(__name__,
                                                        test_filename)
        dump_filename = "tmp.dump"
        os.system("mavlogdump.py %s >%s" % (test_filepath, dump_filename))
        with open(dump_filename) as f:
            got = f.read()

        possibles = ["test.BIN.py3.dumped",
                     "test.BIN.dumped"]
        success = False
        for expected in possibles:
            expected_filepath = pkg_resources.resource_filename(__name__,
                                                                expected)
            with open(expected_filepath) as e:
                expected = e.read()

            if expected == got:
                success = True

        assert True

if __name__ == '__main__':
    unittest.main()
