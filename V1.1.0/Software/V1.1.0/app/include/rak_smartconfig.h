/*
 *  Copyright (C) 2014 -2015  By younger
 *
 */

#ifndef __SMARTCONFIG_H__
#define __SMARTCONFIG_H__

#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#define RECEVEBUF_MAXLEN  90
#define SCAN_TIME  100

//#define RAKDEBUG
#ifdef RAKDEBUG
#define RakPrint	os_printf
#else
#define RakPrint(...)
#endif

typedef enum _encrytion_mode {
    ENCRY_NONE           = 1,
    ENCRY_WEP,
    ENCRY_TKIP,
    ENCRY_CCMP
} ENCYTPTION_MODE;

struct router_info {
    SLIST_ENTRY(router_info)     next;

    u8 ssid[32];
    u8 bssid[6];
    u8 channel;
    u8 authmode;

    u16 rx_seq;
    u8 encrytion_mode;
    u8 iv[8];
    u8 iv_check;
};

enum smart_status
{
  SMART_CH_INIT = 0x1,
  SMART_CH_LOCKING = 0x2,
  SMART_CH_LOCKED = 0x4
};

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
void smartconfig_init(void);

#endif
