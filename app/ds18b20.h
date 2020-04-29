#ifndef _DS18B20_H
#define _DS18B20_H

enum {
    TEMP_SENSOR_INIT_SUCCESS,
    TEMP_SENSOR_INIT_ERROR,
    TEMP_SENSOR_READ_SUCCESS,
    TEMP_SENSOR_READ_ERROR,
};

int ds18b20_init(void);

int ds18b20_get_temperature(float *out);

#endif
