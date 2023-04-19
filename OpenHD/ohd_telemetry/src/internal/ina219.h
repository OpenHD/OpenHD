/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Nevada, Reno
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Paulo Alexandre Regis <pregis@nevada.unr.edu>
 */

#ifndef _LIB_INA219_PI
#define _LIB_INA219_PI

#define RANGE_16V                           0 // Range 0-16 volts
#define RANGE_32V                           1 // Range 0-32 volts

#define GAIN_1_40MV                         0 // Maximum shunt voltage 40mV
#define GAIN_2_80MV                         1 // Maximum shunt voltage 80mV
#define GAIN_4_160MV                        2 // Maximum shunt voltage 160mV
#define GAIN_8_320MV                        3 // Maximum shunt voltage 320mV
// #define GAIN_AUTO                          -1 // Determine gain automatically

#define ADC_9BIT                            0  // 9-bit conversion time  84us.
#define ADC_10BIT                           1  // 10-bit conversion time 148us.
#define ADC_11BIT                           2  // 11-bit conversion time 2766us.
#define ADC_12BIT                           3  // 12-bit conversion time 532us.
#define ADC_2SAMP                           9  // 2 samples at 12-bit, conversion time 1.06ms.
#define ADC_4SAMP                           10 // 4 samples at 12-bit, conversion time 2.13ms.
#define ADC_8SAMP                           11 // 8 samples at 12-bit, conversion time 4.26ms.
#define ADC_16SAMP                          12 // 16 samples at 12-bit,conversion time 8.51ms
#define ADC_32SAMP                          13 // 32 samples at 12-bit, conversion time 17.02ms.
#define ADC_64SAMP                          14 // 64 samples at 12-bit, conversion time 34.05ms.
#define ADC_128SAMP                         15 // 128 samples at 12-bit, conversion time 68.10ms.

#define __ADDRESS                           0x40

#define __REG_CONFIG                        0x00
#define __REG_SHUNTVOLTAGE                  0x01
#define __REG_BUSVOLTAGE                    0x02
#define __REG_POWER                         0x03
#define __REG_CURRENT                       0x04
#define __REG_CALIBRATION                   0x05

#define __RST                               15
#define __BRNG                              13
#define __PG1                               12
#define __PG0                               11
#define __BADC4                             10
#define __BADC3                             9
#define __BADC2                             8
#define __BADC1                             7
#define __SADC4                             6
#define __SADC3                             5
#define __SADC2                             4
#define __SADC1                             3
#define __MODE3                             2
#define __MODE2                             1
#define __MODE1                             0

#define __OVF                               1
#define __CNVR                              2

#define __CONT_SH_BUS                       7

#define __SHUNT_MILLIVOLTS_LSB              0.01    // 10uV
#define __BUS_MILLIVOLTS_LSB                4       // 4mV
#define __CALIBRATION_FACTOR                0.04096
#define __MAX_CALIBRATION_VALUE             0xFFFE  // Max value supported (65534 decimal)

// In the spec (p17) the current LSB factor for the minimum LSB is
// documented as 32767, but a larger value (100.1% of 32767) is used
// to guarantee that current overflow can always be detected.
#define __CURRENT_LSB_FACTOR                32770


#include <stdint.h>

/**
 * \defgroup ina219 INA219 Current Sensor
 * This section documents the API of the INA219 sensor library.
 */

/**
 * \ingroup ina219
 * 
 * \brief INA219 class to read/write to the INA219 current sensor.
 * 
 * An INA219 object will read and write from/to a file descriptor.
 * This file descriptor is associated to a Linux I2C device,
 * representing the INA219 sensor.
 * 
 */
class INA219
{
    public:
        /**
         * @brief If the current draw from the system is known, it will
         *        give better resolution in the measurements.
         * 
         * @param shunt_resistance Shunt resistance in Ohms.
         * @param max_expected_amps Maximum expected current in Amps.
         */
        INA219(float shunt_resistance, float max_expected_amps);
        /**
         * @brief If the current draw from the system is known, it will
         *        give better resolution in the measurements.
         *        Use custom I2C address.
         * 
         * @param shunt_resistance Shunt resistance in Ohms.
         * @param max_expected_amps Maximum expected current in Amps.
         * @param address Custom I2C address.
         */
        INA219(float shunt_resistance, float max_expected_amps, uint8_t address); // Custom device address and amps

        ~INA219();
    
    
    // Private functions
    private:
        void        init_i2c(uint8_t address);
        uint16_t    read_register(uint8_t register_value);
        void        write_register(uint8_t register_address, uint16_t register_value);
        float       determine_current_lsb(float max_expected_amps, float max_possible_amps);
        void        calibrate(int bus_volts_max, float shunt_volts_max, float max_expected_amps);
    

    // Private viarables
    private:
        int     _file_descriptor;
        float   _shunt_ohms;
        float   _max_expected_amps;
        float   _min_device_current_lsb;
        int     _voltage_range;
        int     _gain;
        float   _current_lsb;
        float   _power_lsb;

    // Public functions
    public:
        /**
         * @brief Configures and calibrates how the INA219 will take measurements.
         * 
         * @param voltage_range The full scale voltage range, this is either 16V
                                or 32V represented by one of the following constants;
                                RANGE_16V, RANGE_32V (default).
        * @param gain The gain which controls the maximum range of the shunt
                    voltage represented by one of the following constants;
                    GAIN_1_40MV, GAIN_2_80MV, GAIN_4_160MV,
                    GAIN_8_320MV, GAIN_AUTO (default).
        * @param bus_adc The bus ADC resolution (9, 10, 11, or 12-bit) or
                        set the number of samples used when averaging results
                        represent by one of the following constants; ADC_9BIT,
                        ADC_10BIT, ADC_11BIT, ADC_12BIT (default),
                        ADC_2SAMP, ADC_4SAMP, ADC_8SAMP, ADC_16SAMP,
                        ADC_32SAMP, ADC_64SAMP, ADC_128SAMP.
        * @param shunt_adc The shunt ADC resolution (9, 10, 11, or 12-bit) or
                            set the number of samples used when averaging results
                            represent by one of the following constants; ADC_9BIT,
                            ADC_10BIT, ADC_11BIT, ADC_12BIT (default),
                            ADC_2SAMP, ADC_4SAMP, ADC_8SAMP, ADC_16SAMP,
                            ADC_32SAMP, ADC_64SAMP, ADC_128SAMP.
        */
        void configure(int voltage_range, int gain, int bus_adc, int shunt_adc);

        /**
         * @brief Put the INA219 into power down mode.
         * 
         */
        void sleep();

        /**
         * @brief Wake the INA219 from power down mode.
         * 
         */
        void wake();

        /**
         * @brief Reset the INA219 to its default configuration.
         * 
         */
        void reset();

        /**
         * @brief Reads the bus voltage register from the sensor and converts to Volts.
         * 
         * @return float Bus voltage in volts (V).
         */
        float voltage();

        /**
         * @brief Reads the shunt voltage register from the sensor and converts to millivolts.
         * 
         * @return float Shunt voltage in millivolts (mV). "Inf" or "NaN" when overflow is detected.
         */
        float shunt_voltage();

        /**
         * @brief This is the sum of the bus voltage and shunt voltage. 
         * 
         * @return float Supply voltage in volts (V). "Inf" or "NaN" when overflow is detected.
         */
        float supply_voltage();

        /**
         * @brief Reads the current register from the sensor and converts to milliamps.
         * 
         * @return float Current in milliamps (mA) going through the sensor. "Inf" or "NaN" when overflow is detected.
         */
        float current();

        /**
         * @brief Reads the bus power register from the sensor and converts to milliwatts.
         * 
         * @return float Bus power consumption in milliwatts (mW).  "Inf" or "NaN" when overflow is detected.
         */
        float power();
    

    // Public variables, because cant #define arrays
    public:
        float __GAIN_VOLTS[4]   = {0.04, 0.08, 0.16, 0.32};
        int   __BUS_RANGE[2]    = {16, 32};
};

#endif
