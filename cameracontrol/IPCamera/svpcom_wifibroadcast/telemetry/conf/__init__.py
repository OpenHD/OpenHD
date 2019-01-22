#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
from telemetry import config_parser

_cfg_files = [ 'master.cfg', 'site.cfg', '/etc/wifibroadcast.cfg', 'local.cfg' ]   # local.cfg is for debug only


def _parse_config(telemetry_cfg=None):
    return config_parser.parse_config(os.path.join(os.getcwd(), os.path.dirname(__file__)), telemetry_cfg or _cfg_files)


try:
    settings, cfg_files = _parse_config()
except Exception, v:
    print >>sys.stderr, 'ERROR: Unable to parse config: %s' % (v,)
    _cfg_files = [ 'master.cfg', 'site.cfg' ]
    settings, cfg_files = _parse_config()
