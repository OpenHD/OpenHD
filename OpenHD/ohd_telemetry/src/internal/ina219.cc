#include "ina219.h"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "openhd_platform.h"

#include <bitset>

INA219::INA219(float shunt_resistance, float max_expected_amps) {
    printf("Initializing INA219 with shunt resistance: %f ohms and max expected amps: %f A\n", shunt_resistance, max_expected_amps);
    init_i2c(__ADDRESS);

    _shunt_ohms = shunt_resistance;
    _max_expected_amps = max_expected_amps;
    _min_device_current_lsb = __CALIBRATION_FACTOR / (_shunt_ohms * __MAX_CALIBRATION_VALUE);
}

INA219::INA219(float shunt_resistance, float max_expected_amps, uint8_t address) {
    printf("Initializing INA219 with shunt resistance: %f ohms, max expected amps: %f A, and address: 0x%02X\n", shunt_resistance, max_expected_amps, address);
    init_i2c(address);

    _shunt_ohms = shunt_resistance;
    _max_expected_amps = max_expected_amps;
    _min_device_current_lsb = __CALIBRATION_FACTOR / (_shunt_ohms * __MAX_CALIBRATION_VALUE);
}

INA219::~INA219() {
    printf("Closing file descriptor: %d\n", _file_descriptor);
    close(_file_descriptor);
}

void INA219::init_i2c(uint8_t address) {
    const char* filename;

    switch (OHDPlatform::instance().platform_type) {
        case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W:
            filename = "/dev/i2c-3";
            break;
        case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3:
            filename = "/dev/i2c-2";
            break;
        case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B:
            filename = "/dev/i2c-7";
            break;
        case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A:
            filename = "/dev/i2c-8";
            break;
        default:
            filename = "/dev/i2c-1";
            break;
    }

    printf("Opening I2C bus at %s\n", filename);
    if ((_file_descriptor = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        has_any_error = true;
        return;
    }

    printf("Acquiring I2C bus access and talking to slave device at address 0x%02X\n", address);
    if (ioctl(_file_descriptor, I2C_SLAVE, address) < 0) {
        perror("Failed to acquire bus access and/or talk to slave device");
        has_any_error = true;
        close(_file_descriptor);
        return;
    }

    has_any_error = false;
}

uint16_t INA219::read_register(uint8_t register_address) {
    uint8_t buf[3];
    buf[0] = register_address;
    printf("Writing to register 0x%02X\n", register_address);

    if (write(_file_descriptor, buf, 1) != 1) {
        perror("Failed to set register");
        return 0;
    }

    usleep(1000); // Wait for data to be ready

    if (read(_file_descriptor, buf, 2) != 2) {
        perror("Failed to read register value");
        return 0;
    }

    uint16_t value = (buf[0] << 8) | buf[1];
    printf("Read value 0x%04X from register 0x%02X\n", value, register_address);
    return value;
}

void INA219::write_register(uint8_t register_address, uint16_t register_value) {
    uint8_t buf[3];
    buf[0] = register_address;
    buf[1] = register_value >> 8;
    buf[2] = register_value & 0xFF;

    printf("Writing 0x%04X to register 0x%02X\n", register_value, register_address);
    if (write(_file_descriptor, buf, 3) != 3) {
        perror("Failed to write to the i2c bus");
        has_any_error = true;
    }
}

void INA219::configure(int voltage_range, int gain, int bus_adc, int shunt_adc) {
    printf("Configuring INA219 with voltage range %d, gain %d, bus ADC %d, and shunt ADC %d\n", voltage_range, gain, bus_adc, shunt_adc);
    reset();

    int len = sizeof(__BUS_RANGE) / sizeof(__BUS_RANGE[0]);
    if (voltage_range > len - 1) {
        perror("Invalid voltage range, must be one of: RANGE_16V, RANGE_32");
        return;
    }

    _voltage_range = voltage_range;
    _gain = gain;

    calibrate(__BUS_RANGE[voltage_range], __GAIN_VOLTS[gain], _max_expected_amps);

    uint16_t calibration = (voltage_range << __BRNG | _gain << __PG0 | bus_adc << __BADC1 | shunt_adc << __SADC1 | __CONT_SH_BUS);
    write_register(__REG_CONFIG, calibration);
}

void INA219::calibrate(int bus_volts_max, float shunt_volts_max, float max_expected_amps) {
    printf("Calibrating with bus volts max: %d, shunt volts max: %f, max expected amps: %f\n", bus_volts_max, shunt_volts_max, max_expected_amps);
    float max_possible_amps = shunt_volts_max / _shunt_ohms;
    _current_lsb = determine_current_lsb(max_expected_amps, max_possible_amps);
    _power_lsb = _current_lsb * 20.0;
    uint16_t calibration = (uint16_t)trunc(__CALIBRATION_FACTOR / (_current_lsb * _shunt_ohms));
    printf("Calibration value: 0x%04X\n", calibration);
    write_register(__REG_CALIBRATION, calibration);
}

float INA219::determine_current_lsb(float max_expected_amps, float max_possible_amps) {
    float current_lsb;
    float nearest = roundf(max_possible_amps * 1000.0) / 1000.0;

    if (max_expected_amps > nearest) {
        printf("Warning: Expected current %f A is greater than max possible current %f A\n", max_expected_amps, max_possible_amps);
    }

    current_lsb = (max_expected_amps < max_possible_amps) ? (max_expected_amps / __CURRENT_LSB_FACTOR) : (max_possible_amps / __CURRENT_LSB_FACTOR);
    if (current_lsb < _min_device_current_lsb) {
        current_lsb = _min_device_current_lsb;
    }
    
    printf("Determined current LSB: %f\n", current_lsb);
    return current_lsb;
}

void INA219::sleep() {
    uint16_t config = read_register(__REG_CONFIG);
    printf("Putting INA219 to sleep. Original config: 0x%04X\n", config);
    write_register(__REG_CONFIG, config & 0xFFF8);
}

void INA219::wake() {
    uint16_t config = read_register(__REG_CONFIG);
    printf("Waking up INA219. Original config: 0x%04X\n", config);
    write_register(__REG_CONFIG, config | 0x0007);
    usleep(40); // 40us delay to recover from powerdown (p14 of spec)
}

void INA219::reset() {
    printf("Resetting INA219\n");
    write_register(__REG_CONFIG, __RST);
}

float INA219::voltage() {
    uint16_t value = read_register(__REG_BUSVOLTAGE) >> 3;
    float voltage = float(value) * __BUS_MILLIVOLTS_LSB / 1000.0;
    printf("Bus voltage: %f V\n", voltage);
    return voltage;
}

float INA219::shunt_voltage() {
    uint16_t shunt_voltage_raw = read_register(__REG_SHUNTVOLTAGE);
    float shunt_voltage = __SHUNT_MILLIVOLTS_LSB * (int16_t)shunt_voltage_raw;
    printf("Shunt voltage: %f mV\n", shunt_voltage);
    return shunt_voltage;
}

float INA219::supply_voltage() {
    float voltage = voltage() + (shunt_voltage() / 1000.0);
    printf("Supply voltage: %f V\n", voltage);
    return voltage;
}

float INA219::current() {
    uint16_t current_raw = read_register(__REG_CURRENT);
    int16_t current = (int16_t)current_raw;
    if (current > 32767) current -= 65536;
    float current_value = current * _current_lsb * 1000.0;
    printf("Current: %f mA\n", current_value);
    return current_value;
}

float INA219::power() {
    uint16_t power_raw = read_register(__REG_POWER);
    int16_t power = (int16_t)power_raw;
    float power_value = power * _power_lsb * 1000.0;
    printf("Power: %f mW\n", power_value);
    return power_value;
}
