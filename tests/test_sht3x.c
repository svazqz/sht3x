#include "mock_i2c.h"
#include <sht3x.h>
#include <assert.h>
#include <math.h>
#include <string.h>

static const uint8_t valid_sample[] = { 0xbe, 0xef, 0x92, 0, 0, 0x81 };

int main(void)
{
    SHT3X sample = { 0 };
    mock_load(valid_sample, sizeof(valid_sample));
    assert(readSensorChecked(&sample));
    assert(fabsf(sample.temperature - (48879 * 175.0f / 65535.0f - 45.0f)) < 0.001f);
    assert(sample.humidity == 0.0f);
    const uint8_t writes[] = { 0x88, 0x2c, 0x10, 0x89 };
    assert(bus_mock.write_count == sizeof(writes) && memcmp(bus_mock.writes, writes, sizeof(writes)) == 0);
    assert(bus_mock.starts == 2 && bus_mock.stops == 2 && bus_mock.reply_count == 6);
    for (size_t index = 0; index < 5; ++index)
        assert(bus_mock.replies[index] == I2C_ACK);
    assert(bus_mock.replies[5] == I2C_NACK);

    for (size_t crc = 2; crc <= 5; crc += 3) {
        uint8_t bad_sample[sizeof(valid_sample)];
        memcpy(bad_sample, valid_sample, sizeof(bad_sample));
        bad_sample[crc] ^= 1;
        mock_load(bad_sample, sizeof(bad_sample));
        sample = (SHT3X){ 15.0f, 42.0f };
        assert(!readSensorChecked(&sample));
        assert(sample.temperature == 15.0f && sample.humidity == 42.0f);
    }

    mock_load(valid_sample, sizeof(valid_sample));
    bus_mock.nack_at_write = 2;
    readSensor(&sample);
    assert(isnan(sample.temperature) && isnan(sample.humidity));
    assert(bus_mock.starts == 1 && bus_mock.stops == 1 && bus_mock.read_count == 0);

    mock_load(valid_sample, sizeof(valid_sample));
    bus_mock.nack_at_write = 4;
    assert(!readSensorChecked(&sample) && bus_mock.read_count == 0 && bus_mock.stops == 2);

    mock_load(valid_sample, sizeof(valid_sample));
    bus_mock.fail_at_read = 3;
    assert(!readSensorChecked(&sample) && bus_mock.read_count == 3 && bus_mock.stops == 2);

    const uint8_t upper_humidity[] = { 0, 0, 0x81, 0xff, 0xff, 0xac };
    mock_load(upper_humidity, sizeof(upper_humidity));
    readSensor(&sample);
    assert(sample.temperature == -45.0f && fabsf(sample.humidity - 100.0f) < 0.001f);

    mock_load(valid_sample, sizeof(valid_sample));
    bus_mock.stop_status = I2C_BUS_ERROR;
    assert(!readSensorChecked(&sample) && bus_mock.starts == 1);
    assert(sample.temperature == -45.0f && fabsf(sample.humidity - 100.0f) < 0.001f);

    mock_load(NULL, 0);
    assert(!readSensorChecked(NULL));
    readSensor(NULL);
    assert(bus_mock.starts == 0);
    return 0;
}
