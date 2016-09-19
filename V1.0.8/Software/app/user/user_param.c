#include "user_interface.h"
#include "osapi.h"
#include "gw.h"

//512KB flash 0x3c 1024KB 0x7C

#define ESP_PARAM_START_SEC   0xfc

#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_FLAG      3
struct esp_platform_sec_flag_param {
    uint8 flag;
    uint8 pad[3];
};

/******************************************************************************
 * FunctionName : user_esp_platform_load_param
 * Description  : load parameter from flash, toggle use two sector by flag value.
 * Parameters   : param--the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_load_param(void *param, uint16 len)
{
    struct esp_platform_sec_flag_param flag;

    spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                   (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));

    if (flag.flag == 0) {
        spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
                       (uint32 *)param, len);
    } else {
        spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
                       (uint32 *)param, len);
    }
}
void ICACHE_FLASH_ATTR
user_esp_platform_clear_param(void)
{
	uint8 bank[4096]={0};
	os_memset(bank,0,4096);
	spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
	                        (uint32 *)bank, 4096);
	spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
	                        (uint32 *)bank, 4096);
	spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
	                        (uint32 *)bank, 4096);
	os_printf("user_esp_platform_clear_param\n");
#if 0
	SpiFlashOpResult re;
	re = spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0);
	os_printf("erase1 %d\n",re);
	re = spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1);
	os_printf("erase2 %d\n",re);
	re = spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_FLAG);
	os_printf("erase3 %d\n",re);
#endif
}
/******************************************************************************
 * FunctionName : user_esp_platform_save_param
 * Description  : toggle save param to two sector by flag value,
 *              : protect write and erase data while power off.
 * Parameters   : param -- the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_save_param(void *param, uint16 len)
{
    struct esp_platform_sec_flag_param flag;

    spi_flash_read((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                       (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));

    if (flag.flag == 0) {
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)param, len);
        flag.flag = 1;
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_FLAG);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));
    } else {
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)param, len);
        flag.flag = 0;
        spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_FLAG);
        spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_FLAG) * SPI_FLASH_SEC_SIZE,
                        (uint32 *)&flag, sizeof(struct esp_platform_sec_flag_param));
    }
}


#define RefreshTaskPrio        	2
#define RefreshTaskQueueLen    	1	//一次只需要执行一次的写flash

os_event_t    RefreshTaskQueue[RefreshTaskQueueLen];

uint32 cnt = 0;

static void ICACHE_FLASH_ATTR
RefreshTask(os_event_t *events)
{
	gwinfo->saved = 1;
	gwinfo->saved_cnt ++;
	user_esp_platform_save_param(gwinfo, sizeof(GWCFGINFOSTRU));
	os_printf("saved 0x%x: %d\n",ESP_PARAM_START_SEC,gwinfo->saved_cnt);
}


void ICACHE_FLASH_ATTR
tri_refresh(void)
{
	system_os_post(RefreshTaskPrio, 0, 0);
}

void ICACHE_FLASH_ATTR
param_init(void)
{

	system_os_task(RefreshTask, RefreshTaskPrio, RefreshTaskQueue, RefreshTaskQueueLen);
}

