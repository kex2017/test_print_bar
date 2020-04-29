#include "dev_cfg.h"

#include <stdio.h>
#include <string.h>

#include "periph/flashpage.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static env_cfg_t env_cfg;
static dev_cfg_t *g_dev_cfg = &env_cfg.dev_cfg;

static void load_saved_dev_cfg(void)
{
    memcpy((void *)env_cfg.env_buf, (void *)ENV_DEV_CFG_ADDR, FLASH_PAGE_SIZE);
}

void set_dev_id(uint32_t dev_id)
{
    g_dev_cfg->dev_id = dev_id;
    update_saved_dev_cfg();
}

uint32_t get_dev_id(void)
{
    return g_dev_cfg->dev_id;
}

void increase_lora_send_msg_num(void)
{
    g_dev_cfg->lora_run_status.send_msg_num += 1;
    update_saved_dev_cfg();
}

void increase_lora_recv_msg_num(void)
{
    g_dev_cfg->lora_run_status.recv_msg_num += 1;
    update_saved_dev_cfg();
}

void record_lora_error_info(uint32_t timestamp, uint8_t error_code)
{
    if(g_dev_cfg->lora_run_status.error_times < MAX_LORA_ERROR_INFO_SAVE_NUM){
        g_dev_cfg->lora_run_status.lora_error_info[g_dev_cfg->lora_run_status.error_times].error_time = timestamp;
        g_dev_cfg->lora_run_status.lora_error_info[g_dev_cfg->lora_run_status.error_times].error_code = error_code;
    }
    g_dev_cfg->lora_run_status.error_times += 1;
    update_saved_dev_cfg();
}

void update_saved_dev_cfg(void)
{
    uint32_t page = 0;
    page = (ENV_DEV_CFG_ADDR - CPU_FLASH_BASE_ADDR) / FLASH_PAGE_SIZE;
    flashpage_write(page, (void *)env_cfg.env_buf);
}

void init_dev_cfg(void)
{
    memset(env_cfg.env_buf, 0x0, FLASH_PAGE_SIZE);

    load_saved_dev_cfg();

    if (g_dev_cfg->flag != CFG_FLAG_ON)
    {
        memset(g_dev_cfg, 0x0, FLASH_PAGE_SIZE);
        g_dev_cfg->dev_id = DEFAULT_DEV_ID;
        memset(&g_dev_cfg->lora_run_status, 0, sizeof(lora_run_status_t));
        g_dev_cfg->flag = CFG_FLAG_ON;
        update_saved_dev_cfg();
    }
    DEBUG("[env]:init dev ok!\r\n");
}

int show_dev_cfg(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    printf("p2p sctm cfg info:\r\n");
    printf("\tflag :%s\r\n", g_dev_cfg->flag == CFG_FLAG_ON ? "FLAG ON" : "FLAG OFF");
    printf("\tdev id : %ld\r\n", g_dev_cfg->dev_id);
    printf("lora run status:\r\n");
    printf("\t lora send msg num :%ld\r\n", g_dev_cfg->lora_run_status.send_msg_num);
    printf("\t lora recv msg num :%ld\r\n", g_dev_cfg->lora_run_status.recv_msg_num);
    printf("\t lora cfg error times :%ld\r\n", g_dev_cfg->lora_run_status.error_times);
    for (uint8_t i = 0; i < g_dev_cfg->lora_run_status.error_times; i++)
    {
        printf("\r\tlora cfg error[%d]:timestamp :%ld, error code :%d\r\n", i,\
        g_dev_cfg->lora_run_status.lora_error_info[i].error_time,\
        g_dev_cfg->lora_run_status.lora_error_info[i].error_code);
    }
    return 0;
}
