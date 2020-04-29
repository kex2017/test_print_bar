#include "ds18b20.h"

#include "board.h"
#include "ds18.h"
#include "ds18_params.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static ds18_t ds18b20_dev;

void power_on_ds18b20(void)
{
    ds18_vcc_a_on();
}

void power_off_ds18b20(void)
{
    ds18_vcc_a_off();
}

int ds18b20_init(void)
{
    int result = ds18_init(&ds18b20_dev, &ds18_params[0]);
    if (result == DS18_ERROR)
    {
        puts("[ds18b20] Error!!! The sensor pin could not be initialized");
        return TEMP_SENSOR_INIT_ERROR;
    }
    else
    {
        DEBUG("[ds18b20]: Init success!\r\n");
        return TEMP_SENSOR_INIT_SUCCESS;
    }
}

int ds18b20_get_temperature(float *out)
{
    int16_t temperature = 0;
    /* Get temperature in centidegrees celsius */
    if (ds18_get_temperature(&ds18b20_dev, &temperature) == DS18_OK)
    {
        *out = temperature / 100.0;
        DEBUG("[ds18b20] Temperature [C]: %.2f\r\n",*out);
        return TEMP_SENSOR_READ_SUCCESS;
    }
    else
    {
        puts("[Error] Could not read temperature");
        return TEMP_SENSOR_READ_ERROR;
    }
}
