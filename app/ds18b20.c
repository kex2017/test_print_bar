#include "ds18b20.h"
#include "data_send.h"

#include "board.h"
#include "ds18.h"
#include "ds18_params.h"
#include "frame_decode.h"

#include "xtimer.h"
#include "periph/rtc.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define TEMP_SAMPLE_SERV_PRIORITY (4)
#define TEMP_SAMPLE_SERV_STACKSIZE (1024)
char temp_sample_serv_stack[TEMP_SAMPLE_SERV_STACKSIZE];

static uint32_t temp_sample_interval_last_time = 0;
static uint32_t temp_sample_interval_now_time = 0;

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
        return TEMP_SENSOR_SUCCESS;
    }
}

int ds18b20_get_temperature(float *out)
{
    int16_t temperature = 0;

    /* Get temperature in centidegrees celsius */
    if (ds18_get_temperature(&ds18b20_dev, &temperature) == DS18_OK)
    {
        *out = temperature / 100.0;
        DEBUG("[ds18b20] Temperature [C]: %.2f\r\n", *out);
        return TEMP_SENSOR_SUCCESS;
    }
    else
    {
        puts("[Error] Could not read temperature");
        return TEMP_SENSOR_READ_ERROR;
    }
}

void *temp_sample_serv(void *arg)
{
    (void)arg;
    float temperature = 0.0;
    int error_code = 0;
    uint8_t retry_times = 0;

    error_code = ds18b20_init();

    while (1)
    {
        temp_sample_interval_now_time = rtc_get_counter();
        if (temp_sample_interval_now_time - temp_sample_interval_last_time >= TEMP_SAMPLE_PERIOD)
        {
            retry_times = 0;
            power_on_ds18b20();
            do
            {
                error_code &= ~TEMP_SENSOR_READ_ERROR;
                error_code |= ds18b20_get_temperature(&temperature);
                if (!error_code)
                    break;
                xtimer_usleep(50 * 1000);
                retry_times++;
            } while (error_code && (retry_times < TEMP_SAMPLE_RETRY_TIMES));

            if (retry_times == TEMP_SAMPLE_RETRY_TIMES)
            {
                DEBUG("[ds18b20]: read temperature error , error code is %x\r\n", error_code);
            }
            else
            {
                error_code &= ~TEMP_SENSOR_READ_ERROR;
            }
            power_off_ds18b20();
            encode_and_send_temp_data(temp_sample_interval_now_time, temperature, error_code);
            temp_sample_interval_last_time = temp_sample_interval_now_time;
        }
        xtimer_sleep(TEMP_SAMPLE_PERIOD);
    }
    return NULL;
}

kernel_pid_t temperature_sample_serv_start(void)
{
    kernel_pid_t _sample_pid = thread_create(temp_sample_serv_stack,
                                            sizeof(temp_sample_serv_stack),
                                            TEMP_SAMPLE_SERV_PRIORITY, THREAD_CREATE_STACKTEST,
                                            temp_sample_serv, NULL, "temperature_sample_serv");
    printf("temperature pid is %d\r\n", _sample_pid);
    get_sample_pid_hook(_sample_pid);
    return _sample_pid;
}
