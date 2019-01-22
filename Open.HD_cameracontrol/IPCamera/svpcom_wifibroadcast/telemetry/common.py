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

from twisted.python import log
from twisted.internet import reactor, defer, task
from telemetry.conf import settings
from twisted.internet.error import ReactorNotRunning

__system_failed = False


def fatal_error(stop_reactor=True):
    global __system_failed
    __system_failed = True

    if stop_reactor:
        try:
            reactor.stop()
        except ReactorNotRunning:
            pass


def exit_status():
    return 1 if __system_failed else 0


def abort_on_crash(f, stop_reactor=True, warn_cancel=True):
    if isinstance(f, defer.FirstError):
        f = f.value.subFailure

    if settings.common.debug:
        log.err(f, 'Stopping reactor due to fatal error')
    else:
        log.msg('Stopping reactor due to fatal error: %s' % (f.value,))

    fatal_error(stop_reactor)


def df_sleep(timeout, res=None):
    return task.deferLater(reactor, timeout, lambda: res)

