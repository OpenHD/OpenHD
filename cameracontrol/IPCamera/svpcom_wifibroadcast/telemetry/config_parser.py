#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2018 Vasily Evseenko <svpcom@p2ptech.org>

#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; version 3.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

import ConfigParser
import ast
import copy
import glob
import os

from twisted.python import log


class ConfigError(Exception):
    pass


class Settings(object):
    # Used for interpolation of string values
    def __getitem__(self, attr):
        try:
            section, attr = attr.split('.')
        except ValueError:
            raise KeyError(attr)

        return getattr(getattr(self, section), attr)

    def __repr__(self):
        return repr(self.__dict__)

    def __deepcopy__(self, memo):
        return copy.deepcopy(self.__dict__, memo)


class Section(object):
    def __repr__(self):
        return repr(self.__dict__)

    def __deepcopy__(self, memo):
        return copy.deepcopy(self.__dict__, memo)


def parse_config(basedir, cfg_patterns, interpolate=True):
    settings = Settings()
    settings.path = Section()
    settings.path.cfg_root = basedir
    settings.common = Section()

    used_files = []

    for g in cfg_patterns:
        for f in (glob.glob(os.path.join(basedir, g)) if isinstance(g, basestring) else [g]):
            fd = open(f) if isinstance(f, basestring) else f
            filename = getattr(fd, 'filename', str(fd))

            try:
                fd.seek(0) # handle case when source config is fd
                config = ConfigParser.RawConfigParser()

                try:
                    config.readfp(fd, filename=filename)
                except Exception, v:
                    raise ConfigError(v)

                used_files.append(filename)
                fd.seek(0)

                for section in config.sections():
                    _s = getattr(settings, section, Section())

                    for item, value in config.items(section):
                        try:
                            value = ast.literal_eval(value)
                            if interpolate and isinstance(value, basestring):
                                # Interpolate string using current settings
                                value = value % settings
                        except:
                            err = '[%s] %s = %s' % (section, item, value)
                            log.msg('Config parse error: %s' % (err,), isError=1)
                            raise ConfigError('Parse error: %s' % (err,))

                        setattr(_s, item, value)

                    s_name=str(section)
                    setattr(settings, s_name, _s)
            finally:
                if isinstance(f, basestring):
                    fd.close()

    return settings, used_files
