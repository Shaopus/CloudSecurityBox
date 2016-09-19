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
void smartconfig_init(void);

#endif
