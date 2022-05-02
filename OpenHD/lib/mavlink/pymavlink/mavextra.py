#!/usr/bin/env python
'''
useful extra functions for use by mavlink clients

Copyright Andrew Tridgell 2011
Released under GNU GPL version 3 or later
'''
from __future__ import print_function
from __future__ import absolute_import
from builtins import object

from math import *

try:
    # in case numpy isn't installed
    from .quaternion import Quaternion
except:
    pass

try:
    # rotmat doesn't work on Python3.2 yet
    from .rotmat import Vector3, Matrix3
except Exception:
    pass


def kmh(mps):
    '''convert m/s to Km/h'''
    return mps*3.6

def altitude(SCALED_PRESSURE, ground_pressure=None, ground_temp=None):
    '''calculate barometric altitude'''
    from . import mavutil
    self = mavutil.mavfile_global
    if ground_pressure is None:
        if self.param('GND_ABS_PRESS', None) is None:
            return 0
        ground_pressure = self.param('GND_ABS_PRESS', 1)
    if ground_temp is None:
        ground_temp = self.param('GND_TEMP', 0)
    scaling = ground_pressure / (SCALED_PRESSURE.press_abs*100.0)
    temp = ground_temp + 273.15
    return log(scaling) * temp * 29271.267 * 0.001

def altitude2(SCALED_PRESSURE, ground_pressure=None, ground_temp=None):
    '''calculate barometric altitude'''
    from . import mavutil
    self = mavutil.mavfile_global
    if ground_pressure is None:
        if self.param('GND_ABS_PRESS', None) is None:
            return 0
        ground_pressure = self.param('GND_ABS_PRESS', 1)
    if ground_temp is None:
        ground_temp = self.param('GND_TEMP', 0)
    scaling = SCALED_PRESSURE.press_abs*100.0 / ground_pressure
    temp = ground_temp + 273.15
    return 153.8462 * temp * (1.0 - exp(0.190259 * log(scaling)))

def mag_heading(RAW_IMU, ATTITUDE, declination=None, SENSOR_OFFSETS=None, ofs=None):
    '''calculate heading from raw magnetometer'''
    if declination is None:
        from . import mavutil
        declination = degrees(mavutil.mavfile_global.param('COMPASS_DEC', 0))
    mag_x = RAW_IMU.xmag
    mag_y = RAW_IMU.ymag
    mag_z = RAW_IMU.zmag
    if SENSOR_OFFSETS is not None and ofs is not None:
        mag_x += ofs[0] - SENSOR_OFFSETS.mag_ofs_x
        mag_y += ofs[1] - SENSOR_OFFSETS.mag_ofs_y
        mag_z += ofs[2] - SENSOR_OFFSETS.mag_ofs_z

    # go via a DCM matrix to match the APM calculation
    dcm_matrix = rotation(ATTITUDE)
    cos_pitch_sq = 1.0-(dcm_matrix.c.x*dcm_matrix.c.x)
    headY = mag_y * dcm_matrix.c.z - mag_z * dcm_matrix.c.y
    headX = mag_x * cos_pitch_sq - dcm_matrix.c.x * (mag_y * dcm_matrix.c.y + mag_z * dcm_matrix.c.z)

    heading = degrees(atan2(-headY,headX)) + declination
    if heading < 0:
        heading += 360
    return heading

def mag_field_df(MAG, ofs=None, diagonals=(1.0,1.0,1.0), offdiagonals=(0.0,0.0,0.0)):
    '''calculate magnetic field strength from raw magnetometer for DF '''
    mag = Vector3(MAG.MagX, MAG.MagY, MAG.MagZ)
    if ofs is not None:
        mag += Vector3(ofs[0],ofs[1],ofs[2]) - Vector3(MAG.OfsX, MAG.OfsY, MAG.OfsZ)
        diagonals = Vector3(diagonals[0], diagonals[1], diagonals[2])
        offdiagonals = Vector3(offdiagonals[0], offdiagonals[1], offdiagonals[2])
        rot = Matrix3(Vector3(diagonals.x,    offdiagonals.x, offdiagonals.y),
                      Vector3(offdiagonals.x, diagonals.y,    offdiagonals.z),
                      Vector3(offdiagonals.y, offdiagonals.z, diagonals.z))
        mag = rot * mag
    return mag.length()

def mag_heading_df(MAG, ATT, declination=None, ofs=None, diagonals=(1.0,1.0,1.0), offdiagonals=(0.0,0.0,0.0)):
    '''calculate heading from raw magnetometer'''
    if declination is None:
        from pymavlink import mavutil
        declination = degrees(mavutil.mavfile_global.param('COMPASS_DEC', 0))
    mag = Vector3(MAG.MagX,MAG.MagY,MAG.MagZ)
    if ofs is not None:
        mag += Vector3(ofs[0],ofs[1],ofs[2]) - Vector3(MAG.OfsX, MAG.OfsY, MAG.OfsZ)
        diagonals = Vector3(diagonals[0], diagonals[1], diagonals[2])
        offdiagonals = Vector3(offdiagonals[0], offdiagonals[1], offdiagonals[2])
        rot = Matrix3(Vector3(diagonals.x,    offdiagonals.x, offdiagonals.y),
                      Vector3(offdiagonals.x, diagonals.y,    offdiagonals.z),
                      Vector3(offdiagonals.y, offdiagonals.z, diagonals.z))
        mag = rot * mag

    # go via a DCM matrix to match the APM calculation
    dcm_matrix = rotation_df(ATT)
    cos_pitch_sq = 1.0-(dcm_matrix.c.x*dcm_matrix.c.x)
    headY = mag.y * dcm_matrix.c.z - mag.z * dcm_matrix.c.y
    headX = mag.x * cos_pitch_sq - dcm_matrix.c.x * (mag.y * dcm_matrix.c.y + mag.z * dcm_matrix.c.z)

    heading = degrees(atan2(-headY,headX)) + declination
    if heading < 0:
        heading += 360
    return heading

def gps_time_to_epoch(week, msec):
    '''convert GPS week and TOW to a time in seconds since 1970'''
    epoch = 86400*(10*365 + int((1980-1969)/4) + 1 + 6 - 2)
    return epoch + 86400*7*week + msec*0.001 - 18


def mag_heading_motors(RAW_IMU, ATTITUDE, declination, SENSOR_OFFSETS, ofs, SERVO_OUTPUT_RAW, motor_ofs):
    '''calculate heading from raw magnetometer'''
    ofs = get_motor_offsets(SERVO_OUTPUT_RAW, ofs, motor_ofs)

    if declination is None:
        from . import mavutil
        declination = degrees(mavutil.mavfile_global.param('COMPASS_DEC', 0))
    mag_x = RAW_IMU.xmag
    mag_y = RAW_IMU.ymag
    mag_z = RAW_IMU.zmag
    if SENSOR_OFFSETS is not None and ofs is not None:
        mag_x += ofs[0] - SENSOR_OFFSETS.mag_ofs_x
        mag_y += ofs[1] - SENSOR_OFFSETS.mag_ofs_y
        mag_z += ofs[2] - SENSOR_OFFSETS.mag_ofs_z

    headX = mag_x*cos(ATTITUDE.pitch) + mag_y*sin(ATTITUDE.roll)*sin(ATTITUDE.pitch) + mag_z*cos(ATTITUDE.roll)*sin(ATTITUDE.pitch)
    headY = mag_y*cos(ATTITUDE.roll) - mag_z*sin(ATTITUDE.roll)
    heading = degrees(atan2(-headY,headX)) + declination
    if heading < 0:
        heading += 360
    return heading

def mag_field(RAW_IMU, SENSOR_OFFSETS=None, ofs=None):
    '''calculate magnetic field strength from raw magnetometer'''
    mag_x = RAW_IMU.xmag
    mag_y = RAW_IMU.ymag
    mag_z = RAW_IMU.zmag
    if SENSOR_OFFSETS is not None and ofs is not None:
        mag_x += ofs[0] - SENSOR_OFFSETS.mag_ofs_x
        mag_y += ofs[1] - SENSOR_OFFSETS.mag_ofs_y
        mag_z += ofs[2] - SENSOR_OFFSETS.mag_ofs_z
    return sqrt(mag_x**2 + mag_y**2 + mag_z**2)

def mag_field_df(MAG, ofs=None):
    '''calculate magnetic field strength from raw magnetometer (dataflash version)'''
    mag = Vector3(MAG.MagX, MAG.MagY, MAG.MagZ)
    offsets = Vector3(MAG.OfsX, MAG.OfsY, MAG.OfsZ)
    if ofs is not None:
        mag = (mag - offsets) + Vector3(ofs[0], ofs[1], ofs[2])
    return mag.length()

def get_motor_offsets(SERVO_OUTPUT_RAW, ofs, motor_ofs):
    '''calculate magnetic field strength from raw magnetometer'''
    from . import mavutil
    self = mavutil.mavfile_global

    m = SERVO_OUTPUT_RAW
    motor_pwm = m.servo1_raw + m.servo2_raw + m.servo3_raw + m.servo4_raw
    motor_pwm *= 0.25
    rc3_min = self.param('RC3_MIN', 1100)
    rc3_max = self.param('RC3_MAX', 1900)
    motor = (motor_pwm - rc3_min) / (rc3_max - rc3_min)
    if motor > 1.0:
        motor = 1.0
    if motor < 0.0:
        motor = 0.0

    motor_offsets0 = motor_ofs[0] * motor
    motor_offsets1 = motor_ofs[1] * motor
    motor_offsets2 = motor_ofs[2] * motor
    ofs = (ofs[0] + motor_offsets0, ofs[1] + motor_offsets1, ofs[2] + motor_offsets2)

    return ofs

def mag_field_motors(RAW_IMU, SENSOR_OFFSETS, ofs, SERVO_OUTPUT_RAW, motor_ofs):
    '''calculate magnetic field strength from raw magnetometer'''
    mag_x = RAW_IMU.xmag
    mag_y = RAW_IMU.ymag
    mag_z = RAW_IMU.zmag

    ofs = get_motor_offsets(SERVO_OUTPUT_RAW, ofs, motor_ofs)

    if SENSOR_OFFSETS is not None and ofs is not None:
        mag_x += ofs[0] - SENSOR_OFFSETS.mag_ofs_x
        mag_y += ofs[1] - SENSOR_OFFSETS.mag_ofs_y
        mag_z += ofs[2] - SENSOR_OFFSETS.mag_ofs_z
    return sqrt(mag_x**2 + mag_y**2 + mag_z**2)

def angle_diff(angle1, angle2):
    '''show the difference between two angles in degrees'''
    ret = angle1 - angle2
    if ret > 180:
        ret -= 360;
    if ret < -180:
        ret += 360
    return ret

average_data = {}

def average(var, key, N):
    '''average over N points'''
    global average_data
    if not key in average_data:
        average_data[key] = [var]*N
        return var
    average_data[key].pop(0)
    average_data[key].append(var)
    return sum(average_data[key])/N

derivative_data = {}

def second_derivative_5(var, key):
    '''5 point 2nd derivative'''
    global derivative_data
    from . import mavutil
    tnow = mavutil.mavfile_global.timestamp

    if not key in derivative_data:
        derivative_data[key] = (tnow, [var]*5)
        return 0
    (last_time, data) = derivative_data[key]
    data.pop(0)
    data.append(var)
    derivative_data[key] = (tnow, data)
    h = (tnow - last_time)
    # N=5 2nd derivative from
    # http://www.holoborodko.com/pavel/numerical-methods/numerical-derivative/smooth-low-noise-differentiators/
    ret = ((data[4] + data[0]) - 2*data[2]) / (4*h**2)
    return ret

def second_derivative_9(var, key):
    '''9 point 2nd derivative'''
    global derivative_data
    from . import mavutil
    tnow = mavutil.mavfile_global.timestamp

    if not key in derivative_data:
        derivative_data[key] = (tnow, [var]*9)
        return 0
    (last_time, data) = derivative_data[key]
    data.pop(0)
    data.append(var)
    derivative_data[key] = (tnow, data)
    h = (tnow - last_time)
    # N=5 2nd derivative from
    # http://www.holoborodko.com/pavel/numerical-methods/numerical-derivative/smooth-low-noise-differentiators/
    f = data
    ret = ((f[8] + f[0]) + 4*(f[7] + f[1]) + 4*(f[6]+f[2]) - 4*(f[5]+f[3]) - 10*f[4])/(64*h**2)
    return ret

lowpass_data = {}

def lowpass(var, key, factor):
    '''a simple lowpass filter'''
    global lowpass_data
    if not key in lowpass_data:
        lowpass_data[key] = var
    else:
        lowpass_data[key] = factor*lowpass_data[key] + (1.0 - factor)*var
    return lowpass_data[key]

def lpalpha(sample_rate_hz, cutoff_hz):
    '''find alpha for low pass filter'''
    rc = 1.0 / (2*pi*cutoff_hz)
    dt = 1.0 / sample_rate_hz
    return 1.0 - dt/(dt+rc)

lowpass_hz_data = {}

def lowpassHz(var, key, sample_rate_hz, cutoff_hz):
    '''a simple lowpass filter with specified frequency'''
    global lowpass_hz_data
    alpha = lpalpha(sample_rate_hz, cutoff_hz)
    if not key in lowpass_hz_data:
        lowpass_hz_data[key] = var
    else:
        lowpass_hz_data[key] = alpha*lowpass_hz_data[key] + (1.0-alpha)*var
    return lowpass_hz_data[key]

last_diff = {}

def diff(var, key):
    '''calculate differences between values'''
    global last_diff
    ret = 0
    if not key in last_diff:
        last_diff[key] = var
        return 0
    ret = var - last_diff[key]
    last_diff[key] = var
    return ret

last_delta = {}

def delta(var, key, tusec=None):
    '''calculate slope'''
    global last_delta
    if tusec is not None:
        tnow = tusec * 1.0e-6
    else:
        from . import mavutil
        tnow = mavutil.mavfile_global.timestamp
    ret = 0
    if key in last_delta:
        (last_v, last_t, last_ret) = last_delta[key]
        if last_t == tnow:
            return last_ret
        if tnow == last_t:
            ret = 0
        else:
            ret = (var - last_v) / (tnow - last_t)
    last_delta[key] = (var, tnow, ret)
    return ret

last_sum = {}

def sum(var, key):
    '''sum variable'''
    global last_sum
    ret = 0
    if not key in last_sum:
        last_sum[key] = 0
    last_sum[key] += var
    return last_sum[key]

last_integral = {}

def integral(var, key, timeus):
    '''integrate variable'''
    global last_integral
    ret = 0
    if not key in last_integral:
        last_integral[key] = (0,timeus)
    (lastsum,lastt) = last_integral[key]
    dt = (timeus - lastt) * 1.0e-6
    dv = var * dt
    newv = lastsum + dv
    last_integral[key] = (newv,timeus)
    return newv


def delta_angle(var, key, tusec=None):
    '''calculate slope of an angle'''
    global last_delta
    if tusec is not None:
        tnow = tusec * 1.0e-6
    else:
        from . import mavutil
        tnow = mavutil.mavfile_global.timestamp
    dv = 0
    ret = 0
    if key in last_delta:
        (last_v, last_t, last_ret) = last_delta[key]
        if last_t == tnow:
            return last_ret
        if tnow == last_t:
            ret = 0
        else:
            dv = var - last_v
            if dv > 180:
                dv -= 360
            if dv < -180:
                dv += 360
            ret = dv / (tnow - last_t)
    last_delta[key] = (var, tnow, ret)
    return ret

def roll_estimate(RAW_IMU,GPS_RAW_INT=None,ATTITUDE=None,SENSOR_OFFSETS=None, ofs=None, mul=None,smooth=0.7):
    '''estimate roll from accelerometer'''
    rx = RAW_IMU.xacc * 9.81 / 1000.0
    ry = RAW_IMU.yacc * 9.81 / 1000.0
    rz = RAW_IMU.zacc * 9.81 / 1000.0
    if ATTITUDE is not None and GPS_RAW_INT is not None:
        ry -= ATTITUDE.yawspeed * GPS_RAW_INT.vel*0.01
        rz += ATTITUDE.pitchspeed * GPS_RAW_INT.vel*0.01
    if SENSOR_OFFSETS is not None and ofs is not None:
        rx += SENSOR_OFFSETS.accel_cal_x
        ry += SENSOR_OFFSETS.accel_cal_y
        rz += SENSOR_OFFSETS.accel_cal_z
        rx -= ofs[0]
        ry -= ofs[1]
        rz -= ofs[2]
        if mul is not None:
            rx *= mul[0]
            ry *= mul[1]
            rz *= mul[2]
    return lowpass(degrees(-asin(ry/sqrt(rx**2+ry**2+rz**2))),'_roll',smooth)

def pitch_estimate(RAW_IMU, GPS_RAW_INT=None,ATTITUDE=None, SENSOR_OFFSETS=None, ofs=None, mul=None, smooth=0.7):
    '''estimate pitch from accelerometer'''
    rx = RAW_IMU.xacc * 9.81 / 1000.0
    ry = RAW_IMU.yacc * 9.81 / 1000.0
    rz = RAW_IMU.zacc * 9.81 / 1000.0
    if ATTITUDE is not None and GPS_RAW_INT is not None:
        ry -= ATTITUDE.yawspeed * GPS_RAW_INT.vel*0.01
        rz += ATTITUDE.pitchspeed * GPS_RAW_INT.vel*0.01
    if SENSOR_OFFSETS is not None and ofs is not None:
        rx += SENSOR_OFFSETS.accel_cal_x
        ry += SENSOR_OFFSETS.accel_cal_y
        rz += SENSOR_OFFSETS.accel_cal_z
        rx -= ofs[0]
        ry -= ofs[1]
        rz -= ofs[2]
        if mul is not None:
            rx *= mul[0]
            ry *= mul[1]
            rz *= mul[2]
    return lowpass(degrees(asin(rx/sqrt(rx**2+ry**2+rz**2))),'_pitch',smooth)

def rotation(ATTITUDE):
    '''return the current DCM rotation matrix'''
    r = Matrix3()
    r.from_euler(ATTITUDE.roll, ATTITUDE.pitch, ATTITUDE.yaw)
    return r

def mag_rotation(RAW_IMU, inclination, declination):
    '''return an attitude rotation matrix that is consistent with the current mag
       vector'''
    m_body = Vector3(RAW_IMU.xmag, RAW_IMU.ymag, RAW_IMU.zmag)
    m_earth = Vector3(m_body.length(), 0, 0)

    r = Matrix3()
    r.from_euler(0, -radians(inclination), radians(declination))
    m_earth = r * m_earth

    r.from_two_vectors(m_earth, m_body)
    return r

def mag_pitch(RAW_IMU, inclination, declination):
    '''estimate pithc from mag'''
    m = mag_rotation(RAW_IMU, inclination, declination)
    (r, p, y) = m.to_euler()
    return degrees(p)

def mag_roll(RAW_IMU, inclination, declination):
    '''estimate roll from mag'''
    m = mag_rotation(RAW_IMU, inclination, declination)
    (r, p, y) = m.to_euler()
    return degrees(r)

def gravity(RAW_IMU, SENSOR_OFFSETS=None, ofs=None, mul=None, smooth=0.7):
    '''estimate pitch from accelerometer'''
    if hasattr(RAW_IMU, 'xacc'):
        rx = RAW_IMU.xacc * 9.81 / 1000.0
        ry = RAW_IMU.yacc * 9.81 / 1000.0
        rz = RAW_IMU.zacc * 9.81 / 1000.0
    else:
        rx = RAW_IMU.AccX
        ry = RAW_IMU.AccY
        rz = RAW_IMU.AccZ
    if SENSOR_OFFSETS is not None and ofs is not None:
        rx += SENSOR_OFFSETS.accel_cal_x
        ry += SENSOR_OFFSETS.accel_cal_y
        rz += SENSOR_OFFSETS.accel_cal_z
        rx -= ofs[0]
        ry -= ofs[1]
        rz -= ofs[2]
        if mul is not None:
            rx *= mul[0]
            ry *= mul[1]
            rz *= mul[2]
    return sqrt(rx**2+ry**2+rz**2)



def pitch_sim(SIMSTATE, GPS_RAW):
    '''estimate pitch from SIMSTATE accels'''
    xacc = SIMSTATE.xacc - lowpass(delta(GPS_RAW.v,"v")*6.6, "v", 0.9)
    zacc = SIMSTATE.zacc
    zacc += SIMSTATE.ygyro * GPS_RAW.v;
    if xacc/zacc >= 1:
        return 0
    if xacc/zacc <= -1:
        return -0
    return degrees(-asin(xacc/zacc))

ORGN = None

def get_origin():
  global ORGN
  if ORGN is not None:
      return ORGN
  from . import mavutil
  self = mavutil.mavfile_global
  ret = self.messages.get('ORGN', None)
  if ret is None:
      ret = self.messages.get('GPS', None)
      if ret.Status < 3:
          return None
  ORGN = ret
  return ret

# graph distance_two(GPS,XKF1[0])

def get_lat_lon_alt(MSG):
    '''gets lat and lon in radians and alt in meters from a position msg'''
    if hasattr(MSG, 'Lat'):
        lat = radians(MSG.Lat)
        lon = radians(MSG.Lng)
        alt = MSG.Alt
    elif hasattr(MSG, 'cog'):
        lat = radians(MSG.lat)*1.0e-7
        lon = radians(MSG.lon)*1.0e-7
        alt = MSG.alt*0.001
    elif hasattr(MSG,'lat'):
        lat = radians(MSG.lat)
        lon = radians(MSG.lon)
        alt = MSG.alt*0.001
    elif hasattr(MSG, 'PN'):
        # origin relative position from EKF
        global ORGN
        if ORGN is None:
            ORGN = get_origin()
        if ORGN is None:
            return None
        (lat,lon) = gps_offset(ORGN.Lat, ORGN.Lng, MSG.PN, MSG.PE)
        lat = radians(lat)
        lon = radians(lon)
        alt = ORGN.Alt - MSG.PD
    else:
        return None
    return (lat, lon, alt)

def _distance_two(MSG1, MSG2, horizontal=True):
    '''distance between two points'''
    (lat1, lon1, alt1) = get_lat_lon_alt(MSG1)
    (lat2, lon2, alt2) = get_lat_lon_alt(MSG2)
    dLat = lat2 - lat1
    dLon = lon2 - lon1

    a = sin(0.5*dLat)**2 + sin(0.5*dLon)**2 * cos(lat1) * cos(lat2)
    c = 2.0 * atan2(sqrt(a), sqrt(1.0-a))
    ground_dist = 6371 * 1000 * c
    if horizontal:
        return ground_dist
    return sqrt(ground_dist**2 + (alt2-alt1)**2)

def distance_two(MSG1, MSG2, horizontal=True):
    '''distance between two points'''
    try:
        return _distance_two(MSG1, MSG2)
    except Exception as ex:
        print(ex)
        return None

first_fix = None

def distance_home(GPS_RAW):
    '''distance from first fix point'''
    global first_fix
    if (hasattr(GPS_RAW, 'fix_type') and GPS_RAW.fix_type < 2) or \
       (hasattr(GPS_RAW, 'Status')   and GPS_RAW.Status   < 2):
        return 0

    if first_fix is None:
        first_fix = GPS_RAW
        return 0
    return distance_two(GPS_RAW, first_fix)

def sawtooth(ATTITUDE, amplitude=2.0, period=5.0):
    '''sawtooth pattern based on uptime'''
    mins = (ATTITUDE.usec * 1.0e-6)/60
    p = fmod(mins, period*2)
    if p < period:
        return amplitude * (p/period)
    return amplitude * (period - (p-period))/period

def rate_of_turn(speed, bank):
    '''return expected rate of turn in degrees/s for given speed in m/s and
       bank angle in degrees'''
    if abs(speed) < 2 or abs(bank) > 80:
        return 0
    ret = degrees(9.81*tan(radians(bank))/speed)
    return ret

def wingloading(bank):
    '''return expected wing loading factor for a bank angle in radians'''
    return 1.0/cos(bank)

def airspeed(VFR_HUD, ratio=None, used_ratio=None, offset=None):
    '''recompute airspeed with a different ARSPD_RATIO'''
    from . import mavutil
    mav = mavutil.mavfile_global
    if ratio is None:
        ratio = 1.9936 # APM default
    if used_ratio is None:
        if 'ARSPD_RATIO' in mav.params:
            used_ratio = mav.params['ARSPD_RATIO']
        else:
            print("no ARSPD_RATIO in mav.params")
            used_ratio = ratio
    if hasattr(VFR_HUD,'airspeed'):
        airspeed = VFR_HUD.airspeed
    else:
        airspeed = VFR_HUD.Airspeed
    airspeed_pressure = (airspeed**2) / used_ratio
    if offset is not None:
        airspeed_pressure += offset
        if airspeed_pressure < 0:
            airspeed_pressure = 0
    airspeed = sqrt(airspeed_pressure * ratio)
    return airspeed

def EAS2TAS(ARSP,GPS,BARO,ground_temp=25):
    '''EAS2TAS from ARSP.Temp'''
    tempK = ground_temp + 273.15 - 0.0065 * GPS.Alt
    return sqrt(1.225 / (BARO.Press / (287.26 * tempK)))


def airspeed_ratio(VFR_HUD):
    '''recompute airspeed with a different ARSPD_RATIO'''
    from . import mavutil
    mav = mavutil.mavfile_global
    airspeed_pressure = (VFR_HUD.airspeed**2) / ratio
    airspeed = sqrt(airspeed_pressure * ratio)
    return airspeed

def airspeed_voltage(VFR_HUD, ratio=None):
    '''back-calculate the voltage the airspeed sensor must have seen'''
    from . import mavutil
    mav = mavutil.mavfile_global
    if ratio is None:
        ratio = 1.9936 # APM default
    if 'ARSPD_RATIO' in mav.params:
        used_ratio = mav.params['ARSPD_RATIO']
    else:
        used_ratio = ratio
    if 'ARSPD_OFFSET' in mav.params:
        offset = mav.params['ARSPD_OFFSET']
    else:
        return -1
    airspeed_pressure = (pow(VFR_HUD.airspeed,2)) / used_ratio
    raw = airspeed_pressure + offset
    SCALING_OLD_CALIBRATION = 204.8
    voltage = 5.0 * raw / 4096
    return voltage


def earth_rates(ATTITUDE):
    '''return angular velocities in earth frame'''
    from math import sin, cos, tan, fabs

    p     = ATTITUDE.rollspeed
    q     = ATTITUDE.pitchspeed
    r     = ATTITUDE.yawspeed
    phi   = ATTITUDE.roll
    theta = ATTITUDE.pitch
    psi   = ATTITUDE.yaw

    phiDot   = p + tan(theta)*(q*sin(phi) + r*cos(phi))
    thetaDot = q*cos(phi) - r*sin(phi)
    if fabs(cos(theta)) < 1.0e-20:
        theta += 1.0e-10
    psiDot   = (q*sin(phi) + r*cos(phi))/cos(theta)
    return (phiDot, thetaDot, psiDot)

def roll_rate(ATTITUDE):
    '''return roll rate in earth frame'''
    (phiDot, thetaDot, psiDot) = earth_rates(ATTITUDE)
    return phiDot

def pitch_rate(ATTITUDE):
    '''return pitch rate in earth frame'''
    (phiDot, thetaDot, psiDot) = earth_rates(ATTITUDE)
    return thetaDot

def yaw_rate(ATTITUDE):
    '''return yaw rate in earth frame'''
    (phiDot, thetaDot, psiDot) = earth_rates(ATTITUDE)
    return psiDot


def gps_velocity(GLOBAL_POSITION_INT):
    '''return GPS velocity vector'''
    return Vector3(GLOBAL_POSITION_INT.vx, GLOBAL_POSITION_INT.vy, GLOBAL_POSITION_INT.vz) * 0.01


def gps_velocity_old(GPS_RAW_INT):
    '''return GPS velocity vector'''
    return Vector3(GPS_RAW_INT.vel*0.01*cos(radians(GPS_RAW_INT.cog*0.01)),
                   GPS_RAW_INT.vel*0.01*sin(radians(GPS_RAW_INT.cog*0.01)), 0)

def gps_velocity_body(GPS_RAW_INT, ATTITUDE):
    '''return GPS velocity vector in body frame'''
    r = rotation(ATTITUDE)
    return r.transposed() * Vector3(GPS_RAW_INT.vel*0.01*cos(radians(GPS_RAW_INT.cog*0.01)),
                                    GPS_RAW_INT.vel*0.01*sin(radians(GPS_RAW_INT.cog*0.01)),
                                    -tan(ATTITUDE.pitch)*GPS_RAW_INT.vel*0.01)

def earth_accel(RAW_IMU,ATTITUDE):
    '''return earth frame acceleration vector'''
    r = rotation(ATTITUDE)
    accel = Vector3(RAW_IMU.xacc, RAW_IMU.yacc, RAW_IMU.zacc) * 9.81 * 0.001
    return r * accel

def earth_gyro(RAW_IMU,ATTITUDE):
    '''return earth frame gyro vector'''
    r = rotation(ATTITUDE)
    accel = Vector3(degrees(RAW_IMU.xgyro), degrees(RAW_IMU.ygyro), degrees(RAW_IMU.zgyro)) * 0.001
    return r * accel

def airspeed_energy_error(NAV_CONTROLLER_OUTPUT, VFR_HUD):
    '''return airspeed energy error matching APM internals
    This is positive when we are going too slow
    '''
    aspeed_cm = VFR_HUD.airspeed*100
    target_airspeed = NAV_CONTROLLER_OUTPUT.aspd_error + aspeed_cm
    airspeed_energy_error = ((target_airspeed*target_airspeed) - (aspeed_cm*aspeed_cm))*0.00005
    return airspeed_energy_error


def energy_error(NAV_CONTROLLER_OUTPUT, VFR_HUD):
    '''return energy error matching APM internals
    This is positive when we are too low or going too slow
    '''
    aspeed_energy_error = airspeed_energy_error(NAV_CONTROLLER_OUTPUT, VFR_HUD)
    alt_error = NAV_CONTROLLER_OUTPUT.alt_error*100
    energy_error = aspeed_energy_error + alt_error*0.098
    return energy_error

def rover_turn_circle(SERVO_OUTPUT_RAW):
    '''return turning circle (diameter) in meters for steering_angle in degrees
    '''

    # this matches Toms slash
    max_wheel_turn = 35
    wheelbase      = 0.335
    wheeltrack     = 0.296

    steering_angle = max_wheel_turn * (SERVO_OUTPUT_RAW.servo1_raw - 1500) / 400.0
    theta = radians(steering_angle)
    return (wheeltrack/2) + (wheelbase/sin(theta))

def rover_yaw_rate(VFR_HUD, SERVO_OUTPUT_RAW):
    '''return yaw rate in degrees/second given steering_angle and speed'''
    max_wheel_turn=35
    speed = VFR_HUD.groundspeed
    # assume 1100 to 1900 PWM on steering
    steering_angle = max_wheel_turn * (SERVO_OUTPUT_RAW.servo1_raw - 1500) / 400.0
    if abs(steering_angle) < 1.0e-6 or abs(speed) < 1.0e-6:
        return 0
    d = rover_turn_circle(SERVO_OUTPUT_RAW)
    c = pi * d
    t = c / speed
    rate = 360.0 / t
    return rate

def rover_lat_accel(VFR_HUD, SERVO_OUTPUT_RAW):
    '''return lateral acceleration in m/s/s'''
    speed = VFR_HUD.groundspeed
    yaw_rate = rover_yaw_rate(VFR_HUD, SERVO_OUTPUT_RAW)
    accel = radians(yaw_rate) * speed
    return accel


def demix1(servo1, servo2, gain=0.5):
    '''de-mix a mixed servo output'''
    s1 = servo1 - 1500
    s2 = servo2 - 1500
    out1 = (s1+s2)*gain
    out2 = (s1-s2)*gain
    return out1+1500

def demix2(servo1, servo2, gain=0.5):
    '''de-mix a mixed servo output'''
    s1 = servo1 - 1500
    s2 = servo2 - 1500
    out1 = (s1+s2)*gain
    out2 = (s1-s2)*gain
    return out2+1500

def mixer(servo1, servo2, mixtype=1, gain=0.5):
    '''mix two servos'''
    s1 = servo1 - 1500
    s2 = servo2 - 1500
    v1 = (s1-s2)*gain
    v2 = (s1+s2)*gain
    if mixtype == 2:
        v2 = -v2
    elif mixtype == 3:
        v1 = -v1
    elif mixtype == 4:
        v1 = -v1
        v2 = -v2
    if v1 > 600:
        v1 = 600
    elif v1 < -600:
        v1 = -600
    if v2 > 600:
        v2 = 600
    elif v2 < -600:
        v2 = -600
    return (1500+v1,1500+v2)

def mix1(servo1, servo2, mixtype=1, gain=0.5):
    '''de-mix a mixed servo output'''
    (v1,v2) = mixer(servo1, servo2, mixtype=mixtype, gain=gain)
    return v1

def mix2(servo1, servo2, mixtype=1, gain=0.5):
    '''de-mix a mixed servo output'''
    (v1,v2) = mixer(servo1, servo2, mixtype=mixtype, gain=gain)
    return v2

def wrap_180(angle):
    if angle > 180:
        angle -= 360.0
    if angle < -180:
        angle += 360.0
    return angle


def wrap_360(angle):
    if angle > 360:
        angle -= 360.0
    if angle < 0:
        angle += 360.0
    return angle

class DCM_State(object):
    '''DCM state object'''
    def __init__(self, roll, pitch, yaw):
        self.dcm = Matrix3()
        self.dcm2 = Matrix3()
        self.dcm.from_euler(radians(roll), radians(pitch), radians(yaw))
        self.dcm2.from_euler(radians(roll), radians(pitch), radians(yaw))
        self.mag = Vector3()
        self.gyro = Vector3()
        self.accel = Vector3()
        self.gps = None
        self.rate = 50.0
        self.kp = 0.2
        self.kp_yaw = 0.3
        self.omega_P = Vector3()
        self.omega_P_yaw = Vector3()
        self.omega_I = Vector3() # (-0.00199045287445, -0.00653007719666, -0.00714212376624)
        self.omega_I_sum = Vector3()
        self.omega_I_sum_time = 0
        self.omega = Vector3()
        self.ra_sum = Vector3()
        self.last_delta_angle = Vector3()
        self.last_velocity = Vector3()
        (self.roll, self.pitch, self.yaw) = self.dcm.to_euler()
        (self.roll2, self.pitch2, self.yaw2) = self.dcm2.to_euler()

    def update(self, gyro, accel, mag, GPS):
        if self.gyro != gyro or self.accel != accel:
            delta_angle = old_div((gyro+self.omega_I), self.rate)
            self.dcm.rotate(delta_angle)
            correction = self.last_delta_angle % delta_angle
            #print (delta_angle - self.last_delta_angle) * 58.0
            corrected_delta = delta_angle + 0.0833333 * correction
            self.dcm2.rotate(corrected_delta)
            self.last_delta_angle = delta_angle

            self.dcm.normalize()
            self.dcm2.normalize()

            self.gyro = gyro
            self.accel = accel
            (self.roll, self.pitch, self.yaw) = self.dcm.to_euler()
            (self.roll2, self.pitch2, self.yaw2) = self.dcm2.to_euler()

dcm_state = None

def DCM_update(IMU, ATT, MAG, GPS):
    '''implement full DCM system'''
    global dcm_state
    if dcm_state is None:
        dcm_state = DCM_State(ATT.Roll, ATT.Pitch, ATT.Yaw)

    mag   = Vector3(MAG.MagX, MAG.MagY, MAG.MagZ)
    gyro  = Vector3(IMU.GyrX, IMU.GyrY, IMU.GyrZ)
    accel = Vector3(IMU.AccX, IMU.AccY, IMU.AccZ)
    accel2 = Vector3(IMU.AccX, IMU.AccY, IMU.AccZ)
    dcm_state.update(gyro, accel, mag, GPS)
    return dcm_state

class PX4_State(object):
    '''PX4 DCM state object'''
    def __init__(self, roll, pitch, yaw, timestamp):
        self.dcm = Matrix3()
        self.dcm.from_euler(radians(roll), radians(pitch), radians(yaw))
        self.gyro = Vector3()
        self.accel = Vector3()
        self.timestamp = timestamp
        (self.roll, self.pitch, self.yaw) = self.dcm.to_euler()

    def update(self, gyro, accel, timestamp):
        if self.gyro != gyro or self.accel != accel:
            delta_angle = gyro * (timestamp - self.timestamp)
            self.timestamp = timestamp
            self.dcm.rotate(delta_angle)
            self.dcm.normalize()
            self.gyro = gyro
            self.accel = accel
            (self.roll, self.pitch, self.yaw) = self.dcm.to_euler()

px4_state = None

def PX4_update(IMU, ATT):
    '''implement full DCM using PX4 native SD log data'''
    global px4_state
    if px4_state is None:
        px4_state = PX4_State(degrees(ATT.Roll), degrees(ATT.Pitch), degrees(ATT.Yaw), IMU._timestamp)

    gyro  = Vector3(IMU.GyroX, IMU.GyroY, IMU.GyroZ)
    accel = Vector3(IMU.AccX, IMU.AccY, IMU.AccZ)
    px4_state.update(gyro, accel, IMU._timestamp)
    return px4_state

_downsample_N = 0

def downsample(N):
    '''conditional that is true on every Nth sample'''
    global _downsample_N
    _downsample_N = (_downsample_N + 1) % N
    return _downsample_N == 0

def armed(HEARTBEAT):
    '''return 1 if armed, 0 if not'''
    from . import mavutil
    if HEARTBEAT.type == mavutil.mavlink.MAV_TYPE_GCS:
        self = mavutil.mavfile_global
        if self.motors_armed():
            return 1
        return 0
    if HEARTBEAT.base_mode & mavutil.mavlink.MAV_MODE_FLAG_SAFETY_ARMED:
        return 1
    return 0

def rotation_df(ATT):
    '''return the current DCM rotation matrix'''
    r = Matrix3()
    r.from_euler(radians(ATT.Roll), radians(ATT.Pitch), radians(ATT.Yaw))
    return r

def rotation2(AHRS2):
    '''return the current DCM rotation matrix'''
    r = Matrix3()
    r.from_euler(AHRS2.roll, AHRS2.pitch, AHRS2.yaw)
    return r

def earth_accel2(RAW_IMU,ATTITUDE):
    '''return earth frame acceleration vector from AHRS2'''
    r = rotation2(ATTITUDE)
    accel = Vector3(RAW_IMU.xacc, RAW_IMU.yacc, RAW_IMU.zacc) * 9.81 * 0.001
    return r * accel

def earth_accel_df(IMU,ATT):
    '''return earth frame acceleration vector from df log'''
    r = rotation_df(ATT)
    accel = Vector3(IMU.AccX, IMU.AccY, IMU.AccZ)
    return r * accel

def earth_accel2_df(IMU,IMU2,ATT):
    '''return earth frame acceleration vector from df log'''
    r = rotation_df(ATT)
    accel1 = Vector3(IMU.AccX, IMU.AccY, IMU.AccZ)
    accel2 = Vector3(IMU2.AccX, IMU2.AccY, IMU2.AccZ)
    accel = 0.5 * (accel1 + accel2)
    return r * accel

def gps_velocity_df(GPS):
    '''return GPS velocity vector'''
    vx = GPS.Spd * cos(radians(GPS.GCrs))
    vy = GPS.Spd * sin(radians(GPS.GCrs))
    return Vector3(vx, vy, GPS.VZ)

def distance_gps2(GPS, GPS2):
    '''distance between two points'''
    if GPS.TimeMS != GPS2.TimeMS:
        # reject messages not time aligned
        return None
    return distance_two(GPS, GPS2)


radius_of_earth = 6378100.0 # in meters

def wrap_valid_longitude(lon):
  ''' wrap a longitude value around to always have a value in the range
      [-180, +180) i.e 0 => 0, 1 => 1, -1 => -1, 181 => -179, -181 => 179
  '''
  return (((lon + 180.0) % 360.0) - 180.0)

def gps_newpos(lat, lon, bearing, distance):
  '''extrapolate latitude/longitude given a heading and distance
  thanks to http://www.movable-type.co.uk/scripts/latlong.html
  '''
  import math
  lat1 = math.radians(lat)
  lon1 = math.radians(lon)
  brng = math.radians(bearing)
  dr = distance/radius_of_earth

  lat2 = math.asin(math.sin(lat1)*math.cos(dr) +
                   math.cos(lat1)*math.sin(dr)*math.cos(brng))
  lon2 = lon1 + math.atan2(math.sin(brng)*math.sin(dr)*math.cos(lat1),
                           math.cos(dr)-math.sin(lat1)*math.sin(lat2))
  return (math.degrees(lat2), wrap_valid_longitude(math.degrees(lon2)))

def gps_offset(lat, lon, east, north):
  '''return new lat/lon after moving east/north
  by the given number of meters'''
  import math
  bearing = math.degrees(math.atan2(east, north))
  distance = math.sqrt(east**2 + north**2)
  return gps_newpos(lat, lon, bearing, distance)

ekf_home = None

def ekf1_pos(EKF1):
  '''calculate EKF position when EKF disabled'''
  global ekf_home
  from . import mavutil
  self = mavutil.mavfile_global
  if ekf_home is None:
      if not 'GPS' in self.messages or self.messages['GPS'].Status != 3:
          return None
      ekf_home = self.messages['GPS']
      (ekf_home.Lat, ekf_home.Lng) = gps_offset(ekf_home.Lat, ekf_home.Lng, -EKF1.PE, -EKF1.PN)
  (lat,lon) = gps_offset(ekf_home.Lat, ekf_home.Lng, EKF1.PE, EKF1.PN)
  return (lat, lon)

def quat_to_euler(q):
  '''
  Get Euler angles from a quaternion
  :param q: quaternion [w, x, y , z]
  :returns: euler angles [roll, pitch, yaw]
  '''
  quat = Quaternion(q)
  return quat.euler

def euler_to_quat(e):
  '''
  Get quaternion from euler angles
  :param e: euler angles [roll, pitch, yaw]
  :returns: quaternion [w, x, y , z]
  '''
  quat = Quaternion(e)
  return quat.q

def rotate_quat(attitude, roll, pitch, yaw):
  '''
  Returns rotated quaternion
  :param attitude: quaternion [w, x, y , z]
  :param roll: rotation in rad
  :param pitch: rotation in rad
  :param yaw: rotation in rad
  :returns: quaternion [w, x, y , z]
  '''
  quat = Quaternion(attitude)
  rotation = Quaternion([roll, pitch, yaw])
  res = rotation * quat

  return res.q

def qroll(MSG):
    '''return quaternion roll in degrees'''
    q = Quaternion([MSG.Q1,MSG.Q2,MSG.Q3,MSG.Q4])
    return degrees(q.euler[0])

    
def qpitch(MSG):
    '''return quaternion pitch in degrees'''
    q = Quaternion([MSG.Q1,MSG.Q2,MSG.Q3,MSG.Q4])
    return degrees(q.euler[1])

    
def qyaw(MSG):
    '''return quaternion yaw in degrees'''
    q = Quaternion([MSG.Q1,MSG.Q2,MSG.Q3,MSG.Q4])
    return degrees(q.euler[2])

def euler_rotated(MSG, roll, pitch, yaw):
    '''return eulers in radians from quaternion for view at given attitude in euler radians'''
    rot_view = Matrix3()
    rot_view.from_euler(roll, pitch, yaw)
    q = Quaternion([MSG.Q1,MSG.Q2,MSG.Q3,MSG.Q4])
    dcm = (rot_view * q.dcm.transposed()).transposed()
    return dcm.to_euler()

def euler_p90(MSG):
    '''return eulers in radians from quaternion for view at pitch 90'''
    return euler_rotated(MSG, 0, radians(90), 0);

def qroll_p90(MSG):
    '''return quaternion roll in degrees for view at pitch 90'''
    return degrees(euler_p90(MSG)[0])

def qpitch_p90(MSG):
    '''return quaternion roll in degrees for view at pitch 90'''
    return degrees(euler_p90(MSG)[1])

def qyaw_p90(MSG):
    '''return quaternion roll in degrees for view at pitch 90'''
    return degrees(euler_p90(MSG)[2])


def rotation_df(ATT):
    '''return the current DCM rotation matrix'''
    r = Matrix3()
    r.from_euler(radians(ATT.Roll), radians(ATT.Pitch), radians(ATT.Yaw))
    return r

def rotation2(AHRS2):
    '''return the current DCM rotation matrix'''
    r = Matrix3()
    r.from_euler(AHRS2.roll, AHRS2.pitch, AHRS2.yaw)
    return r

def earth_accel2(RAW_IMU,ATTITUDE):
    '''return earth frame acceleration vector from AHRS2'''
    r = rotation2(ATTITUDE)
    accel = Vector3(RAW_IMU.xacc, RAW_IMU.yacc, RAW_IMU.zacc) * 9.81 * 0.001
    return r * accel

def earth_accel_df(IMU,ATT):
    '''return earth frame acceleration vector from df log'''
    r = rotation_df(ATT)
    accel = Vector3(IMU.AccX, IMU.AccY, IMU.AccZ)
    return r * accel

def earth_accel2_df(IMU,IMU2,ATT):
    '''return earth frame acceleration vector from df log'''
    r = rotation_df(ATT)
    accel1 = Vector3(IMU.AccX, IMU.AccY, IMU.AccZ)
    accel2 = Vector3(IMU2.AccX, IMU2.AccY, IMU2.AccZ)
    accel = 0.5 * (accel1 + accel2)
    return r * accel

def gps_velocity_df(GPS):
    '''return GPS velocity vector'''
    vx = GPS.Spd * cos(radians(GPS.GCrs))
    vy = GPS.Spd * sin(radians(GPS.GCrs))
    return Vector3(vx, vy, GPS.VZ)

def armed(HEARTBEAT):
    '''return 1 if armed, 0 if not'''
    from pymavlink import mavutil
    if HEARTBEAT.type == mavutil.mavlink.MAV_TYPE_GCS:
        self = mavutil.mavfile_global
        if self.motors_armed():
            return 1
        return 0
    if HEARTBEAT.base_mode & mavutil.mavlink.MAV_MODE_FLAG_SAFETY_ARMED:
        return 1
    return 0

def mode(HEARTBEAT):
    '''return flight mode number'''
    from pymavlink import mavutil
    if HEARTBEAT.type == mavutil.mavlink.MAV_TYPE_GCS:
        return None
    return HEARTBEAT.custom_mode


'''
    magnetic field tables for estimating earths mag field
    updated 2019-12-20
'''

# set to the sampling in degrees for the table below
SAMPLING_RES = 10.0
SAMPLING_MIN_LAT = -90.0
SAMPLING_MAX_LAT = 90.0
SAMPLING_MIN_LON = -180.0
SAMPLING_MAX_LON = 180.0

declination_table = [
    [149.10950,139.10950,129.10950,119.10950,109.10949,99.10950,89.10950,79.10950,69.10950,59.10950,49.10950,39.10950,29.10950,19.10950,9.10950,-0.89050,-10.89050,-20.89050,-30.89050,-40.89050,-50.89050,-60.89050,-70.89050,-80.89050,-90.89050,-100.89050,-110.89050,-120.89050,-130.89050,-140.89050,-150.89050,-160.89050,-170.89050,179.10950,169.10950,159.10950,149.10950],
    [129.37759,117.14583,106.01898,95.84726,86.44522,77.63150,69.24826,61.16874,53.29825,45.57105,37.94414,30.38880,22.88112,15.39339,7.88854,0.31945,-7.36677,-15.22089,-23.28322,-31.57827,-40.11442,-48.88906,-57.89765,-67.14429,-76.65158,-86.46832,-96.67422,-107.38079,-118.72599,-130.85732,-143.89431,-157.86353,-172.61739,172.21319,157.16190,142.76170,129.37759],
    [85.60184,77.69003,71.32207,65.86993,60.92414,56.17033,51.35320,46.28164,40.84704,35.03587,28.92623,22.66416,16.41848,10.31921,4.39763,-1.44271,-7.40082,-13.70324,-20.51470,-27.87783,-35.70713,-43.83304,-52.06997,-60.27655,-68.39086,-76.44339,-84.56374,-93.00460,-102.21930,-113.07088,-127.37057,-149.05145,176.63172,138.21637,112.07842,96.22737,85.60184],
    [47.72047,46.41844,44.94283,43.50977,42.16271,40.77290,39.04552,36.59993,33.11430,28.45556,22.74662,16.37046,9.89648,3.90131,-1.27904,-5.73319,-9.95573,-14.61164,-20.21833,-26.91079,-34.40272,-42.16094,-49.65783,-56.52405,-62.55849,-67.66009,-71.72876,-74.52850,-75.43728,-72.72706,-60.57997,-20.41341,26.63644,42.82781,47.52694,48.39676,47.72047],
    [31.02920,31.23624,30.96588,30.54974,30.22312,30.09074,29.97250,29.32817,27.43015,23.68926,17.94459,10.65044,2.87620,-4.06486,-9.27368,-12.71750,-15.14455,-17.66990,-21.38496,-26.87077,-33.73354,-40.89381,-47.34608,-52.47467,-55.91656,-57.36320,-56.37027,-52.13926,-43.55753,-30.12705,-13.67554,1.91730,13.93567,22.07926,27.11546,29.86289,31.02920],
    [22.39580,22.91483,22.98471,22.79294,22.51132,22.37364,22.48467,22.51169,21.58462,18.60470,12.86231,4.67251,-4.38742,-12.20529,-17.49574,-20.37578,-21.69620,-22.20533,-22.93466,-25.58202,-30.65181,-36.60256,-41.68581,-44.89480,-45.67065,-43.68591,-38.75262,-30.86937,-20.99711,-11.25673,-2.98341,3.98182,9.94668,14.86513,18.60975,21.08265,22.39580],
    [16.86268,17.34487,17.55107,17.53468,17.27224,16.88812,16.63481,16.50963,15.80216,13.15648,7.42999,-1.11751,-10.42072,-17.95472,-22.58300,-24.81140,-25.51932,-24.64114,-22.09731,-20.12401,-21.49578,-25.56754,-29.71013,-31.93909,-31.38680,-28.14427,-22.75379,-15.84114,-8.81817,-3.40017,0.41409,3.84742,7.42617,10.85398,13.75385,15.78065,16.86268],
    [13.19097,13.44856,13.58422,13.65261,13.48939,13.02568,12.52149,12.14860,11.29753,8.56495,2.76096,-5.61344,-14.17225,-20.58114,-24.03412,-24.98709,-24.11858,-21.26636,-16.32028,-11.21874,-9.02165,-10.74849,-14.47798,-17.30779,-17.65042,-15.69359,-12.14311,-7.48791,-2.96526,-0.12587,1.36049,3.09789,5.60507,8.31685,10.73216,12.41267,13.19097],
    [10.92623,10.90181,10.82333,10.86460,10.78695,10.37670,9.88910,9.46007,8.36291,5.29505,-0.57591,-8.37062,-15.75003,-20.80957,-22.79710,-21.87616,-18.84351,-14.45358,-9.42840,-4.80202,-1.83473,-1.74130,-4.26028,-7.17479,-8.52577,-8.09283,-6.32284,-3.48771,-0.62426,0.78982,1.09893,2.05326,4.13896,6.57935,8.80977,10.35435,10.92623],
    [9.71011,9.51881,9.24068,9.25106,9.26720,8.95743,8.53646,8.00522,6.50726,2.98362,-2.85308,-9.84907,-15.97767,-19.64088,-20.07848,-17.56993,-13.32746,-8.73278,-4.74905,-1.53742,0.92858,1.76616,0.36916,-1.99224,-3.56114,-3.89436,-3.25158,-1.74963,-0.12369,0.39195,0.09209,0.65986,2.57335,5.00216,7.34943,9.08114,9.71011],
    [9.00312,9.03132,8.80862,8.92740,9.13380,8.96714,8.45876,7.49648,5.31405,1.20550,-4.60853,-10.79680,-15.64160,-17.86099,-17.02957,-13.81388,-9.48335,-5.27860,-2.08821,0.18491,2.08754,3.09405,2.33958,0.49969,-0.94208,-1.51458,-1.49063,-0.97753,-0.41673,-0.66423,-1.43031,-1.23789,0.43821,2.92085,5.61318,7.88479,9.00312],
    [8.03874,8.87718,9.23144,9.74451,10.27560,10.29756,9.57016,7.89237,4.74571,-0.17093,-6.17240,-11.69433,-15.25467,-16.11759,-14.45574,-11.15430,-7.17811,-3.38526,-0.55632,1.30997,2.82221,3.77763,3.40183,2.00714,0.77788,0.16424,-0.15468,-0.39946,-0.85273,-1.96753,-3.33820,-3.67623,-2.39633,0.05772,3.10388,6.04655,8.03874],
    [6.42021,8.49313,9.96485,11.21264,12.15378,12.34411,11.39654,9.00192,4.80210,-1.14083,-7.63429,-12.77860,-15.31639,-15.15258,-12.98558,-9.72317,-6.02652,-2.46224,0.32036,2.16718,3.52576,4.45316,4.47022,3.64413,2.71916,2.05267,1.37415,0.37187,-1.18524,-3.37771,-5.55055,-6.50029,-5.64204,-3.28034,-0.00971,3.47278,6.42021],
    [4.55870,7.84457,10.59505,12.78315,14.21311,14.53879,13.38981,10.37263,5.13228,-2.00167,-9.27410,-14.41195,-16.39580,-15.63899,-13.13217,-9.75841,-6.05603,-2.45211,0.55836,2.75052,4.36042,5.58048,6.24404,6.24213,5.76940,4.95204,3.62521,1.54168,-1.40447,-4.90584,-7.98277,-9.46456,-8.87577,-6.53558,-3.08458,0.80580,4.55870],
    [3.13967,7.31097,11.07216,14.15725,16.20221,16.79070,15.47250,11.72257,5.14656,-3.57391,-11.94254,-17.34882,-19.11810,-18.05435,-15.26042,-11.58179,-7.54393,-3.53438,0.07849,3.08157,5.54519,7.63184,9.31427,10.36791,10.53101,9.56965,7.27456,3.54700,-1.35789,-6.53724,-10.58593,-12.40763,-11.80293,-9.26734,-5.52522,-1.23338,3.13967],
    [2.40982,7.18541,11.61646,15.39834,18.09395,19.11444,17.67695,12.80844,3.91551,-7.49296,-17.41503,-23.01926,-24.41774,-22.89374,-19.60750,-15.34185,-10.59502,-5.72094,-1.00157,3.37937,7.37061,10.97982,14.11553,16.47981,17.57833,16.80075,13.55567,7.60935,-0.25054,-7.92815,-13.21489,-15.22877,-14.33921,-11.39247,-7.22465,-2.48217,2.40982],
    [1.84909,7.14349,12.09954,16.39700,19.54576,20.73345,18.58921,11.09809,-2.76476,-18.58691,-29.30539,-33.52891,-33.25409,-30.30365,-25.79412,-20.37504,-14.44263,-8.26365,-2.03561,4.09039,9.99389,15.55055,20.57404,24.74657,27.54152,28.12085,25.24078,17.56424,5.48335,-6.76322,-14.61951,-17.38523,-16.44524,-13.21307,-8.68808,-3.52579,1.84909],
    [-0.07018,5.11056,9.81033,13.43064,14.95811,12.44881,2.42652,-17.21607,-37.22275,-47.59912,-50.02338,-48.04885,-43.68750,-37.95581,-31.39385,-24.31250,-16.90710,-9.31264,-1.63265,6.04381,13.62973,21.02738,28.11104,34.69910,40.50309,45.02417,47.32932,45.58173,36.48238,17.86736,-1.80184,-12.43534,-15.24263,-13.75101,-10.05982,-5.28238,-0.07018],
    [-177.79784,-167.79784,-157.79784,-147.79784,-137.79784,-127.79784,-117.79784,-107.79784,-97.79784,-87.79784,-77.79784,-67.79784,-57.79784,-47.79784,-37.79784,-27.79784,-17.79784,-7.79784,2.20217,12.20217,22.20217,32.20217,42.20217,52.20217,62.20217,72.20217,82.20217,92.20217,102.20217,112.20217,122.20217,132.20217,142.20217,152.20217,162.20217,172.20217,-177.79784]
]

inclination_table = [
    [-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447,-72.08447],
    [-78.33243,-77.56645,-76.64486,-75.60941,-74.49599,-73.33711,-72.16456,-71.01082,-69.90877,-68.88978,-67.98065,-67.20063,-66.55969,-66.05909,-65.69426,-65.45930,-65.35147,-65.37404,-65.53651,-65.85220,-66.33408,-66.99021,-67.82010,-68.81276,-69.94649,-71.18994,-72.50361,-73.84119,-75.15044,-76.37388,-77.45008,-78.31699,-78.91913,-79.21830,-79.20379,-78.89480,-78.33243],
    [-80.91847,-79.09801,-77.26826,-75.41050,-73.49957,-71.51974,-69.48020,-67.42760,-65.44927,-63.66181,-62.18407,-61.10090,-60.43119,-60.11709,-60.04466,-60.08935,-60.16521,-60.25535,-60.41391,-60.74312,-61.35672,-62.34264,-63.73840,-65.52698,-67.65072,-70.03207,-72.58967,-75.24472,-77.91857,-80.52353,-82.93966,-84.94483,-86.05606,-85.75384,-84.42566,-82.72116,-80.91847],
    [-77.51837,-75.51694,-73.59315,-71.68670,-69.71302,-67.57376,-65.19328,-62.57944,-59.87571,-57.36704,-55.41993,-54.35624,-54.29717,-55.07005,-56.26617,-57.42621,-58.22580,-58.56311,-58.55509,-58.48876,-58.73003,-59.58712,-61.19398,-63.49338,-66.31349,-69.46462,-72.79529,-76.19407,-79.56126,-82.77630,-85.61580,-87.26733,-86.31815,-84.15731,-81.85873,-79.63120,-77.51837],
    [-71.58980,-69.64769,-67.77321,-65.94443,-64.10554,-62.13602,-59.85758,-57.13808,-54.05141,-50.99340,-48.66453,-47.83440,-48.89447,-51.51382,-54.80435,-57.85064,-60.06631,-61.18986,-61.20437,-60.42942,-59.58264,-59.49073,-60.61581,-62.88772,-65.91135,-69.25190,-72.58760,-75.67681,-78.24048,-79.94645,-80.57004,-80.16984,-79.02581,-77.42625,-75.56790,-73.58591,-71.58980],
    [-64.35997,-62.39436,-60.44044,-58.48692,-56.55523,-54.62878,-52.55624,-50.07572,-47.03679,-43.74840,-41.19888,-40.75016,-43.15983,-47.79606,-53.17503,-58.10488,-62.07251,-64.81023,-65.90745,-65.16861,-63.23594,-61.51933,-61.25674,-62.70064,-65.25795,-68.11437,-70.67345,-72.55166,-73.47174,-73.47848,-72.95756,-72.18292,-71.16028,-69.83589,-68.20337,-66.33091,-64.35997],
    [-54.94450,-52.83610,-50.71907,-48.52548,-46.29540,-44.14811,-42.08032,-39.77454,-36.80280,-33.30065,-30.58530,-30.73124,-34.77290,-41.50404,-48.77340,-55.23978,-60.63675,-64.93033,-67.56104,-67.82610,-65.84530,-62.87774,-60.76994,-60.59320,-61.93831,-63.73453,-65.20081,-65.88752,-65.54695,-64.51690,-63.50867,-62.74044,-61.89461,-60.69909,-59.07143,-57.07765,-54.94450],
    [-42.10646,-39.67640,-37.35701,-34.97293,-32.46788,-30.02667,-27.76992,-25.30303,-22.01486,-18.09122,-15.32823,-16.39044,-22.30870,-31.32094,-40.74071,-48.83600,-55.18344,-59.91449,-62.78717,-63.30498,-61.42903,-57.98330,-54.69009,-53.10628,-53.23404,-54.07531,-54.84677,-54.97074,-54.01281,-52.44135,-51.30869,-50.77990,-50.14138,-48.93456,-47.08149,-44.68014,-42.10646],
    [-25.12461,-22.20972,-19.71770,-17.30812,-14.73074,-12.18096,-9.79096,-7.01347,-3.30221,0.76014,3.06841,1.05911,-6.08614,-16.75791,-28.16035,-37.77466,-44.50466,-48.52854,-50.32132,-50.14678,-47.98684,-44.21133,-40.39026,-38.23866,-37.87010,-38.34407,-38.95664,-39.04699,-37.96969,-36.26974,-35.35719,-35.32784,-34.98679,-33.69969,-31.46052,-28.41042,-25.12461],
    [-4.97565,-1.60199,0.92214,3.11849,5.46677,7.81249,10.07397,12.84091,16.38100,19.76510,21.12151,18.61808,11.61848,1.03273,-10.71878,-20.60587,-26.95396,-29.87498,-30.40244,-29.49437,-27.12952,-23.19291,-19.13605,-16.84996,-16.43635,-16.86796,-17.51065,-17.78722,-16.98601,-15.63402,-15.29474,-15.99460,-16.17398,-15.02255,-12.56796,-8.96345,-4.97565],
    [14.91447,18.35017,20.72172,22.57409,24.52718,26.56390,28.61333,31.02478,33.78706,36.01013,36.36989,33.79530,27.90158,19.21562,9.54519,1.38248,-3.71763,-5.61325,-5.28417,-3.97847,-1.76155,1.76478,5.44818,7.52401,7.87847,7.51982,7.00436,6.66556,7.01607,7.64759,7.26628,5.87086,4.95018,5.43404,7.43409,10.86385,14.91447],
    [31.20265,34.13364,36.24286,37.87203,39.58418,41.50443,43.52947,45.65845,47.68007,48.91359,48.52705,46.02412,41.42395,35.29504,28.85019,23.50541,20.17823,19.13590,19.80674,21.15030,22.94717,25.52415,28.20453,29.75720,30.04189,29.82318,29.56477,29.39315,29.46905,29.45564,28.53745,26.73967,25.15400,24.67305,25.61444,27.99981,31.20265],
    [43.45897,45.53118,47.31626,48.90746,50.63263,52.61803,54.74225,56.79950,58.45770,59.16957,58.40919,56.07765,52.56584,48.52949,44.70395,41.69430,39.88037,39.44508,40.12934,41.29382,42.64758,44.29218,45.93985,46.96938,47.25944,47.23840,47.23429,47.28737,47.30538,46.94314,45.73923,43.78378,41.82093,40.58525,40.46579,41.54349,43.45897],
    [53.18759,54.43224,55.88059,57.49427,59.34040,61.41406,63.57613,65.58932,67.09997,67.62703,66.81035,64.77326,62.07457,59.34036,57.01844,55.33747,54.40642,54.27684,54.81467,55.68344,56.63785,57.64137,58.61275,59.35237,59.79549,60.08844,60.36788,60.60803,60.61967,60.08419,58.75938,56.80187,54.74411,53.13609,52.31629,52.38221,53.18759],
    [62.00682,62.70613,63.84875,65.37429,67.21435,69.25270,71.31641,73.17326,74.49560,74.88083,74.10396,72.39157,70.27835,68.26326,66.64304,65.52888,64.93677,64.84030,65.14402,65.67923,66.29619,66.93920,67.60553,68.28409,68.96109,69.62909,70.24417,70.67488,70.69663,70.07359,68.73701,66.90517,64.98300,63.35731,62.27637,61.83476,62.00682],
    [70.71443,71.15184,72.02039,73.27261,74.82799,76.56362,78.31147,79.84421,80.85027,81.00173,80.21459,78.77568,77.10397,75.52742,74.23457,73.30289,72.74118,72.51798,72.57149,72.82152,73.19914,73.67611,74.26606,74.99681,75.87341,76.84434,77.77369,78.43201,78.54711,77.95236,76.72716,75.15436,73.56000,72.20185,71.23761,70.73764,70.71443],
    [79.00682,79.29184,79.87277,80.71498,81.76476,82.94241,84.12827,85.13086,85.65991,85.46559,84.62947,83.45809,82.20769,81.03569,80.03242,79.24434,78.68745,78.35646,78.23285,78.29380,78.52195,78.91276,79.47430,80.21709,81.13521,82.18169,83.23875,84.08767,84.43289,84.09671,83.21590,82.09358,80.98565,80.05465,79.39115,79.03778,79.00682],
    [86.14235,86.25121,86.50061,86.87153,87.33295,87.83175,88.26493,88.44295,88.20870,87.65877,86.96733,86.23857,85.52963,84.87675,84.30531,83.83351,83.47382,83.23411,83.11886,83.13031,83.26944,83.53626,83.92911,84.44289,85.06632,85.77827,86.54222,87.29519,87.92224,88.23116,88.09287,87.66150,87.15950,86.71170,86.37734,86.18408,86.14235],
    [88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502,88.07502]
]

intensity_table = [
    [0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677,0.54677],
    [0.60733,0.60103,0.59321,0.58408,0.57385,0.56274,0.55099,0.53886,0.52664,0.51464,0.50318,0.49258,0.48311,0.47506,0.46864,0.46409,0.46158,0.46131,0.46341,0.46797,0.47499,0.48434,0.49579,0.50895,0.52332,0.53833,0.55334,0.56771,0.58086,0.59227,0.60156,0.60848,0.61292,0.61488,0.61448,0.61189,0.60733],
    [0.63154,0.61845,0.60363,0.58729,0.56950,0.55031,0.52986,0.50843,0.48660,0.46508,0.44473,0.42628,0.41025,0.39690,0.38632,0.37857,0.37385,0.37260,0.37540,0.38291,0.39557,0.41347,0.43621,0.46292,0.49236,0.52306,0.55344,0.58192,0.60704,0.62760,0.64283,0.65244,0.65659,0.65582,0.65087,0.64254,0.63154],
    [0.62000,0.60125,0.58151,0.56085,0.53899,0.51544,0.48977,0.46196,0.43279,0.40379,0.37690,0.35385,0.33554,0.32180,0.31173,0.30436,0.29937,0.29738,0.29983,0.30853,0.32501,0.34983,0.38230,0.42070,0.46273,0.50598,0.54808,0.58665,0.61944,0.64465,0.66128,0.66932,0.66957,0.66335,0.65211,0.63725,0.62000],
    [0.58540,0.56274,0.53995,0.51720,0.49410,0.46971,0.44278,0.41255,0.37961,0.34621,0.31570,0.29135,0.27491,0.26562,0.26073,0.25737,0.25418,0.25173,0.25221,0.25901,0.27564,0.30394,0.34296,0.38959,0.43988,0.49027,0.53788,0.58008,0.61437,0.63888,0.65302,0.65739,0.65335,0.64259,0.62670,0.60715,0.58540],
    [0.53990,0.51548,0.49130,0.46766,0.44447,0.42102,0.39585,0.36752,0.33593,0.30307,0.27292,0.25027,0.23814,0.23543,0.23760,0.24017,0.24116,0.24059,0.23977,0.24231,0.25416,0.28019,0.32067,0.37126,0.42556,0.47801,0.52510,0.56440,0.59381,0.61260,0.62170,0.62236,0.61571,0.60295,0.58524,0.56369,0.53990],
    [0.48818,0.46438,0.44084,0.41767,0.39521,0.37350,0.35178,0.32863,0.30295,0.27543,0.24958,0.23085,0.22313,0.22534,0.23244,0.24016,0.24725,0.25311,0.25619,0.25738,0.26278,0.28057,0.31495,0.36280,0.41574,0.46566,0.50804,0.54032,0.56070,0.57052,0.57345,0.57128,0.56367,0.55075,0.53312,0.51166,0.48818],
    [0.43218,0.41124,0.39069,0.37048,0.35104,0.33291,0.31620,0.29993,0.28232,0.26276,0.24367,0.22976,0.22479,0.22857,0.23760,0.24889,0.26192,0.27556,0.28577,0.28998,0.29174,0.29954,0.32156,0.35867,0.40314,0.44569,0.48090,0.50518,0.51615,0.51692,0.51415,0.50994,0.50209,0.48968,0.47316,0.45335,0.43218],
    [0.37898,0.36321,0.34812,0.33368,0.32029,0.30839,0.29830,0.28945,0.28010,0.26891,0.25668,0.24625,0.24088,0.24246,0.25067,0.26352,0.27927,0.29594,0.30956,0.31664,0.31798,0.31969,0.33051,0.35422,0.38581,0.41752,0.44408,0.46107,0.46532,0.46038,0.45387,0.44781,0.43921,0.42706,0.41219,0.39562,0.37898],
    [0.34141,0.33249,0.32432,0.31714,0.31161,0.30779,0.30545,0.30409,0.30213,0.29754,0.28963,0.27981,0.27109,0.26711,0.27059,0.28075,0.29432,0.30838,0.32039,0.32820,0.33136,0.33312,0.33973,0.35435,0.37434,0.39514,0.41304,0.42408,0.42536,0.41914,0.41071,0.40169,0.39062,0.37761,0.36424,0.35187,0.34141],
    [0.32867,0.32594,0.32420,0.32395,0.32630,0.33102,0.33698,0.34292,0.34678,0.34593,0.33903,0.32732,0.31415,0.30395,0.30057,0.30442,0.31263,0.32253,0.33245,0.34086,0.34708,0.35294,0.36127,0.37252,0.38535,0.39852,0.41027,0.41774,0.41871,0.41327,0.40286,0.38883,0.37279,0.35689,0.34336,0.33390,0.32867],
    [0.34041,0.34097,0.34394,0.34953,0.35870,0.37101,0.38453,0.39684,0.40514,0.40637,0.39894,0.38449,0.36738,0.35274,0.34431,0.34264,0.34599,0.35295,0.36228,0.37184,0.38062,0.38995,0.40068,0.41156,0.42185,0.43217,0.44201,0.44914,0.45121,0.44627,0.43315,0.41330,0.39087,0.37008,0.35387,0.34401,0.34041],
    [0.37313,0.37420,0.38001,0.39014,0.40446,0.42181,0.43988,0.45594,0.46693,0.46961,0.46214,0.44621,0.42680,0.40959,0.39817,0.39301,0.39304,0.39763,0.40583,0.41537,0.42477,0.43480,0.44612,0.45788,0.46954,0.48141,0.49294,0.50198,0.50563,0.50085,0.48580,0.46223,0.43524,0.41014,0.39049,0.37812,0.37313],
    [0.42356,0.42408,0.43096,0.44342,0.46009,0.47887,0.49731,0.51304,0.52358,0.52620,0.51923,0.50394,0.48467,0.46656,0.45306,0.44501,0.44198,0.44352,0.44889,0.45639,0.46470,0.47409,0.48543,0.49886,0.51385,0.52947,0.54409,0.55526,0.56003,0.55559,0.54075,0.51743,0.49032,0.46455,0.44383,0.43002,0.42356],
    [0.48455,0.48475,0.49083,0.50202,0.51666,0.53249,0.54719,0.55888,0.56585,0.56650,0.55991,0.54692,0.53031,0.51359,0.49951,0.48937,0.48348,0.48174,0.48371,0.48850,0.49531,0.50420,0.51581,0.53051,0.54771,0.56575,0.58223,0.59442,0.59970,0.59626,0.58388,0.56455,0.54198,0.52029,0.50258,0.49049,0.48455],
    [0.54041,0.54034,0.54396,0.55064,0.55927,0.56837,0.57642,0.58216,0.58460,0.58302,0.57718,0.56756,0.55545,0.54258,0.53062,0.52078,0.51381,0.51011,0.50972,0.51240,0.51793,0.52626,0.53755,0.55170,0.56796,0.58472,0.59979,0.61087,0.61607,0.61448,0.60648,0.59374,0.57883,0.56443,0.55261,0.54449,0.54041],
    [0.57307,0.57207,0.57258,0.57422,0.57649,0.57880,0.58055,0.58121,0.58037,0.57778,0.57340,0.56742,0.56031,0.55268,0.54526,0.53876,0.53378,0.53081,0.53014,0.53192,0.53617,0.54284,0.55170,0.56233,0.57398,0.58557,0.59583,0.60355,0.60784,0.60838,0.60548,0.60000,0.59319,0.58628,0.58027,0.57579,0.57307],
    [0.57801,0.57662,0.57545,0.57444,0.57349,0.57249,0.57133,0.56991,0.56816,0.56605,0.56360,0.56089,0.55803,0.55520,0.55261,0.55047,0.54900,0.54836,0.54871,0.55012,0.55257,0.55599,0.56021,0.56498,0.56997,0.57483,0.57918,0.58272,0.58521,0.58659,0.58688,0.58625,0.58495,0.58326,0.58141,0.57962,0.57801],
    [0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612,0.56612]
]


def interpolate_table(table, latitude_deg, longitude_deg):
    '''interpolate inside a table for a given lat/lon in degrees'''
    # round down to nearest sampling resolution
    min_lat = int(floor(latitude_deg / SAMPLING_RES) * SAMPLING_RES)
    min_lon = int(floor(longitude_deg / SAMPLING_RES) * SAMPLING_RES)

    # find index of nearest low sampling point
    min_lat_index = int(floor(-(SAMPLING_MIN_LAT) + min_lat) / SAMPLING_RES)
    min_lon_index = int(floor(-(SAMPLING_MIN_LON) + min_lon) / SAMPLING_RES)

    # calculate intensity
    data_sw = table[min_lat_index][min_lon_index]
    data_se = table[min_lat_index][min_lon_index + 1]
    data_ne = table[min_lat_index + 1][min_lon_index + 1]
    data_nw = table[min_lat_index + 1][min_lon_index]

    # perform bilinear interpolation on the four grid corners
    data_min = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_se - data_sw) + data_sw
    data_max = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_ne - data_nw) + data_nw

    value = ((latitude_deg - min_lat) / SAMPLING_RES) * (data_max - data_min) + data_min
    return value


'''
calculate magnetic field intensity and orientation, interpolating in tables

returns array [declination_deg, inclination_deg, intensity] or None
'''
def get_mag_field_ef(latitude_deg, longitude_deg):
    # limit to table bounds
    if latitude_deg < SAMPLING_MIN_LAT:
        return None
    if latitude_deg >= SAMPLING_MAX_LAT:
        return None
    if longitude_deg < SAMPLING_MIN_LON:
        return None
    if longitude_deg >= SAMPLING_MAX_LON:
        return None

    intensity_gauss = interpolate_table(intensity_table, latitude_deg, longitude_deg)
    declination_deg = interpolate_table(declination_table, latitude_deg, longitude_deg)
    inclination_deg = interpolate_table(inclination_table, latitude_deg, longitude_deg)

    return [declination_deg, inclination_deg, intensity_gauss]
    
earth_field = None
earth_declination = None

def expected_earth_field_lat_lon(lat, lon):
    '''return expected magnetic field for a location'''
    global earth_field
    if earth_field is not None:
        return earth_field

    field_var = get_mag_field_ef(lat, lon)
    mag_ef = Vector3(field_var[2]*1000.0, 0.0, 0.0)
    R = Matrix3()
    R.from_euler(0.0, -radians(field_var[1]), radians(field_var[0]))
    mag_ef = R * mag_ef
    earth_field = mag_ef
    return earth_field

def expected_earth_field(GPS):
    '''return expected magnetic field for a location'''
    global earth_field
    global earth_declination
    if earth_field is not None:
        return earth_field

    if hasattr(GPS,'fix_type'):
        gps_status = GPS.fix_type
        lat = GPS.lat*1.0e-7
        lon = GPS.lon*1.0e-7
    else:
        gps_status = GPS.Status
        lat = GPS.Lat
        lon = GPS.Lng

    if gps_status < 3:
        return Vector3(0,0,0)
    field_var = get_mag_field_ef(lat, lon)
    earth_declination = field_var[0]
    mag_ef = Vector3(field_var[2]*1000.0, 0.0, 0.0)
    R = Matrix3()
    R.from_euler(0.0, -radians(field_var[1]), radians(field_var[0]))
    mag_ef = R * mag_ef
    earth_field = mag_ef
    return earth_field

def expected_mag(GPS,ATT,roll_adjust=0,pitch_adjust=0,yaw_adjust=0):
    '''return expected magnetic field for a location and attitude'''
    global earth_field

    expected_earth_field(GPS)
    if earth_field is None:
        return Vector3(0,0,0)

    if hasattr(ATT,'roll'):
        roll = degrees(ATT.roll)+roll_adjust
        pitch = degrees(ATT.pitch)+pitch_adjust
        yaw = degrees(ATT.yaw)+yaw_adjust
    else:
        roll = ATT.Roll+roll_adjust
        pitch = ATT.Pitch+pitch_adjust
        yaw = ATT.Yaw+yaw_adjust

    rot = Matrix3()
    rot.from_euler(radians(roll), radians(pitch), radians(yaw))

    field = rot.transposed() * earth_field

    return field

def expected_mag_latlon(lat,lon,ATT,roll_adjust=0,pitch_adjust=0,yaw_adjust=0):
    '''return expected magnetic field for a location and attitude'''
    global earth_field

    expected_earth_field_lat_lon(lat,lon)
    if earth_field is None:
        return Vector3(0,0,0)

    if hasattr(ATT,'roll'):
        roll = degrees(ATT.roll)+roll_adjust
        pitch = degrees(ATT.pitch)+pitch_adjust
        yaw = degrees(ATT.yaw)+yaw_adjust
    else:
        roll = ATT.Roll+roll_adjust
        pitch = ATT.Pitch+pitch_adjust
        yaw = ATT.Yaw+yaw_adjust

    rot = Matrix3()
    rot.from_euler(radians(roll), radians(pitch), radians(yaw))

    field = rot.transposed() * earth_field

    return field

def mag_yaw(GPS,ATT,MAG):
    '''calculate heading from raw magnetometer'''
    ef = expected_earth_field(GPS)
    mag_x = MAG.MagX
    mag_y = MAG.MagY
    mag_z = MAG.MagZ

    # go via a DCM matrix to match the APM calculation
    dcm_matrix = rotation_df(ATT)
    cos_pitch_sq = 1.0-(dcm_matrix.c.x*dcm_matrix.c.x)
    headY = mag_y * dcm_matrix.c.z - mag_z * dcm_matrix.c.y
    headX = mag_x * cos_pitch_sq - dcm_matrix.c.x * (mag_y * dcm_matrix.c.y + mag_z * dcm_matrix.c.z)

    global earth_declination
    heading = degrees(atan2(-headY,headX)) + earth_declination
    if heading < 0:
        heading += 360
    return heading

def expected_mag_yaw(GPS,ATT,MAG,roll_adjust=0,pitch_adjust=0,yaw_adjust=0):
    '''return expected magnetic field for a location and attitude'''

    earth_field = expected_earth_field(GPS)

    roll = ATT.Roll+roll_adjust
    pitch = ATT.Pitch+pitch_adjust
    yaw = mag_yaw(GPS,ATT,MAG)

    rot = Matrix3()
    rot.from_euler(radians(roll), radians(pitch), radians(yaw))

    field = rot.transposed() * earth_field

    return field

def earth_field_error(GPS,NKF2):
    '''return vector error in earth field estimate'''
    global earth_field
    expected_earth_field(GPS)
    if earth_field is None:
        return Vector3(0,0,0)
    ef = Vector3(NKF2.MN,NKF2.ME,NKF2.MD)
    ret = ef - earth_field
    return ret


def distance_home_df(GPS,ORGN):
    '''distance from home origin'''
    return distance_two(GPS_RAW, first_fix)

def airspeed_estimate(GLOBAL_POSITION_INT,WIND):
    '''estimate airspeed'''
    wind = WIND
    gpi = GLOBAL_POSITION_INT
    from pymavlink.rotmat import Vector3
    import math
    wind3d = Vector3(wind.speed*math.cos(math.radians(wind.direction)),
                     wind.speed*math.sin(math.radians(wind.direction)), 0)
    ground = Vector3(gpi.vx*0.01, gpi.vy*0.01, 0)
    airspeed = (ground + wind3d).length()
    return airspeed


def distance_from(GPS_RAW1, lat, lon):
    '''distance from a given location'''
    if hasattr(GPS_RAW1, 'Lat'):
        lat1 = radians(GPS_RAW1.Lat)
        lon1 = radians(GPS_RAW1.Lng)
    elif hasattr(GPS_RAW1, 'cog'):
        lat1 = radians(GPS_RAW1.lat)*1.0e-7
        lon1 = radians(GPS_RAW1.lon)*1.0e-7
    else:
        lat1 = radians(GPS_RAW1.lat)
        lon1 = radians(GPS_RAW1.lon)

    lat2 = radians(lat)
    lon2 = radians(lon)

    dLat = lat2 - lat1
    dLon = lon2 - lon1

    a = sin(0.5*dLat)**2 + sin(0.5*dLon)**2 * cos(lat1) * cos(lat2)
    c = 2.0 * atan2(sqrt(a), sqrt(1.0-a))
    ground_dist = 6371 * 1000 * c
    return ground_dist

def distance_lat_lon(lat1, lon1, lat2, lon2):
    '''distance between two points'''
    dLat = radians(lat2) - radians(lat1)
    dLon = radians(lon2) - radians(lon1)

    a = sin(0.5*dLat)**2 + sin(0.5*dLon)**2 * cos(lat1) * cos(lat2)
    c = 2.0 * atan2(sqrt(a), sqrt(1.0-a))
    ground_dist = 6371 * 1000 * c
    return ground_dist

def constrain(v, minv, maxv):
    if v < minv:
        v = minv
    if v > maxv:
        v = maxv
    return v

def sim_body_rates(SIM):
    '''return body frame rates from simulator attitudes'''
    rollRate = delta(SIM.Roll,'sbr',SIM.TimeUS)
    pitchRate = delta(SIM.Pitch,'sbp',SIM.TimeUS)
    yawRate = delta(SIM.Yaw,'sby',SIM.TimeUS)
    phi = radians(SIM.Roll)
    theta = radians(SIM.Pitch)
    phiDot = radians(rollRate)
    thetaDot = radians(pitchRate)
    psiDot = radians(yawRate)

    p = phiDot - psiDot*sin(theta)
    q = cos(phi)*thetaDot + sin(phi)*psiDot*cos(theta)
    r = cos(phi)*psiDot*cos(theta) - sin(phi)*thetaDot
    return Vector3(p, q, r)

def reset_state_data():
    '''reset state data, used on log rewind'''
    global average_data
    global derivative_data
    global lowpass_data
    global last_diff
    global last_delta
    global first_fix
    global dcm_state
    global earth_field
    global last_sum
    global last_integral
    average_data.clear()
    derivative_data.clear()
    lowpass_data.clear()
    last_delta.clear()
    last_sum.clear()
    last_integral.clear()
    first_fix = None
    dcm_state = None
    earth_field = None
