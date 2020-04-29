#ifndef _DS18B20_H
#define _DS18B20_H

#include <stdint.h>
#include "kernel_types.h"

enum {
    TEMP_SENSOR_SUCCESS = 0,
    TEMP_SENSOR_INIT_ERROR = 1,
    TEMP_SENSOR_READ_ERROR = 2,
};

#define TEMP_SAMPLE_RETRY_TIMES (5)
#define TEMP_SAMPLE_PERIOD (5)

int ds18b20_init(void);

int ds18b20_get_temperature(float *out);

kernel_pid_t temperature_sample_serv_start(void);

#endif
