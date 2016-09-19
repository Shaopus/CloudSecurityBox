/******************************************************************************
 * Copyright (C) 2014 -2016  Espressif System
 *
 * FileName: user_plug.c
 *
 * Description: plug demo's function realization
 *
 * Modification history:
 * 2015/7/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_config.h"
#include "user_interface.h"
#include "gw.h"

#if PLUG_DEVICE
#include "user_plug.h"

LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[PLUG_KEY_NUM];


/******************************************************************************
 * FunctionName : user_plug_get_status
 * Description  : get plug's status, 0x00 or 0x01
 * Parameters   : none
 * Returns      : uint8 - plug's status
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
user_plug_get_status(void)
{
    return gwinfo->plug_status;
}

/******************************************************************************
 * FunctionName : user_plug_set_status
 * Description  : set plug's status, 0x00 or 0x01
 * Parameters   : uint8 - status
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_plug_set_status(bool status)
{
    if (status != gwinfo->plug_status) {
        if (status > 1) {
            os_printf("error status input!\n");
            return;
        }
        os_printf("status input! %d\n", status);

        gwinfo->plug_status= status;
        PLUG_STATUS_OUTPUT(PLUG_RELAY_LED_IO_NUM, status);
    }
}

/******************************************************************************
 * FunctionName : user_plug_short_press
 * Description  : key's short press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_plug_short_press(void)
{
    user_plug_set_status((~gwinfo->plug_status) & 0x01);
}

/******************************************************************************
 * FunctionName : user_plug_long_press
 * Description  : key's long press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_plug_long_press(void)
{
    int boot_flag=12345;

    system_restore();
    
    system_rtc_mem_write(70, &boot_flag, sizeof(boot_flag));
    os_printf("long_press boot_flag %d  \n",boot_flag);
    system_rtc_mem_read(70, &boot_flag, sizeof(boot_flag));
    os_printf("long_press boot_flag %d  \n",boot_flag);

    system_restart();
}
/******************************************************************************
 * FunctionName : user_get_key_status
 * Description  : a
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
//BOOL ICACHE_FLASH_ATTR
//user_get_key_status(void)
//{
//    return get_key_status(single_key[0]);
//}

/******************************************************************************
 * FunctionName : user_plug_init
 * Description  : init plug's key function and relay output
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_plug_init(void)
{
    os_printf("user_plug_init start!\n");

    single_key[0] = key_init_single(PLUG_KEY_0_IO_NUM, PLUG_KEY_0_IO_MUX, PLUG_KEY_0_IO_FUNC,
                                    user_plug_long_press, user_plug_short_press);

    keys.key_num = PLUG_KEY_NUM;
    keys.single_key = single_key;

    key_init(&keys);

    PIN_FUNC_SELECT(PLUG_RELAY_LED_IO_MUX, PLUG_RELAY_LED_IO_FUNC);
}
#endif

