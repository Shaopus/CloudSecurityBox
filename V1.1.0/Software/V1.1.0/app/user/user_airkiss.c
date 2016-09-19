//ƽ̨���ͷ�ļ� 
#include "ets_sys.h" 
#include "driver/uart.h" 
#include "osapi.h" 
#include "ip_addr.h" 
#include "user_interface.h" 
 
//����AirKissͷ�ļ� 
#include "airkiss.h"


#if 1
 
//��ǰ�����������ŵ� 
uint8_t cur_channel = 1; 
 
//�����л��ŵ��Ķ�ʱ����ƽ̨��� 
os_timer_t time_serv; 
 
//AirKiss��������Ҫ��RAM��Դ�����AirKiss����Թ���������ʹ�� 
airkiss_context_t akcontex; 
 
//��һ�ָ���ʡ��Դ��ʹ�÷�����ͨ��malloc��̬����RAM��Դ����ɺ�����free�ͷţ���Ҫƽ̨֧�� 
//ʾ���� 
//airkiss_context_t *akcontexprt; 
//akcontexprt =  
//(airkiss_context_t *)os_malloc(sizeof(airkiss_context_t)); 

//����AirKiss����Ҫ�õ���һЩ��׼�������ɶ�Ӧ��Ӳ��ƽ̨�ṩ��ǰ����Ϊ��Ҫ���� 
const airkiss_config_t akconf = 
{ 
  (airkiss_memset_fn)&memset,
  (airkiss_memcpy_fn)&memcpy,
  (airkiss_memcmp_fn)&memcmp,
  0
}; 
 
/* 
 * ƽ̨��ض�ʱ���жϴ�������100ms�жϺ��л��ŵ� 
 */ 
static void time_callback(void) 
{ 
	//�л��ŵ�
	os_printf("time_callback %d\r\n",cur_channel);
	if (cur_channel >= 13)
		cur_channel = 1;
	else
		cur_channel++;
	wifi_set_channel(cur_channel); 
	airkiss_change_channel(&akcontex);//�建��
} 
 
/* 
 * airkiss�ɹ����ȡ������Ϣ��ƽ̨�޹أ��޸Ĵ�ӡ�������� 
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
     signed rssi:8;//��ʾ�ð����ź�ǿ��
     unsigned rate:4;
     unsigned is_group:1;
     unsigned:1;
     unsigned  sig_mode:2;//��ʾ�ð��Ƿ��� 11n �İ���0 ��ʾ�� 11n���� 0 ��ʾ11n
     unsigned legacy_length:12;//�������11n�İ�������ʾ���ĳ���
     unsigned damatch0:1;
     unsigned damatch1:1;
     unsigned bssidmatch0:1;
     unsigned bssidmatch1:1;
     unsigned MCS:7;//����� 11n �İ�������ʾ���ĵ��Ʊ������У���Чֵ��0-76
     unsigned CWB:1;//�����11n �İ�������ʾ�Ƿ�Ϊ HT40 �İ�
     unsigned HT_length:16;//�����11n �İ�������ʾ���ĳ���
     unsigned Smoothing:1;
     unsigned Not_Sounding:1;
     unsigned:1;
     unsigned Aggregation:1;



     unsigned STBC:2;
     unsigned FEC_CODING:1;//�����11n �İ�������ʾ�Ƿ�Ϊ LDPC �İ�
     unsigned SGI:1;
     unsigned rxend_state:8;
     unsigned ampdu_cnt:8;
     unsigned channel:4;//��ʾ�ð����ڵ��ŵ�
      unsigned:12;
};

struct LenSeq{
u16 len;//����
u16 seq;//�������кţ����и� 12bit �������кţ��� 4bit ��Fragment ��(һ����0)
u8 addr3[6];//���еĵ�3 ����ַ
};

struct sniffer_buf{
  struct RxControl rx_ctrl;
  u8 buf[36];//���� ieee80211 ��ͷ
  u16 cnt;//���ĸ���
  struct LenSeq lenseq[1];//���ĳ���
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
 * ����ģʽ��ץ����802.11����֡�����ȣ�ƽ̨��� 
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
	//������֡����airkiss����д���
	//os_printf("%d , %s\r\n",len,buf);
	ret = airkiss_recv(&akcontex, hd, hd_len);
	//�жϷ���ֵ��ȷ���Ƿ������ŵ����߶�ȡ���
	if ( ret == AIRKISS_STATUS_CHANNEL_LOCKED)
	{
		os_printf("ch locked\r\n");
		os_timer_disarm(&time_serv);
	}
	else if ( ret == AIRKISS_STATUS_COMPLETE )
	{
		os_printf("airkiss complete\r\n");
		airkiss_finish();
		wifi_promiscuous_enable(0);//�رջ���ģʽ��ƽ̨���
	}
}

/* 
 * ��ʼ������ʼ����AirKiss���̣�ƽ̨��� 
 */ 
void start_airkiss(void) 
{ 
  int8_t ret; 
  //����п���AES���ܣ�����AES���룬ע�����ֻ��˵�����һ�� 
  const char* key = "Wechatiothardwav"; 
 
  os_printf("Start airkiss!\r\n");
  //���ýӿڳ�ʼ��AirKiss���̣�ÿ�ε��øýӿڣ��������¿�ʼ�� akconf��ҪԤ�����úò���
  os_memset(akcontex,0,sizeof(airkiss_context_t));
  ret = airkiss_init(&akcontex, &akconf); 
  //�жϷ���ֵ�Ƿ���ȷ 
  if (ret < 0) 
	{
	  os_printf("Airkiss init failed!\r\n");
		return;
	}
 
#if AIRKISS_ENABLE_CRYPT 
//���ʹ��AES���ܹ�����Ҫ���ú�AES��Կ��ע�������ȷ�Ŀ��ļ��ļ��еĺ�Ҫ��
  airkiss_set_key(&akcontex, key, strlen(key)); 
#endif 

  os_printf("Finish init airkiss!\r\n");
  //������Ӳ��ƽ̨��أ�����ģ��ΪSTATIONģʽ����������ģʽ��������ʱ�����ڶ�ʱ�л��ŵ�
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
