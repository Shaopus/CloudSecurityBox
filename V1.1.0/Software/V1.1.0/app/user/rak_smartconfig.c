#include "rak_smartconfig.h"

uint8 smart_lock_flag=0;
uint8 smart_mac[6];
uint8 smart_channel=-1;
uint8 smart_status = SMART_CH_INIT;
os_timer_t channel_timer;
uint8 current_channel;
uint16 channel_bits;


SLIST_HEAD(router_info_head, router_info) router_list;

void ICACHE_FLASH_ATTR rak_promiscuous_rx(uint8 *buf, uint16 len);

void wifi_scan_done(void *arg, STATUS status);
void smartconfig_end();

#define USE_RAK 0

#include "gw.h"

#if USE_RAK == 0
#include "airkiss.h"

airkiss_context_t akcontex;

const airkiss_config_t akconf =
{
  (airkiss_memset_fn)&memset,
  (airkiss_memcpy_fn)&memcpy,
  (airkiss_memcmp_fn)&memcmp,
  0
};

void start_airkiss(void)
{
  int8_t ret;
  //锟斤拷锟斤拷锌锟斤拷锟紸ES锟斤拷锟杰ｏ拷锟斤拷锟斤拷AES锟斤拷锟诫，注锟斤拷锟斤拷锟街伙拷锟剿碉拷锟斤拷锟斤拷一锟斤拷
  const char* key = "xdnb123456789012";

  os_printf("Start airkiss!\r\n");
  //锟斤拷锟矫接口筹拷始锟斤拷AirKiss锟斤拷锟教ｏ拷每锟轿碉拷锟矫该接口ｏ拷锟斤拷锟斤拷锟斤拷锟铰匡拷始锟斤拷 akconf锟斤拷要预锟斤拷锟斤拷锟矫好诧拷锟斤拷

  ret = airkiss_init(&akcontex, &akconf);
  //锟叫断凤拷锟斤拷值锟角凤拷锟斤拷确
  if (ret < 0)
	{
	  os_printf("Airkiss init failed!\r\n");
		return;
	}

#if AIRKISS_ENABLE_CRYPT
//锟斤拷锟绞癸拷锟紸ES锟斤拷锟杰癸拷锟斤拷锟斤拷要锟斤拷锟矫猴拷AES锟斤拷钥锟斤拷注锟斤拷锟斤拷锟斤拷锟饺凤拷目锟斤拷募锟斤拷募锟斤拷械暮锟揭拷锟�
  airkiss_set_key(&akcontex, key, strlen(key));
#endif
}

static void airkiss_finish(void)
{
  int8_t err;
  uint8 buffer[256];
  airkiss_result_t result;
  err = airkiss_get_result(&akcontex, &result);

  if (err == 0)
  {


	struct station_config conf1;
	os_memcpy(&conf1.ssid[0],result.ssid,32);
	os_memcpy(&conf1.password[0],result.pwd,64);
	smartconfig_end();
	//wifi_set_opmode(STATION_MODE);
	wifi_station_set_config(&conf1);
	wifi_station_set_auto_connect(1);

	set_ap_config(CONFIGING);

	deploy_led_output(0);

	os_printf("airkiss_get_result() ok!\r\n");
    os_printf("ssid = %s, pwd = %s, ssid_length = %d,pwd_length = %d, random = 0x%02x\r\n",
    		result.ssid, result.pwd, result.ssid_length, result.pwd_length, result.random);

    os_timer_disarm(&channel_timer);
	os_timer_setfn(&channel_timer, system_restart, NULL);
	os_timer_arm(&channel_timer, 1500, 0);
  }
  else
  {
	  os_printf("airkiss_get_result() failed !\r\n");
  }
}

uint32 err_flag = 0;
void ICACHE_FLASH_ATTR
rak_promiscuous_rx(uint8 *buf, uint16 len)
{
	int8_t ret;
	uint8 * hd;
	uint16 hd_len;
	if(len >= 60)
	{
		int i;
		struct sniffer_buf *p = (struct sniffer_buf *)buf;
		hd = p->buf;
		hd_len =  p->lenseq[0].len;

	}
	else
		return;
	//锟斤拷锟斤拷锟斤拷帧锟斤拷锟斤拷airkiss锟斤拷锟斤拷写锟斤拷锟�
	//os_printf("%d , %s\r\n",len,buf);
	ret = airkiss_recv(&akcontex, hd, hd_len);
	//锟叫断凤拷锟斤拷值锟斤拷确锟斤拷锟角凤拷锟斤拷锟斤拷锟脚碉拷锟斤拷锟竭讹拷取锟斤拷锟�
	if ( ret == AIRKISS_STATUS_CHANNEL_LOCKED)
	{
		os_timer_disarm(&channel_timer);
		smart_lock_flag = 1;
		os_printf("ch locked\r\n");
	}
	else if ( ret == AIRKISS_STATUS_COMPLETE )
	{
		os_printf("airkiss complete\r\n");
		os_printf("airkiss version:%s\r\n",airkiss_version());
		airkiss_finish();
		wifi_promiscuous_enable(0);//锟截闭伙拷锟斤拷模式锟斤拷平台锟斤拷锟�
	}
	else if(ret<0)
	{
		err_flag = 1;
		os_printf("ret=%d\r\n",ret);
	}
}
#endif

/******************************************************************************
 * FunctionName : channel_timer_cb
 * Description  : change channel to sniffer the current packet.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
channel_timer_cb(void *arg)
{
  uint8 i;
  static uint32 cnt=0;
  //1.th If find one channel send smartdata,lock on this.
  cnt++;
  if(cnt&0x01)
	  deploy_led_output(1);
  else
	  deploy_led_output(0);
  if(cnt == 200)
  {
	  cnt=0;
	  yy_bf(WIFI_CFG);
  }


  if( smart_channel > 0 && smart_lock_flag == 1)
  {
	os_timer_disarm(&channel_timer);
    //wifi_set_channel(smart_channel);
    //RakPrint("[smart] locked Smartlink channel=%d\n",smart_channel);
    return;
  }

#if 0
  if(err_flag != 0)
  {
	  err_flag = 0;
	  start_airkiss();
  }
#endif
  //2.th scan channel by timer 
  for (i = current_channel; i < 14; i++) {
    if ((channel_bits & (1 << i)) != 0) {
      current_channel = i + 1;

#if USE_RAK == 0
      airkiss_change_channel(&akcontex);//锟藉缓锟斤拷
#endif
      wifi_set_channel(i);

       RakPrint("[smart] current channel2 %d---%d\n", i, system_get_time());
      os_timer_arm(&channel_timer, SCAN_TIME, 0);
      break;
    }
  }

  if (i == 14) {
    current_channel = 1;
    for (i = current_channel; i < 14; i++) {
      if ((channel_bits & (1 << i)) != 0) {
        current_channel = i + 1;
#if USE_RAK == 0
    	airkiss_change_channel(&akcontex);//锟藉缓锟斤拷
#endif
        wifi_set_channel(i);

         RakPrint("[smart] current channel3%d---%d\n", i, system_get_time());
        os_timer_arm(&channel_timer, SCAN_TIME, 0);
        break;
      }
    }
  }
}

/******************************************************************************
 * FunctionName : wifi_scan_done
 * Description  : after scan done ,get the ap info, start scan channel
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
wifi_scan_done(void *arg, STATUS status)
{
  uint8 ssid[33];

  channel_bits = 0;
  current_channel = 1;

  struct router_info *info = NULL;
  
  //1.th check the router list. 
  while((info = SLIST_FIRST(&router_list)) != NULL){
    SLIST_REMOVE_HEAD(&router_list, next);

    os_free(info);
  }

  //2.th get the bss info. 
  if (status == OK) {
    uint8 i;
    struct bss_info *bss = (struct bss_info *)arg;

    while (bss != NULL) {
      os_memset(ssid, 0, 33);

      if (os_strlen(bss->ssid) <= 32) {
        os_memcpy(ssid, bss->ssid, os_strlen(bss->ssid));
      } else {
        os_memcpy(ssid, bss->ssid, 32);
      }

      if (bss->channel != 0) {
        struct router_info *info = NULL;

        RakPrint("[smart] ssid %s, channel %d, authmode %d, rssi %d\n",
            ssid, bss->channel, bss->authmode, bss->rssi);

        RakPrint(MACSTR "\n", MAC2STR(bss->bssid));

        channel_bits |= 1 << (bss->channel);

        info = (struct router_info *)os_zalloc(sizeof(struct router_info));
        info->authmode = bss->authmode;
        info->channel = bss->channel;
        os_memcpy(info->ssid,bss->ssid,32);
        os_memcpy(info->bssid, bss->bssid, 6);

        SLIST_INSERT_HEAD(&router_list, info, next);
      }
      bss = STAILQ_NEXT(bss, next);
    }

    for (i = current_channel; i < 14; i++) {
      if ((channel_bits & (1 << i)) != 0) {
        current_channel = i + 1;
        wifi_set_channel(i);
        RakPrint("[smart] current channel1 %d---%d\n", i, system_get_time());
        break;
      }
    }
#if USE_RAK == 0
    start_airkiss();
#endif
    //3.th start sniffer and scan channel. 
    wifi_promiscuous_enable(1);
    //wifi_set_promiscuous_rx_cb(wifi_promiscuous_rx);
    wifi_set_promiscuous_rx_cb(rak_promiscuous_rx);

    os_timer_disarm(&channel_timer);
    os_timer_setfn(&channel_timer, channel_timer_cb, NULL);
    os_timer_arm(&channel_timer, SCAN_TIME, 0);
  } else {
    RakPrint("[smart] err, scan status %d\n", status);
  }
}


/******************************************************************************
 * FunctionName : smartconfig_init
 * Description  : smartconfig_init
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
smartconfig_init(void)
{
  //1.th init the router info 
	//struct smart_pkt * pkt;
	//pkt = (struct smart_pkt *)os_malloc(sizeof(struct smart_pkt));
	RakPrint("rak smart config start\n");
	SLIST_INIT(&router_list);



  //2.th scan wifi 
	wifi_station_scan(NULL,wifi_scan_done);
}


/******************************************************************************
 * FunctionName : smartconfig_end
 * Description  : smartconfig_end
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
smartconfig_end()
{
  wifi_promiscuous_enable(0);
  //os_free(pkt);
}

