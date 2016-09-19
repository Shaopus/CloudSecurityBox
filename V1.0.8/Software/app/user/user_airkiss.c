//平台相关头文件 
#include "ets_sys.h" 
#include "driver/uart.h" 
#include "osapi.h" 
#include "ip_addr.h" 
#include "user_interface.h" 
 
//包含AirKiss头文件 
#include "airkiss.h"


#if 1
 
//当前监听的无线信道 
uint8_t cur_channel = 1; 
 
//用于切换信道的定时器，平台相关 
os_timer_t time_serv; 
 
//AirKiss过程中需要的RAM资源，完成AirKiss后可以供其他代码使用 
airkiss_context_t akcontex; 
 
//另一种更节省资源的使用方法，通过malloc动态申请RAM资源，完成后利用free释放，需要平台支持 
//示例： 
//airkiss_context_t *akcontexprt; 
//akcontexprt =  
//(airkiss_context_t *)os_malloc(sizeof(airkiss_context_t)); 

//定义AirKiss库需要用到的一些标准函数，由对应的硬件平台提供，前三个为必要函数 
const airkiss_config_t akconf = 
{ 
  (airkiss_memset_fn)&memset,
  (airkiss_memcpy_fn)&memcpy,
  (airkiss_memcmp_fn)&memcmp,
  0
}; 
 
/* 
 * 平台相关定时器中断处理函数，100ms中断后切换信道 
 */ 
static void time_callback(void) 
{ 
	//切换信道
	os_printf("time_callback %d\r\n",cur_channel);
	if (cur_channel >= 13)
		cur_channel = 1;
	else
		cur_channel++;
	wifi_set_channel(cur_channel); 
	airkiss_change_channel(&akcontex);//清缓存
} 
 
/* 
 * airkiss成功后读取配置信息，平台无关，修改打印函数即可 
 */ 
static void airkiss_finish(void) 
{ 
  int8_t err; 
  uint8 buffer[256]; 
  airkiss_result_t result; 
  err = airkiss_get_result(&akcontex, &result);
  
  if (err == 0) 
  { 
    //os_printf("airkiss_get_result() ok!");
    //os_sprintf(buffer, "ssid = \"%s\", pwd = \"%s\", ssid_length = %d, "pwd_length = %d, random = 0x%02x\r\n", result.ssid, result.pwd, result.ssid_length, result.pwd_length, result.random);
    //os_printf(buffer);
	os_printf("airkiss_get_result() ok!\r\n");
    os_printf("ssid = %s, pwd = %s, ssid_length = %d,pwd_length = %d, random = 0x%02x\r\n", result.ssid, result.pwd, result.ssid_length, result.pwd_length, result.random);

  } 
  else 
  { 
	  os_printf("airkiss_get_result() failed !\r\n");
  } 
} 
 
#pragma pack(1)
struct RxControl {
     signed rssi:8;//表示该包的信号强度
     unsigned rate:4;
     unsigned is_group:1;
     unsigned:1;
     unsigned  sig_mode:2;//表示该包是否是 11n 的包，0 表示非 11n，非 0 表示11n
     unsigned legacy_length:12;//如果不是11n的包，它表示包的长度
     unsigned damatch0:1;
     unsigned damatch1:1;
     unsigned bssidmatch0:1;
     unsigned bssidmatch1:1;
     unsigned MCS:7;//如果是 11n 的包，它表示包的调制编码序列，有效值：0-76
     unsigned CWB:1;//如果是11n 的包，它表示是否为 HT40 的包
     unsigned HT_length:16;//如果是11n 的包，它表示包的长度
     unsigned Smoothing:1;
     unsigned Not_Sounding:1;
     unsigned:1;
     unsigned Aggregation:1;



     unsigned STBC:2;
     unsigned FEC_CODING:1;//如果是11n 的包，它表示是否为 LDPC 的包
     unsigned SGI:1;
     unsigned rxend_state:8;
     unsigned ampdu_cnt:8;
     unsigned channel:4;//表示该包所在的信道
      unsigned:12;
};

struct LenSeq{
u16 len;//包长
u16 seq;//包的序列号，其中高 12bit 就是序列号，低 4bit 是Fragment 号(一般是0)
u8 addr3[6];//包中的第3 个地址
};

struct sniffer_buf{
  struct RxControl rx_ctrl;
  u8 buf[36];//包含 ieee80211 包头
  u16 cnt;//包的个数
  struct LenSeq lenseq[1];//包的长度
};


#define MAX_PW		64
struct smart_pkt
{
	uint8 smart_buf[MAX_PW];
	uint8 flag[MAX_PW/2];
	uint8 pw_len;
};

#pragma pack()
/* 
 * 混杂模式下抓到的802.11网络帧及长度，平台相关 
 */ 
static void wifi_promiscuous_rx(uint8 *buf, uint16 len) 
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
	//将网络帧传入airkiss库进行处理
	//os_printf("%d , %s\r\n",len,buf);
	ret = airkiss_recv(&akcontex, hd, hd_len);
	//判断返回值，确定是否锁定信道或者读取结果
	if ( ret == AIRKISS_STATUS_CHANNEL_LOCKED)
	{
		os_printf("ch locked\r\n");
		os_timer_disarm(&time_serv);
	}
	else if ( ret == AIRKISS_STATUS_COMPLETE )
	{
		os_printf("airkiss complete\r\n");
		airkiss_finish();
		wifi_promiscuous_enable(0);//关闭混杂模式，平台相关
	}
}

/* 
 * 初始化并开始进入AirKiss流程，平台相关 
 */ 
void start_airkiss(void) 
{ 
  int8_t ret; 
  //如果有开启AES功能，定义AES密码，注意与手机端的密码一致 
  const char* key = "Wechatiothardwav"; 
 
  os_printf("Start airkiss!\r\n");
  //调用接口初始化AirKiss流程，每次调用该接口，流程重新开始， akconf需要预先设置好参数
  os_memset(akcontex,0,sizeof(airkiss_context_t));
  ret = airkiss_init(&akcontex, &akconf); 
  //判断返回值是否正确 
  if (ret < 0) 
	{
	  os_printf("Airkiss init failed!\r\n");
		return;
	}
 
#if AIRKISS_ENABLE_CRYPT 
//如果使用AES加密功能需要设置好AES密钥，注意包含正确的库文件文件中的宏要打开
  airkiss_set_key(&akcontex, key, strlen(key)); 
#endif 

  os_printf("Finish init airkiss!\r\n");
  //以下与硬件平台相关，设置模块为STATION模式并开启混杂模式，启动定时器用于定时切换信道
  wifi_station_disconnect(); 
  wifi_set_opmode(STATION_MODE); 
  cur_channel = 1; 
  wifi_set_channel(cur_channel); 
  
    os_timer_setfn(&time_serv, (os_timer_func_t 
*)time_callback, NULL); 
  os_timer_arm(&time_serv, 100, 1); 
  wifi_set_promiscuous_rx_cb(wifi_promiscuous_rx); 
  wifi_promiscuous_enable(1); 
} 
#endif
