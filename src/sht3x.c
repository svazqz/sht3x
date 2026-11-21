#define _POSIX_C_SOURCE 200809L
#include <sht3x.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <time.h>

static uint8_t measurement_crc(const uint8_t data[2])
{
    uint8_t crc = 0xff;
    for (size_t index = 0; index < 2; ++index) {
        crc ^= data[index];
        for (unsigned int bit = 0; bit < 8; ++bit)
            crc = (uint8_t)((crc << 1) ^ ((crc & 0x80) ? 0x31 : 0));
    }
    return crc;
}

bool readSensorChecked(SHT3X *sensor)
{
    if (sensor == NULL)
        return false;

    i2c_t bus = *getBus();
    i2c_start(bus);
    bool sent = i2c_send_byte(bus, (SHT3x_ADDR << 1) | I2C_WRITE) == I2C_ACK &&
                i2c_send_byte(bus, SHT3x_MSB) == I2C_ACK &&
                i2c_send_byte(bus, SHT3x_LSB) == I2C_ACK;
    i2c_stop(bus);
    if (!sent || i2c_status() != I2C_OK)
        return false;

    struct timespec remaining = { 0, 5000000L };
    while (nanosleep(&remaining, &remaining) == -1 && errno == EINTR) {
    }

    uint8_t data[6] = { 0 };
    i2c_start(bus);
    bool received = i2c_send_byte(bus, (SHT3x_ADDR << 1) | I2C_READ) == I2C_ACK;
    for (size_t index = 0; received && index < sizeof(data); ++index) {
        data[index] = i2c_read_byte(bus);
        i2c_send_bit(bus, index + 1 == sizeof(data) ? I2C_NACK : I2C_ACK);
        received = i2c_status() == I2C_OK;
    }
    i2c_stop(bus);

    if (!received || i2c_status() != I2C_OK ||
        measurement_crc(data) != data[2] || measurement_crc(data + 3) != data[5])
        return false;

    unsigned int raw_temperature = ((unsigned int)data[0] << 8) | data[1];
    unsigned int raw_humidity = ((unsigned int)data[3] << 8) | data[4];
    sensor->temperature = raw_temperature * (175.0f / 65535.0f) - 45.0f;
    sensor->humidity = raw_humidity * (100.0f / 65535.0f);
    return true;
}

void readSensor(SHT3X *sensor)
{
    if (sensor != NULL && !readSensorChecked(sensor)) {
        sensor->temperature = NAN;
        sensor->humidity = NAN;
    }
}
