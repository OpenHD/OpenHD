#!/usr/bin/env python
# -*- coding: utf-8 -*-
# returns the current date and time so you can track when the lib was built

from __future__ import print_function

import sys
import time
import datetime

def main(release):
    ttuple = time.gmtime()
    if not release:
        print ('%d.%d.%d.%d' % (ttuple.tm_year - 2000, ttuple.tm_mon, ttuple.tm_mday,
                               ttuple.tm_hour * 3600 + ttuple.tm_min * 60 + ttuple.tm_sec))
    else:
        delta = datetime.datetime.utcnow() - datetime.datetime(2000 + release[0], release[1], 1)
        print ('%d.%d.%s.%d' % (release[0], release[1], '0.%d' % (999 + delta.days,) if delta.days < 0 else (delta.days + 1),
                               ttuple.tm_hour * 3600 + ttuple.tm_min * 60 + ttuple.tm_sec))

if __name__ == '__main__':
    main(map(int, sys.argv[1].split('.')) if len(sys.argv) == 2 else None)
