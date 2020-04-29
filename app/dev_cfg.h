#ifndef DEV_CFG_H_
#define DEV_CFG_H_

#include <stdint.h>

#define FLASH_PAGE_SIZE       0x100
#define CPU_FLASH_BASE_ADDR   0x08000000
#define ENV_DEV_CFG_ADDR      0x0803FF00

#define DEFAULT_DEV_ID (1001)
#define DEFAULT_LORA_RUN_STATUS {.timestamp = 0, .error_code = 0}

#define CFG_FLAG_OFF 0x12121200
#define CFG_FLAG_ON 0x21212100

#define MAX_LORA_ERROR_INFO_SAVE_NUM (20)

typedef struct _lora_error_info
{
    uint32_t error_time;
    uint8_t error_code;
}lora_error_info_t;

typedef struct  _lora_run_status
{
    uint32_t send_msg_num;
    uint32_t recv_msg_num;
    uint32_t error_times;
    lora_error_info_t lora_error_info[MAX_LORA_ERROR_INFO_SAVE_NUM];
}lora_run_status_t;

typedef struct _dev_cfg{
    uint32_t flag;
    uint32_t dev_id;
    lora_run_status_t lora_run_status;
} dev_cfg_t;

typedef union {
	uint8_t env_buf[FLASH_PAGE_SIZE];
	dev_cfg_t dev_cfg;
}env_cfg_t;

void init_dev_cfg(void);
void update_saved_dev_cfg(void);
int show_dev_cfg(int argc, char** argv);

void set_dev_id(uint32_t dev_id);
uint32_t get_dev_id(void);

void increase_lora_send_msg_num(void);
void increase_lora_recv_msg_num(void);
void record_lora_error_info(uint32_t timestamp, uint8_t error_code);
void clear_lora_status_info(void);

#endif
