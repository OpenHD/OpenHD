#!/usr/bin/env python

'''
fit estimate of PID oscillations
'''
from __future__ import print_function

import numpy
import pylab

from argparse import ArgumentParser
parser = ArgumentParser(description=__doc__)
parser.add_argument("--condition", default=None, help="select packets by condition")
parser.add_argument("--sample-rate", dest='sample_rate', type=int, default=400, help="sample rate of PID values")
parser.add_argument("logs", metavar="LOG", nargs="+")

args = parser.parse_args()

from pymavlink import mavutil

def fft(logfile):
    '''display fft for PID data in logfile'''

    print("Processing log %s" % filename)
    mlog = mavutil.mavlink_connection(filename)
    sample_rate = args.sample_rate

    data = {'PIDR.rate' : 400,
            'PIDP.rate' : 400,
            'PIDY.rate' : 400, }

    for gyr in ['PIDR','PIDP', 'PIDY']:
        for ax in ['P', 'I', 'D']:
            data[gyr+'.'+ax] = []

    # now gather all the data
    while True:
        m = mlog.recv_match(condition=args.condition)
        if m is None:
            break
        type = m.get_type()
        if type == "PIDR":
            data[type+'.P'].append(m.P)
            data[type+'.I'].append(m.I)
            data[type+'.D'].append(m.D)
        elif type == "PIDP":
            data[type+'.P'].append(m.P)
            data[type+'.I'].append(m.I)
            data[type+'.D'].append(m.D)
        elif type == "PIDY":
            data[type+'.P'].append(m.P)
            data[type+'.I'].append(m.I)
            data[type+'.D'].append(m.D)

    print("Extracted %u data points, sample rate %uHz" % (len(data['PIDR.P']), sample_rate))

    fs = int(sample_rate)
    window = numpy.hanning(fs)
    S2 = numpy.inner(window, window)

    for msg in ['PIDR', 'PIDP', 'PIDY']:
        pylab.figure()

        for axis in ['P', 'I', 'D']:
            field = msg + '.' + axis
            d = data[field]
            counts = len(d) // fs
            if counts == 0:
                continue
            sum_fft = numpy.zeros(fs//2+1)
            for i in range(0, counts):
                dx = d[i*fs:(i+1)*fs]
                dx = numpy.array(dx)
                dx *= window
                if len(dx) == 0:
                    continue
                d_fft = numpy.fft.rfft(dx)
                d_fft = numpy.square(abs(d_fft))
                d_fft[0] = 0
                d_fft[-1] = 0
                sum_fft += d_fft
                freq  = numpy.fft.rfftfreq(fs, 1.0 / fs)
            # compute power spectral density
            psd = numpy.sqrt((2 * sum_fft / counts) / (fs * S2))
            pylab.plot(freq, psd, label=field)
        pylab.legend(loc='upper right')

for filename in args.logs:
    fft(filename)

pylab.show()
