#include "sc_dev_cfg.h"
#include "dev_cfg.h"
#include "periph/rtc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

void env_cfg_usage(void)
{
    printf("\r\nusage: device env cfg commands:\r\n");
    printf("\t -h, --help\r\n");
    printf("\t\t print this help message\r\n");
    printf("\t -t, --timestamp\r\n");
    printf("\t\t set device timestamp\r\n");
    printf("\t -i, --id=<device_id>\r\n");
    printf("\t\t set new device ID\r\n");
}

static const struct option long_opts[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"timestamp", required_argument, NULL, 't'},
        {"id", required_argument, NULL, 'i'},
        {NULL, 0, NULL, 0},
};

int set_env_cfg(int argc, char **argv)
{
    int opt = 0;
    uint32_t timestamp, device_id;

    if (argc < 2 || strlen(argv[1]) < 2)
    {
        env_cfg_usage();
        return 1;
    }

    while ((opt = getopt_long(argc, argv, "ht:i:", long_opts, NULL)) != -1)
    {
        switch (opt)
        {
        case 'h':
            env_cfg_usage();
            break;
        case 't':
            timestamp = (uint32_t)atoi(optarg);
            rtc_set_counter(timestamp);
            printf("set time ok!, now timestamp is %ld", rtc_get_counter());
            break;
        case 'i':
            device_id = (uint32_t)atoi(optarg);
            set_dev_id(device_id);
            printf("set dev id as :%ld\r\n", device_id);
            break;
        default:
            if (!optarg)
            {
                printf("optarg is NULL\r\n");
                break;
            }
            printf("unknown command %s!\r\n", optarg);
            break;
        }
    }
    optind = 1;
    return 0;
}
