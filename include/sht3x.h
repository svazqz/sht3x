#ifndef SHT3X_H
#define SHT3X_H

#include <soft_i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHT3x_ADDR 0x44
#define SHT3x_MSB 0x2C
#define SHT3x_LSB 0x10

typedef struct {
  float temperature;
  float humidity;
} SHT3X;

/* On failure, leaves the previous sample unchanged. */
bool readSensorChecked(SHT3X *sensor);

/* Compatibility wrapper: writes NaN for both measurements on failure. */
void readSensor(SHT3X *sensor);

#ifdef __cplusplus
}
#endif

#endif
