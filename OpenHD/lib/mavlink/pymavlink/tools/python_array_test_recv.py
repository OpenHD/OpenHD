#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
from pymavlink import mavutil

master = mavutil.mavlink_connection("udp::14555", dialect="array_test")

while True:
    m = master.recv_msg()
    if m is not None:
        print(m)
