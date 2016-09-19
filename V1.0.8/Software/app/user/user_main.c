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

#define APDEBUG
#ifdef APDEBUG
#define APPrint	os_printf
#else
#define APPrint(...)
#endif
uint8 dev_id[22]={0};

#include "smartconfig.h"

#define SHOW_SNTP_USE

#ifdef SHOW_SNTP_USE
LOCAL os_timer_t sntp_timer;
LOCAL uint32 ntp_retry_counter = 0;
LOCAL void user_start_ntp(void);
#endif

static os_timer_t check_timer;
uint8 sta_mac[6] = {0};

extern uint8 gw_state;
extern void init_ap_ack(void);

//#define SHOW_HEAP_USE
#define STA_SSID "TP-LINK_27ED7A"
#define STA_PASS "xdyz123456"

//#define STA_SSID "360WiFi_xd-ship"
//#define STA_PASS "12345678"

static os_timer_t channel_timer;

uint8 soft_rst = 0;
uint8 ICACHE_FLASH_ATTR
jc_soft_rst(void)
{
	uint8 buf[10];
	system_rtc_mem_read(64,buf,sizeof(buf));
	os_printf("rtc_mem_read = %s\n",buf);

	if(os_strncmp(buf, "SOFT_RST", os_strlen("SOFT_RST")) == 0)
	{
		os_printf("soft rst\n");
		soft_rst = 1;
		return 1;
	}
	system_rtc_mem_write(64,"SOFT_RST", os_strlen("SOFT_RST"));
	os_printf("hard rst\n");

	return 0;
}
//清除软复位标志，用在一键配置完成之后
void ICACHE_FLASH_ATTR
clear_soft_rst(void)
{
	system_rtc_mem_write(64,"HARD_RST", os_strlen("HARD_RST"));
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
		//瀹氭椂鍣ㄤ笉浼氬啀寮�鍚�
		APPrint("STATION_GOT_IP\n");
		get_sta_mac();
		get_sta_ip();
		if(gw_state == GW_INIT)
		{
			gw_start_dns();
			user_webserver_init(80);
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
  //smartconfig_init();

	uint8 i;
	spi_flash_read((0x1fc) * SPI_FLASH_SEC_SIZE, (uint32 *)&dev_id[0],22);
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

void ICACHE_FLASH_ATTR
button_press(void)
{
	ETS_GPIO_INTR_DISABLE(); // Disable gpio interrupts
	// Button interrupt received
	os_printf("BUTTON: Button pressed\r\n");

	// Button pressed, flip switch
//	if (GPIO_REG_READ(BUTTON_GPIO) & BIT12) {
	// Debounce
	smartconfig_end();
	struct station_config conf1;
	os_sprintf(conf1.ssid,"%s",STA_SSID);
	os_sprintf(conf1.password,"%s",STA_PASS);
	wifi_station_set_config(&conf1);
	wifi_station_set_auto_connect(1);
	set_ap_config(CONFIGED);
//	os_delay_us(200000);
    os_timer_disarm(&channel_timer);
	os_timer_setfn(&channel_timer, ap_check_send, NULL);
	os_timer_arm(&channel_timer, 1000, 0);
//	system_restart();
	// Clear interrupt status
	uint32 gpio_status;
	gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

	ETS_GPIO_INTR_ENABLE(); // Enable gpio interrupts
}

void ICACHE_FLASH_ATTR
user_gpio_init(void)
{
	// Configure push button
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0); // Set function
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(0)); // Set as input
	ETS_GPIO_INTR_DISABLE(); // Disable gpio interrupts
	ETS_GPIO_INTR_ATTACH(button_press, NULL);  // GPIO0 interrupt handler
	gpio_pin_intr_state_set(GPIO_ID_PIN(0), 2); // Interrupt on negative edge
	ETS_GPIO_INTR_ENABLE(); // Enable gpio interrupts
}

#ifdef SHOW_HEAP_USE
static ETSTimer prHeapTimer;
static void ICACHE_FLASH_ATTR prHeapTimerCb(void *arg) {
	system_print_meminfo();
	os_printf("Heap: %ld\n", (unsigned long)system_get_free_heap_size());
}
#endif

#ifdef SHOW_SNTP_USE
Time_Inf time_inf;
int lasttime = 0;
int ICACHE_FLASH_ATTR
user_get_ntpweek(Time_Inf *time)
{
	int week =0;
	if(0==memcmp("Mon",(char*)&time->week,3))
			week = 1;
		else if(0==memcmp("Tue",(char*)&time->week,3))
				week = 2;
			else if(0==memcmp("Wed",(char*)&time->week,3))
					week = 3;
				else if(0==memcmp("Thu",(char*)&time->week,3))
						week = 4;
					else if(0==memcmp("Fri",(char*)&time->week,3))
							week = 5;
						else if(0==memcmp("Sta",(char*)&time->week,3))
								week = 6;
							else if(0==memcmp("Sun",(char*)&time->week,3))
									week = 7;
								else
									return -1;
	return week;
}

int ICACHE_FLASH_ATTR
user_get_ntphour(Time_Inf *time)
{
	return atoi((char*)&time->hour);
}

int ICACHE_FLASH_ATTR
user_get_ntpmin(Time_Inf *time)
{
	return atoi((char*)&time->min);
}

void ICACHE_FLASH_ATTR
user_set_timemode(uint8 mode)
{
	os_printf("arm 0x%x\n",mode);
	if(mode>0 && mode<=4)
	{
		uint8 modechanged = 0;
		if(mode == OUTMODE)
		{
			modechanged = 1;
			deploy_led_output(1);
			gw_changemode(OUTMODE);
		}
		else if(mode == INHOMEMODE)
		{
			modechanged = 1;
			deploy_led_output(1);
			gw_changemode(INHOMEMODE);

		}
		else if(mode == NIGHTMODE)
		{
			modechanged = 1;
			deploy_led_output(1);
			gw_changemode(NIGHTMODE);

		}
		else if(mode == CANCELMODE)
		{
			gw_changemode(CANCELMODE);
			yy_bf(SYS_UNDEPLOY);
			deploy_led_output(0);
		}
		if(modechanged == 1)
		{
			modechanged = 0;

			if(jc_cc()!=0)
			{
				uint8 buf[2];
				buf[1] = NOT_CLOSED_WINDOW;
				if(gwinfo->ucCurSceneMode == OUTMODE)
					buf[0] = OUT_MODE_START_ONE_MIN;
				else if(gwinfo->ucCurSceneMode == INHOMEMODE)
					buf[0] = INHOME_MODE;
				else if(gwinfo->ucCurSceneMode == NIGHTMODE)
					buf[0] = NIGHT_MODE;
				yy_start(2,buf);

			}
			else
			{
				if(gwinfo->ucCurSceneMode == OUTMODE)
					yy_bf(OUT_MODE_START_ONE_MIN);
				else if(gwinfo->ucCurSceneMode == INHOMEMODE)
					yy_bf(INHOME_MODE);
				else if(gwinfo->ucCurSceneMode == NIGHTMODE)
					yy_bf(NIGHT_MODE);
			}

		}
	}
	os_printf("Timer Change Mode!\n\r");
	tri_refresh();
}

void ICACHE_FLASH_ATTR
user_check_timer(int ltime,int nowtime,int week)
{
	uint8 i = 0,j = 0;
	int settime = 0;
	lasttime = nowtime;
	for(i = 0;i<TIMER_TOTAL;i++)
	{
		if(gwinfo->timer_inf[i].timerday_number != 0)
		{
			for(j =0 ;j < gwinfo->timer_inf[i].timerday_number; j++)
			{
				settime = (gwinfo->timer_inf[i].timerdata_inf[j].hour)*60+
						(gwinfo->timer_inf[i].timerdata_inf[j].min);
//				os_printf("settime:%d\n",settime);
				if((ltime < settime)&&(settime<= nowtime))
				{
					if((week == gwinfo->timer_inf[i].week))
					{
						uint8 mode = gwinfo->timer_inf[i].timerdata_inf[j].mode ;
						user_set_timemode(mode);
					}
				}
				else if( (settime <= nowtime)&& (nowtime< ltime))
				{
					if((week-1) == gwinfo->timer_inf[i].week)
					{
						uint8 mode = gwinfo->timer_inf[i].timerdata_inf[j].mode ;
						user_set_timemode(mode);
					}
				}

			}
		}
	}

}
void ICACHE_FLASH_ATTR
user_check_ntptime(char *real_time)
{
	int week = 0,hour = 0,min = 0;
	int nowtime = 0;
	memset(&time_inf,0,sizeof(time_inf));
	memcpy(&time_inf.week,real_time,3);
	memcpy(&time_inf.hour,real_time+11,2);
	memcpy(&time_inf.min,real_time+14,2);

	week = user_get_ntpweek(&time_inf);
	hour = user_get_ntphour(&time_inf);
	min = user_get_ntpmin(&time_inf);

	nowtime = hour*60+min;
	if (lasttime == 0) {
		lasttime = nowtime;
	}
//	os_printf("lasttime:%d,nowtime:%d\n",lasttime,nowtime);
	user_check_timer(lasttime,nowtime,week);
//	os_printf("%s,%s,%s,%d\n\r",time_inf.week,
//		time_inf.hour,time_inf.min,sizeof(time_inf));
}

void ICACHE_FLASH_ATTR
user_check_sntp_stamp(void *arg)
{
   uint32 current_stamp;
   current_stamp = sntp_get_current_timestamp();
   if(current_stamp == 0)
   {
         os_timer_arm(&sntp_timer, 100, 0);
   }
   else
   {
	  os_timer_disarm(&sntp_timer);
//      os_printf("sntp: %d, %s \n", current_stamp, sntp_get_real_time(current_stamp));
      user_check_ntptime(sntp_get_real_time(current_stamp));
   }
}

void ICACHE_FLASH_ATTR
user_get_ntp_time(void)
{
   os_timer_disarm(&sntp_timer);
   os_timer_setfn(&sntp_timer, (os_timer_func_t *)user_check_sntp_stamp, NULL);
   os_timer_arm(&sntp_timer, 100, 0);
}

LOCAL void ICACHE_FLASH_ATTR
user_start_ntp(void)
{
   ip_addr_t *addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));
   sntp_setservername(0, "us.pool.ntp.org"); // set server 0 by domain name
   sntp_setservername(1, "ntp.sjtu.edu.cn"); // set server 1 by domain name
   ipaddr_aton("210.72.145.44", addr);
   sntp_setserver(2, addr); // set server 2 by IP address
   sntp_init();
   os_free(addr);
}
#endif

extern void at_init(void);
extern void gw_init(void);
//extern void hw_test_timer_cb(void);

void user_init(void)
{

//	hw_timer_init(0,1);
//    hw_timer_set_func(hw_test_timer_cb);
//    hw_timer_arm(250);

	uart_init(115200, 115200);
	gw_init();
	user_gpio_init();
	at_init();
	param_init();
	wifi_set_opmode(STATION_MODE);
	system_init_done_cb(system_init_done);

#ifdef SHOW_HEAP_USE
	os_timer_disarm(&prHeapTimer);
	os_timer_setfn(&prHeapTimer, prHeapTimerCb, NULL);
	os_timer_arm(&prHeapTimer, 60000, 1);
#endif

//#ifdef SHOW_SNTP_USE
//	user_get_ntp_time();
//#endif
}

void user_rf_pre_init(void)
{
}


