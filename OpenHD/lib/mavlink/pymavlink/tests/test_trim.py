#!/usr/bin/python


"""
test for trimming under Python 3
"""

from __future__ import absolute_import, print_function
import unittest
import os
import pkg_resources
import sys
from pymavlink import mavutil

class PayLoadTrimZeros(unittest.TestCase):
    '''Trivial test for trimming zeros from end of messages'''

    def test_dump_length(self):
        mavutil.mavlink.WIRE_PROTOCOL_VERSION = 2
        mav = mavutil.mavudp(":12345")

        ts = [ ((1, 1), 10),
              ((1, 0), 9),
              ((0, 0), 9)
              ]
        for t in ts:
            ((sysid, compid), result) = t
            m = mavutil.mavlink.MAVLink_param_request_list_message(sysid, compid)
            packed = m.pack(mav.mav)
            print("(%u/%u should be %u" % (sysid,compid, result))
            self.assertEqual(len(packed), result)

if __name__ == '__main__':
    unittest.main()
