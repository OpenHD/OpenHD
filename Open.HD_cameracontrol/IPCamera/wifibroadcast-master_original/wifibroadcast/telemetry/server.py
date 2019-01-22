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

import sys
import curses
import curses.textpad
import time
from twisted.python import log
from twisted.internet import reactor, defer
from twisted.internet.protocol import ProcessProtocol
from twisted.protocols.basic import LineReceiver
from twisted.internet.error import ReactorNotRunning

__system_failed = False
_DEBUG = False


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
    global _DEBUG

    if isinstance(f, defer.FirstError):
        f = f.value.subFailure

    if _DEBUG:
        log.err(f, 'Stopping reactor due to fatal error')
    else:
        log.msg('Stopping reactor due to fatal error: %s' % (f.value,))

    fatal_error(stop_reactor)


class BadTelemetry(Exception):
    pass


class AntennaProtocol(LineReceiver):
    delimiter = '\n'

    def __init__(self, window, rx_id):
        self.window = window
        self.rx_id = rx_id
        self.ant = {}
        self.count_all = None

    def lineReceived(self, line):
        cols = line.strip().split('\t')
        try:
            if len(cols) < 2:
                raise BadTelemetry()

            #ts = int(cols[0])
            cmd = cols[1]

            if cmd == 'ANT':
                if len(cols) != 4:
                    raise BadTelemetry()
                self.ant[cols[2]] = tuple(int(i) for i in cols[3].split(':'))
            elif cmd == 'PKT':
                if len(cols) != 3:
                    raise BadTelemetry()

                self.window.clear()
                p_all, p_dec_err, p_dec_ok, p_fec_rec, p_lost, p_bad = map(int, cols[2].split(':'))

                if not self.count_all:
                    self.count_all = (p_all, p_dec_err, p_dec_ok, p_fec_rec, p_lost, p_bad)
                else:
                    self.count_all = tuple((a + b) for a, b in zip((p_all, p_dec_err, p_dec_ok, p_fec_rec, p_lost, p_bad), self.count_all))

                self.window.addstr(0, 0, 'PKT: %d recv, %d d_err, %d d_ok, %d fec_r, %d lost, %d bad\n' % self.count_all)
                self.window.addstr(1, 0, 'PKT/s: %d recv, %d d_err, %d d_ok, %d fec_r, %d lost, %d bad\n' % (p_all, p_dec_err, p_dec_ok, p_fec_rec, p_lost, p_bad))

                for i, (k, v) in enumerate(sorted(self.ant.iteritems())):
                    pkt_s, rssi_min, rssi_avg, rssi_max = v
                    self.window.addstr(i + 3, 0, '%s: %d pkt/s, rssi %d < %d < %d\n' % (k, pkt_s, rssi_min, rssi_avg, rssi_max))

                self.ant.clear()
            else:
                raise BadTelemetry()

        except BadTelemetry:
            self.window.add_str('Bad telemetry [%s]: %s' % (self.rx_id, line))
            return
        finally:
            self.window.refresh()


class DbgProtocol(LineReceiver):
    delimiter = '\n'

    def __init__(self, window, rx_id):
        self.window = window
        self.rx_id = rx_id

    def lineReceived(self, line):
        self.window.addstr('%s [%s] %s\n' % (time.strftime('%H:%M:%S'), self.rx_id, line))
        self.window.refresh()


class RXProtocol(ProcessProtocol):
    def __init__(self, status_win, log_win, cmd, rx_id):
        self.status_win = status_win
        self.log_win = log_win
        self.cmd = cmd
        self.rx_id = rx_id
        self.ant = AntennaProtocol(self.status_win, rx_id)
        self.dbg = DbgProtocol(self.log_win, rx_id)
        self.df = defer.Deferred()

    def connectionMade(self):
        self.log_win.addstr('Started RX %s\n' % (self.rx_id,))
        self.log_win.refresh()

    def outReceived(self, data):
        self.ant.dataReceived(data)

    def errReceived(self, data):
        self.dbg.dataReceived(data)

    def processEnded(self, status):
        rc = status.value.exitCode
        self.log_win.addstr('Stopped RX %s with code %s\n' % (self.rx_id, rc))
        self.log_win.refresh()

        if rc == 0:
            self.df.callback(str(status.value))
        else:
            self.df.errback(status)

    def start(self):
        df = defer.maybeDeferred(reactor.spawnProcess, self, self.cmd[0], self.cmd, env=None, childFDs={0: "w", 1: "r", 2: "r"})
        return df.addCallback(lambda _: self.df)


def init(stdscr):
    height, width = stdscr.getmaxyx()
    w1h = min(height / 2, 10)
    w1w = width / 2
    w2h = w1h
    w2w = width - w1w
    w3h = height - w1h - 1
    w3w = width
    status_win1 = stdscr.subpad(w1h - 2, w1w - 2, 1, 1)
    status_win2 = stdscr.subpad(w2h - 2, w2w - 2, 1, w1w + 1)
    log_win = stdscr.subpad(w3h - 2, w3w - 2, w1h + 1, 1)

    curses.textpad.rectangle(stdscr, 0, 0, w1h - 1, w1w - 1)
    curses.textpad.rectangle(stdscr, 0, w1w, w2h - 1, w1w + w2w - 1)
    curses.textpad.rectangle(stdscr, w1h, 0, w1h + w3h - 1, w3w - 1)
    stdscr.addstr(0, 3, '[video]')
    stdscr.addstr(0, w1w + 3, '[telem]')
    stdscr.addstr(w1h, 3, '[debug logs]')
    stdscr.refresh()

    for i in (status_win1, status_win2, log_win):
        i.idlok(1)
        i.scrollok(1)

    cmd1 = sys.argv[1].split() # ["./rx", "-a", "5601", "-u", "5600"]
    cmd2 = sys.argv[2].split() # ["./rx", "-a", "14551", "-u", "14550"]

    df1 = RXProtocol(status_win1, log_win, cmd1, 'video').start()
    df2 = RXProtocol(status_win2, log_win, cmd2, 'telem').start()
    return defer.gatherResults([df1, df2], consumeErrors=True)


def main():
    log.startLogging(open('server.log', 'a'))
    #log.startLogging(sys.stdout)

    stdscr = curses.initscr()
    curses.noecho()
    curses.cbreak()
    curses.curs_set(0)
    stdscr.keypad(1)

    reactor.callWhenRunning(lambda: defer.maybeDeferred(init, stdscr)\
                            .addErrback(abort_on_crash))
    reactor.run()
    curses.endwin()
    rc = exit_status()
    log.msg('Exiting with code %d' % rc)
    sys.exit(rc)


if __name__ == '__main__':
    main()
