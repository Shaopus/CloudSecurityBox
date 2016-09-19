/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"
#include "smartconfig.h"
#include "driver/uart.h"
#include "gw.h"
#include "gpio.h"
#include "sntp.h"
#include "ip_addr.h"
#include "rak_smartconfig.h"
#include "mem.h"
#include "user_plug.h"

#define APDEBUG
#ifdef APDEBUG
#define APPrint	os_printf
#else
#define APPrint(...)
#endif
uint8 dev_id[24]={0};

#include "smartconfig.h"

#define SHOW_SNTP_USE
//#define SHOW_HEAP_USE
//#define SHOW_FACTORYMODE_USE
#define SHOW_UPGRADE_USE

#ifdef SHOW_FACTORYMODE_USE

LOCAL os_timer_t channel_timer;
#define STA_SSID "TP-LINK_27ED7A"
#define STA_PASS "xdyz123456"
//#define STA_SSID "360WiFi_xd-ship"
//#define STA_PASS "12345678"
#endif

static os_timer_t check_timer;
uint8 sta_mac[6] = {0};

extern uint8 gw_state;
extern void init_ap_ack(void);


uint8 soft_rst = 0;
uint8 ICACHE_FLASH_ATTR
jc_soft_rst(void)
{
	struct token_rst rst;

	system_rtc_mem_read(64,&rst,sizeof(struct token_rst));

	os_printf("system_rtc_mem_read = %d\n",rst.flag);

	if(rst.flag == 0x01)
	{
		os_printf("soft rst\n");
		soft_rst = 1;
		return 1;
	}
	rst.flag = 0x01;
	system_rtc_mem_write(64,&rst, sizeof(struct token_rst));

	system_rtc_mem_read(64,&rst,sizeof(struct token_rst));

	os_printf("system_rtc_mem_read = %d\n",rst.flag);
	os_printf("hard rst\n");
	return 0;

}
//娓呴櫎杞浣嶆爣蹇楋紝鐢ㄥ湪涓�閿厤缃畬鎴愪箣鍚�
void ICACHE_FLASH_ATTR
clear_soft_rst(void)
{
	struct token_rst rst;
	os_memset(&rst,0,sizeof(struct token_rst));
	system_rtc_mem_write(64,&rst, sizeof(struct token_rst));
}

uint8 ICACHE_FLASH_ATTR
get_rst_stats(void)
{
	return soft_rst;
}

void ICACHE_FLASH_ATTR
get_sta_mac(void)
{
	if(wifi_get_macaddr(0,sta_mac)==TRUE)
		APPrint("sta Mac= %02x:%02x:%02x:%02x:%02x:%02x\n",sta_mac[0],sta_mac[1],sta_mac[2],sta_mac[3],sta_mac[4],sta_mac[5]);
	else
		APPrint("get macaddr error\n");

}

uint8 gw_ip[4];
void ICACHE_FLASH_ATTR
get_sta_ip(void)
{
	struct ip_info info;
	if(wifi_get_ip_info(0,&info))
	{
		os_memcpy(gw_ip, (uint8*)&info.ip, 4);
		os_printf("sta ip = %d.%d.%d.%d\n",gw_ip[0],gw_ip[1],gw_ip[2],gw_ip[3]);
	}
	else
		os_printf("ipconfig error\n");
}

LOCAL struct espconn ptrespconn;
uint8 buf[50]={0};
void ICACHE_FLASH_ATTR
ap_send_ack(struct espconn * ptrconn)
{

	os_sprintf(buf,"MAC=%02x:%02x:%02x:%02x:%02x:%02x",dev_id[0],dev_id[1],dev_id[2],dev_id[3],dev_id[4],dev_id[5]);
	os_printf("%s\r\n",buf);


    buf[32]=gw_ip[0];
    buf[33]=gw_ip[1];
    buf[34]=gw_ip[2];
    buf[35]=gw_ip[3];

    buf[36]=dev_id[0];
	buf[37]=dev_id[1];
	buf[38]=dev_id[2];
	buf[39]=dev_id[3];
	buf[40]=dev_id[4];
	buf[41]=dev_id[5];

	espconn_send(ptrconn,(uint8*)buf,42);
}

void ICACHE_FLASH_ATTR
ap_check_send(void *arg)
{
	clear_soft_rst();
	system_restart();
}


void ICACHE_FLASH_ATTR
user_devicefind_send (void *arg)
{
	struct espconn * ptrconn = (struct espconn *)arg;
	os_printf("sendok\r\n");

	os_timer_disarm(&check_timer);
	os_timer_setfn(&check_timer, (os_timer_func_t *)ap_check_send, NULL); // only send next packet after prev packet sent successfully
	espconn_delete(ptrconn);
	os_timer_arm(&check_timer, 1000, 0);

}

uint8 rec_flag = 0;
LOCAL void ICACHE_FLASH_ATTR
user_devicefind_recv(void *arg, char *pusrdata, unsigned short length)
{
//	os_printf("Recv udp data: %s\n", pusrdata);
	struct espconn *pesp_conn = arg;
	remot_info *premot = NULL;
	sint8 value = ESPCONN_OK;

	set_ap_config(CONFIGED);
	if(os_memcmp(pusrdata,"@XD_SHIP_DEVICE@",16)==0 && rec_flag == 0)
	{
		if (espconn_get_connection_info(pesp_conn,&premot,0) == ESPCONN_OK)
		{
			os_printf("Devivefind_Recv!!!\n\r");
			pesp_conn->proto.tcp->remote_port = premot->remote_port;
			pesp_conn->proto.tcp->remote_ip[0] = premot->remote_ip[0];
			pesp_conn->proto.tcp->remote_ip[1] = premot->remote_ip[1];
			pesp_conn->proto.tcp->remote_ip[2] = premot->remote_ip[2];
			pesp_conn->proto.tcp->remote_ip[3] = premot->remote_ip[3];
			rec_flag = 1;

			ap_send_ack(pesp_conn);
		}
	}

}

void ICACHE_FLASH_ATTR
init_ap_ack(void)
{
//    int i = 1;
//    wifi_set_broadcast_if(STATIONAP_MODE); // send UDP broadcast from both station and soft-AP interface
	ptrespconn.type = ESPCONN_UDP;
    ptrespconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    ptrespconn.proto.udp->local_port = 55555;

     const char udp_remote_ip[4] = {255, 255, 255, 255};

     os_memcpy(ptrespconn.proto.udp->remote_ip, udp_remote_ip, 4);
     ptrespconn.proto.udp->remote_port = 1112;  // ESP8266 udp remote port need to be set everytime we call espconn_sent

     espconn_regist_recvcb(&ptrespconn, user_devicefind_recv);
     espconn_regist_sentcb(&ptrespconn, user_devicefind_send);
     espconn_create(&ptrespconn);
}

static uint32 recon_cnt=0;
void ICACHE_FLASH_ATTR
ap_check_ip(void *arg)
{
	os_timer_disarm(&check_timer);

	uint8 sta = wifi_station_get_connect_status();
	APPrint("sta = %d\n",sta);

	os_printf("rssi %d\n",wifi_station_get_rssi());
	if(sta == STATION_GOT_IP)
	{
		//鐎规碍妞傞崳銊ょ瑝娴兼艾鍟�瀵拷閸氾拷
		APPrint("STATION_GOT_IP\n");
		get_sta_mac();
		get_sta_ip();
		if(gw_state == GW_INIT)
		{
			gw_start_dns();

#ifdef SHOW_UPGRADE_USE
			user_webserver_init(80);
#endif

#ifdef SHOW_SNTP_USE
			user_start_ntp();
#endif
		}
		else if(gw_state == GW_CONFIG)
		{
			init_ap_ack();
		}
		else if(gw_state == GW_WORKING)
		{
			return ;
		}
	}
	else if(sta == STATION_NO_AP_FOUND || sta == STATION_CONNECTING)
	{
		os_printf("recon .. %d\r\n",recon_cnt);
		recon_cnt ++;
		if(recon_cnt == 10)
		{
			os_printf("restarting .. \r\n");
			os_timer_disarm(&check_timer);
			os_timer_setfn(&check_timer, system_restart, NULL);
			os_timer_arm(&check_timer, 1500, 0);
		}
		else
		{
			os_timer_arm(&check_timer, 10000, 0);
		}

	}
	else if(sta == STATION_WRONG_PASSWORD)
	{
		os_timer_disarm(&check_timer);
		os_timer_setfn(&check_timer, system_restart, NULL);
		os_timer_arm(&check_timer, 1500, 0);

	}
	else
	{
		os_timer_arm(&check_timer, 10000, 0);
	}
}
void ICACHE_FLASH_ATTR
ap_check_start(void)
{
	os_timer_disarm(&check_timer);
	os_timer_setfn(&check_timer, (os_timer_func_t *)ap_check_ip, NULL);
	os_timer_arm(&check_timer, 10000, 0);
}
void ICACHE_FLASH_ATTR
connect_ap(void)
{
	struct station_config conf[5];
	int n=0,i;
	n = wifi_station_get_ap_info(conf);
	APPrint("get ap info %d\n",n);
	if(n == 0)
	{
		yy_bf(WIFI_CFG);
		gw_state = GW_CONFIG;
		smartconfig_init();
	}
	else
	{
		for(i=0;i<n;i++)
		{
			APPrint("ap%d = %s,pw = %s\n",i,conf[i].ssid,conf[i].password);
		}

		//wifi_station_get_current_ap_id
		if(wifi_station_get_auto_connect() == 0)
		{
			APPrint("station is not auto connect\n");
			if(wifi_station_set_auto_connect(1) == FALSE)
			{
				APPrint("set auto connect unok\n");
			}
		}
	}
	ap_check_start();

}

void ICACHE_FLASH_ATTR
system_init_done(void)
{

	uint8 i;
	spi_flash_read((0x1fc) * SPI_FLASH_SEC_SIZE, (uint32 *)&dev_id[0],24);
	os_printf("devid = ");
	for(i=0;i<22;i++)
		os_printf("%02x ",dev_id[i]);
	os_printf("\r\n");

	os_printf("phy mode %d\r\n",wifi_get_phy_mode());
	uint8 conf=get_ap_config();
	if(conf == UNCONF)
	{
		clear_soft_rst();
		os_printf("start config\n");
		yy_bf(WIFI_CFG);
		gw_state = GW_CONFIG;
		smartconfig_init();
	}
	else if(conf == CONFIGING)
	{
		clear_soft_rst();
		os_printf("configing\n");
		gw_state = GW_CONFIG;
		connect_ap();
	}
	else if(conf == CONFIGED)
	{
		os_printf("configed gw_stat=%d\n",gw_state);
		//wifi_station_dhcpc_start();
		connect_ap();
		gw_state = GW_INIT;
	}

	set_offline_time();
	jc_soft_rst();
}


#ifdef SHOW_FACTORYMODE_USE
void ICACHE_FLASH_ATTR
button_press(void)
{
	ETS_GPIO_INTR_DISABLE(); // Disable gpio interrupts
	// Button interrupt received
	os_printf("button_press\r\n");

	// Button pressed, flip switch
	smartconfig_end();
	struct station_config conf1;
	os_sprintf(conf1.ssid,"%s",STA_SSID);
	os_sprintf(conf1.password,"%s",STA_PASS);
	wifi_station_set_config(&conf1);
	wifi_station_set_auto_connect(1);
	set_ap_config(CONFIGED);
    os_timer_disarm(&channel_timer);
	os_timer_setfn(&channel_timer, ap_check_send, NULL);
	os_timer_arm(&channel_timer, 1000, 0);
	// Clear interrupt status
	uint32 gpio_status;
	gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

	ETS_GPIO_INTR_ENABLE(); // Enable gpio interrupts
}
/******************************************************************************
 * FunctionName : user_gpio_init
 * Description  : init factory's key function
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_gpio_init(void)
{
	os_printf("user_gpio_init\r\n");
	// Configure push button
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0); // Set function
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(0)); // Set as input
	ETS_GPIO_INTR_DISABLE(); // Disable gpio interrupts
	ETS_GPIO_INTR_ATTACH(button_press, NULL);  // GPIO0 interrupt handler
	gpio_pin_intr_state_set(GPIO_ID_PIN(0), 2); // Interrupt on negative edge
	ETS_GPIO_INTR_ENABLE(); // Enable gpio interrupts
}
#endif

#ifdef SHOW_HEAP_USE
static ETSTimer prHeapTimer;
static void ICACHE_FLASH_ATTR prHeapTimerCb(void *arg) {
	system_print_meminfo();
	os_printf("Heap: %ld\n", (unsigned long)system_get_free_heap_size());
}
#endif

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

extern void at_init(void);
extern void gw_init(void);

void user_init(void)
{

	uart_init(115200, 115200);
	gw_init();

#ifdef PLUG_DEVICE
	user_plug_init();
#endif

#ifdef SHOW_FACTORYMODE_USE
	user_gpio_init();
#endif

	param_init();
	wifi_set_opmode(STATION_MODE);
	system_init_done_cb(system_init_done);

#ifdef SHOW_HEAP_USE
	os_timer_disarm(&prHeapTimer);
	os_timer_setfn(&prHeapTimer, prHeapTimerCb, NULL);
	os_timer_arm(&prHeapTimer, 60000, 1);
#endif

}

void user_rf_pre_init(void)
{
}


