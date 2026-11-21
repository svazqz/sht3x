# sht3x

Temperature and humidity sensor driver. readSensorChecked verifies the CRC of both measurements and preserves the last sample on failure. The legacy readSensor wrapper writes NaN on failure.

Run make to initialize the pinned soft_i2c submodule if needed and build build/libsht3x.a. Run make test for host-side tests with a simulated I²C bus.

For Raspberry Pi applications, build soft_i2c from the submodule with WiringPi installed. Link libsht3x.a before libsoft_i2c.a, followed by -lwiringPi. Initialize the default bus with setupI2C before calling the driver.
