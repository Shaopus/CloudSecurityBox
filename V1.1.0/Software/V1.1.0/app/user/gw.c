#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "gw.h"
#include "stdlib.h"
#include "gpio.h"

#include "user_plug.h"

extern uint8 sta_mac[6];
extern uint8 get_rst_stats(void);



#ifdef TIDPrint
void TidPrint(uint8 * id)
{
	int i;
	os_printf("TID: %d: ",system_get_rtc_time());
	for(i=0;i<8;i++)
	{
		os_printf("%02x ",id[i]);
	}
	os_printf("\n");
}
#else
#define TidPrint(...)
#endif
#define PKTPrint
#ifdef PKTPrint
void ICACHE_FLASH_ATTR
PktPrint(FrameHeader *pkt)
{
	int i;
	char *p;
	p = (uint8 *)pkt;
	for(i=0;i<T16(pkt->PacketLen);i++)
	{
		os_printf("%02x ",p[i]);
	}
	os_printf("\n");
}
#else
#define PktPrint(...)
#endif

#define 		gw_procTaskPrio        1
#define 		gw_procTaskQueueLen    10

os_event_t    	gw_procTaskQueue[gw_procTaskQueueLen];
static 			os_timer_t check_timer;

ip_addr_t 		gw_server_ip;
LOCAL struct 	espconn gw_conn;
LOCAL struct 	_esp_tcp gw_tcp;
uint32 			rsp_cnt;


GWCFGINFOSTRU * gwinfo=NULL;
FrameHeader*	fd = NULL;

uint32 offline_time = 0;

void ICACHE_FLASH_ATTR
link_led_init(void)
{
    PIN_FUNC_SELECT(LINK_LED_IO_MUX, LINK_LED_IO_FUNC);
}

void ICACHE_FLASH_ATTR
link_led_output(uint8 level)
{
    GPIO_OUTPUT_SET(GPIO_ID_PIN(LINK_LED_IO_NUM), level);
}

void ICACHE_FLASH_ATTR
deploy_led_init(void)
{
    PIN_FUNC_SELECT(DEPLOY_LED_IO_MUX, DEPLOY_LED_IO_FUNC);
}

void ICACHE_FLASH_ATTR
deploy_led_output(uint8 level)
{
    GPIO_OUTPUT_SET(GPIO_ID_PIN(DEPLOY_LED_IO_NUM), level);
}

//閿熸枻鎷烽敓鏂ゆ嫹娆犻敓鏂ゆ嫹鑳侀敓鏂ゆ嫹閿燂拷
uint8 ICACHE_FLASH_ATTR
jc_bf(void)
{
	int i;
	uint8 re;
	for(i = 0;i<NODE_TOTAL;i++)
	{
		//gwinfo->node[i].ucLastVal = 0;
		if(gwinfo->node[i].info.ucDeployStatus)
			return 1;
	}

	return 0;

}

void ICACHE_FLASH_ATTR
set_offline_time(void)
{
	//u32 rtc_cali = system_rtc_clock_cali_proc()>>12;

	//os_printf("system_rtc_clock_cali_proc = %d  ",rtc_cali);

	offline_time = 400*60*1000;
	os_printf("offline_time=%u ",offline_time);

	os_printf("sys_time = %d \n",system_get_time());
	os_printf("rtc_time = %d \n",system_get_rtc_time);
}

uint8 ICACHE_FLASH_ATTR
get_ap_config(void)
{
	return gwinfo->smartconf;
}
void ICACHE_FLASH_ATTR
set_ap_config(uint8 conf)
{
	gwinfo->smartconf = conf;
	os_printf("smartconf = 1\n\r");
	tri_refresh();
}


void ICACHE_FLASH_ATTR
init_gwdata(void)
{
	gwinfo = (GWCFGINFOSTRU *)os_malloc(sizeof(GWCFGINFOSTRU));
	fd = (FrameHeader *)os_malloc(sizeof(FrameHeader));
	gw_start_task();
}

uint8 Token[32];

uint8 gw_state;

uint32 tonline_cnt;
uint32 online_cnt;
uint32 hb_cnt;
uint32 state_cnt;
uint32 alarm_cnt;
uint32 delay_cnt;
uint8  delay_flag;
uint8  save_flag;
uint8  save_cnt;
uint8  ntp_cnt;

uint8 reset_flag = 0;

void init_gwinfo(GWCFGINFOSTRU *gw);
int jc_cc(void);
void yy_start(uint8 cnt,uint8 * nr);

void ICACHE_FLASH_ATTR
gw_initcnt(void)
{
	tonline_cnt = 0;
	online_cnt 	= 0;
	hb_cnt 		= 0;
	state_cnt 	= 0;
	alarm_cnt 	= 0;
	delay_cnt	= 0;
	delay_flag 	= 0;
	save_flag 	= 0;
	save_cnt 	= 0;

	rsp_cnt		= 0;

	reset_flag 	= 0;
	gw_state = GW_INIT;
}

void ICACHE_FLASH_ATTR
gw_init(void)
{
	gw_initcnt();
	init_gwdata();
	init_gwinfo(gwinfo);
	link_led_init();
	deploy_led_init();
	link_led_output(0);
	deploy_led_output(0);
}

//閿熸枻鎷锋椂閿熸枻鎷烽敓鑺ワ紝閿熸枻鎷烽敓鏂ゆ嫹閿熺禎lash閿熸枻鎷蜂娇閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸埅鎲嬫嫹閿熸枻鎷烽敓鏂ゆ嫹閿熷彨浼欐嫹閿熶粙鏅ā寮忛敓鏂ゆ嫹鏃堕敓鏂ゆ嫹閿熸枻鎷烽敓鐭尅鎷峰懗閿熸枻鎷烽敓鏂ゆ嫹閿熸彮浼欐嫹閿熺禎lash閿熶茎浼欐嫹閿熸枻鎷穎lash閿熸枻鎷峰啓閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰お閿熸枻鎷�
//閿熸嵎璁规嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鐠為潻鎷风墴閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰啓
#define GW_SAVE_DELAY()	save_flag = 1;save_cnt = 0;

void ICACHE_FLASH_ATTR
init_fddat(FrameHeader *hd)
{
	rsp_cnt++;
	memset(hd,0x00,sizeof(FrameHeader));
	hd->Header = 0x55;
	hd->Number = T32(rsp_cnt);
	hd->Dir = 0x03;


	os_memcpy(hd->MAC,sta_mac,MAC_SIZE);
}

extern uint8 dev_id[24];
void ICACHE_FLASH_ATTR
init_gwinfo(GWCFGINFOSTRU *gw)
{
	int i;
	os_printf("\r\ncompile time:%s %s\r\n", __DATE__, __TIME__);
	os_printf("SDK version: %s \n", system_get_sdk_version());
	os_memset(gw,0,sizeof(GWCFGINFOSTRU));
	user_esp_platform_load_param(gw,sizeof(GWCFGINFOSTRU));
	os_printf("sizeof(GWCFGINFOSTRU) = %d\r\n",sizeof(GWCFGINFOSTRU));

	if(gw->saved != 1)
	{
		os_printf("gw not saved\n");
		memset(gw,0,sizeof(GWCFGINFOSTRU));
		gw->ucCurSceneMode = CANCELMODE;
	}

	for(i = 0;i<NODE_TOTAL;i++)
	{
		if(0x01 == gw->node[i].ucVail)
		{
			gw->node[i].info.ucAlmStatus = 0;
			gw->node[i].ucLastVal = 0;
			gw->node[i].ulLastTime = system_get_rtc_time();;
		}
	}
	if((gw->ucCurSceneMode < CANCELMODE) && (gw->ucCurSceneMode >= OUTMODE))
		deploy_led_output(1);
	else
		deploy_led_output(0);

    // default to be off, for safety.
    if (gw->plug_status == 0xff) {
        gw->plug_status = 0;
    }

    PLUG_STATUS_OUTPUT(PLUG_RELAY_LED_IO_NUM, gw->plug_status);

	os_printf("saved == %d",gw->saved);
	os_memcpy(gw->ucDevID,&dev_id[0],6);
	os_memcpy(gw->ucGwPasswd,&dev_id[6],16);

	os_printf("init Gwinfo!!!\n\r");
	tri_refresh();

}

void ICACHE_FLASH_ATTR
gw_reconn()
{
	gw_initcnt();
	gw_start_dns();
}

void ICACHE_FLASH_ATTR
heart_beat_timer(void *arg)
{
    struct espconn *pgwconn = arg;

    system_os_post(gw_procTaskPrio,SIG_HEART_BEAT, (os_param_t)"SIG_HEART_BEAT\n");

}

void ICACHE_FLASH_ATTR
heart_beat(void)
{
	char hb = 0x56;
	espconn_send(&gw_conn,(uint8*)&hb,1);
}

void ICACHE_FLASH_ATTR
yy_bf(uint8 nr)
{
	uint8 buf[3];
	alarm_cnt = 0;	//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸埅纰夋嫹
	buf[0] = 0x55;
	buf[1] = 1;
	buf[2] = nr;
	uart0_tx_buffer(buf,3);
}

void ICACHE_FLASH_ATTR
yy_fire(void)
{
	uint8 buf[4];
	alarm_cnt = 0;	//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸埅纰夋嫹
	buf[0] = 0x55;
	buf[1] = 2;
	buf[2] = FIRE_SOS;
	buf[3] = FIRE_SOS_1;
	uart0_tx_buffer(buf,4);
}

void ICACHE_FLASH_ATTR
set_timestamp(TimeStamp_t * tm)
{
	tm->tm_year = T16(2015);
	tm->tm_month = 4;
	tm->tm_day = 13;
	tm->tm_hour = 15;
	tm->tm_min = 30;
	tm->tm_sec = 0;
}

void ICACHE_FLASH_ATTR
gw_alarm_send(Term_Inf *inf,uint8 type)
{
	init_fddat(fd);
	fd->PacketLen = T16(PACK_SIZE(30));
	fd->CMDID = T16(CMD_ALARM);
	os_memcpy(&fd->uCmdRsp.alarm_inf,inf,sizeof(Term_Inf));
	fd->uCmdRsp.alarm_inf.arm_type = type;
	set_timestamp(&fd->uCmdRsp.alarm_inf.tm);
	espconn_send(&gw_conn,(uint8*)fd,PACK_SIZE(30));
	os_printf("gw alarm send%02x,%02x \r\n",inf->term_type,type);
}

void ICACHE_FLASH_ATTR
gw_alarm_cancle(void)
{
	init_fddat(fd);
	fd->PacketLen = T16(PACK_SIZE(0));
	fd->CMDID = T16(CMD_CANCLEALARM);
//	PktPrint(fd);
	espconn_send(&gw_conn,(uint8*)fd,PACK_SIZE(0));
}


void ICACHE_FLASH_ATTR
gw_changemode(uint8 mode)
{
	int i;
	if(mode<OUTMODE || mode>CANCELMODE)
		return;

	gwinfo->ucCurSceneMode = mode;
	for(i=0;i<NODE_TOTAL;i++)
	{
		//if(0x01 == gwinfo->node[i].ucVail)
		{
			if(gwinfo->ucCurSceneMode == OUTMODE)
				gwinfo->node[i].info.ucDeployStatus = gwinfo->node[i].ucDelayStateOut;
			else if(gwinfo->ucCurSceneMode == NIGHTMODE)
				gwinfo->node[i].info.ucDeployStatus = gwinfo->node[i].ucDelayStateNight;
			else if(gwinfo->ucCurSceneMode == INHOMEMODE)
				gwinfo->node[i].info.ucDeployStatus = gwinfo->node[i].ucDelayStateInhome;
			else if(gwinfo->ucCurSceneMode == CANCELMODE)
			{
				gwinfo->node[i].info.ucDeployStatus = 0;

			}
		}
		if(gwinfo->node[i].info.ucDeployStatus==0x00)
		{
			gwinfo->node[i].info.ucAlmStatus = 0;
			//gwinfo->node[i].ucLastVal = 0;
		}
	}

}

void ICACHE_FLASH_ATTR
gw_reportarm(struct espconn *pconn,uint32 Number)
{
	uint16 i,j,len=0;
	uint8 *p;
	SceneMode * psm;
	init_fddat(fd);
	fd->Number = Number;
	fd->Dir = PKT_CMD_ACK;
	fd->CMDID = T16(CMD_REPORTARM);
	p = (uint8 *)&fd->uCmdRsp.scmode;
	psm = (SceneMode *)p;
	for(i=OUTMODE;i<=CANCELMODE;i++)
	{
		psm->ucCurMode = i;
		if(i == gwinfo->ucCurSceneMode)
			psm->ucSel = 0x01;
		else
			psm->ucSel = 0x00;
		psm->ucNodeN = gwinfo->ucNodeNum;
		p = (uint8 *)&psm->nodebase[0];
		len += 3;
		//for(j=0;j<gwinfo->ucNodeNum;j++)
		for(j = 0;j<NODE_TOTAL;j++)
		{
			if(0x01 == gwinfo->node[j].ucVail)
			{
				os_memcpy(p,gwinfo->node[j].info.ucNodeID,sizeof(NodeBaseInf)-1);

				if(i==OUTMODE)
					((NodeBaseInf *)p)->ucDeployStatus = gwinfo->node[j].ucDelayStateOut;
				else if(i==NIGHTMODE)
					((NodeBaseInf *)p)->ucDeployStatus = gwinfo->node[j].ucDelayStateNight;
				else if(i==INHOMEMODE)
					((NodeBaseInf *)p)->ucDeployStatus = gwinfo->node[j].ucDelayStateInhome;
				else if(i==CANCELMODE)
					((NodeBaseInf *)p)->ucDeployStatus = 0;
				p = p+sizeof(NodeBaseInf);
				len += sizeof(NodeBaseInf);
			}
		}
		psm = (SceneMode *)p;
	}
	fd->PacketLen = T16(PACK_SIZE(len));
	set_timestamp((TimeStamp_t *)p);
//	PktPrint(fd);
	espconn_send(pconn,(uint8*)fd,PACK_SIZE(len));
}

void ICACHE_FLASH_ATTR
gw_report(struct espconn *pconn,uint32 Number)
{
	uint16 len = 0;
	uint32 i;
	uint8 *p;
	init_fddat(fd);
	fd->Number = Number;
	fd->Dir = PKT_CMD_ACK;
	fd->CMDID = T16(CMD_REPORT);
	os_memcpy(fd->uCmdRsp.report_inf.ucDevID,gwinfo->ucDevID,26);
	len=26;
	if(gwinfo->ucNodeNum == 0)
	{
		fd->uCmdRsp.report_inf.ucNodeN = 0;
		len = len+1;
	}
	else
	{
		uint32 j;
		fd->uCmdRsp.report_inf.ucNodeN = gwinfo->ucNodeNum;
		fd->uCmdRsp.report_inf.ucNDInfSize=sizeof(NodeInf);
		len += 2;
		for(i = 0,j = 0;i<NODE_TOTAL;i++)
		{
			//閿熸枻鎷烽敓绉歌鎷烽敓鏂ゆ嫹鎭敓鏂ゆ嫹閿熺嫛鏂ゆ嫹鍘�
			if(0x01 == gwinfo->node[i].ucVail)
			{
				os_memcpy(&fd->uCmdRsp.report_inf.node[j++],&gwinfo->node[i].info,sizeof(NodeInf));
				len += sizeof(NodeInf);
			}
		}
	}
	fd->PacketLen = T16(PACK_SIZE(len));

	p = (uint8 *)(&fd->uCmdRsp.report_inf);
	p = p +len;
	set_timestamp((TimeStamp_t *)p);;
//	PktPrint(fd);
	espconn_send(pconn,(uint8*)fd,PACK_SIZE(len));


}

void ICACHE_FLASH_ATTR
gw_vernum(struct espconn *pconn,uint32 Number)
{
	init_fddat(fd);
	fd->Number = Number;
	fd->Dir = PKT_CMD_ACK;
	fd->CMDID = T16(CMD_VERNUM);
	fd->uCmdRsp.pad[0] = GW_MAJOR_VERNUM;
	fd->uCmdRsp.pad[1] = GW_MINOR_VERNUM;
	fd->uCmdRsp.pad[2] = GW_REVISION_VERNUM;
	fd->PacketLen = T16(PACK_SIZE(3));
	set_timestamp((TimeStamp_t *)&fd->uCmdRsp.pad[3]);

	espconn_send(pconn,(uint8*)fd,PACK_SIZE(3));
}

void ICACHE_FLASH_ATTR
gw_ackok(uint32 Number,uint16 CMDID)
{
	init_fddat(fd);
	fd->Number = Number;
	fd->Dir = PKT_CMD_ACK;
	fd->CMDID = T16(CMDID);
	fd->uCmdRsp.pad[0] = 0;
	fd->PacketLen = T16(PACK_SIZE(1));
	set_timestamp((TimeStamp_t *)&fd->uCmdRsp.pad[1]);
	espconn_send(&gw_conn,(uint8*)fd,PACK_SIZE(1));

}
void ICACHE_FLASH_ATTR
gw_ackfail(uint32 Number,uint16 CMDID)
{
	init_fddat(fd);
	fd->Number = Number;
	fd->Dir = PKT_CMD_ACK;
	fd->CMDID = T16(CMDID);
	fd->uCmdRsp.pad[0] = 1;
	fd->PacketLen = T16(PACK_SIZE(1));
	set_timestamp((TimeStamp_t *)&fd->uCmdRsp.pad[1]);
	espconn_send(&gw_conn,(uint8*)fd,PACK_SIZE(1));
}

int ICACHE_FLASH_ATTR
gw_deltem(uint8* id)
{
	int i;
	for(i=0;i<NODE_TOTAL;i++)
	{
		if(0==memcmp(id,gwinfo->node[i].info.ucNodeID,8))
		{
			if(gwinfo->node[i].ucVail == 0x01)
			{
				uint8 * n;
				uint8 *  m;
				gwinfo->node[i].ucVail = 0x00;
				//gwinfo->node[i].info.ucOnlineState = 0x00;
				gwinfo->ucNodeNum -=1;
				n = (uint8 *)&gwinfo->node[i].info.ucNodeName;
				m = (uint8 *)&gwinfo->node[i].info.ucNodeID;
				GwPrint("del node=%d %s",i,gwinfo->node[i].info.ucNodeName);
				//memset(&gwinfo->node[i].info.ucNodeName,0,20);

				n[0] = ((m[2]&0xF0)>>4) + '0';
				n[1] =  (m[2]&0x0F) + '0';
				n[2] = ((m[3]&0xF0)>>4) + '0';
				n[3] =  (m[3]&0x0F) + '0';
				n[4] = ((m[4]&0xF0)>>4) + '0';
				n[5] =  (m[4]&0x0F) + '0';
				n[6] = ((m[5]&0xF0)>>4) + '0';
				n[7] =  (m[5]&0x0F) + '0';
				n[8] = ((m[6]&0xF0)>>4) + '0';
				n[9] =  (m[6]&0x0F) + '0';
				n[10] =((m[7]&0xF0)>>4) + '0';
				n[11] = (m[7]&0x0F) + '0';
				return true;
			}
			else
				return false;
		}
	}
	return false;
}

void ICACHE_FLASH_ATTR
user_set_deploymode(uint8 mode)
{
		if(mode>0 &&mode<=4)
		{
			uint32 modechanged = 0;
			if(mode == OUTMODE)
			{
				modechanged = 1;
				delay_flag = 1;
				delay_cnt = 0;
				deploy_led_output(1);

#ifdef PLUG_DEVICE
				user_plug_set_status(1);
#endif

				gw_changemode(OUTMODE);

			}
			else if(mode == INHOMEMODE)
			{
				modechanged = 1;
				delay_flag = 0;
				delay_cnt = 0;
				deploy_led_output(1);

#ifdef PLUG_DEVICE
				user_plug_set_status(1);
#endif
				gw_changemode(INHOMEMODE);

			}
			else if(mode== NIGHTMODE)
			{
				modechanged = 1;
				delay_flag = 0;
				delay_cnt = 0;
				deploy_led_output(1);

#ifdef PLUG_DEVICE
				user_plug_set_status(1);
#endif
				gw_changemode(NIGHTMODE);

			}
			else if(mode == CANCELMODE)
			{
				gw_changemode(CANCELMODE);
				yy_bf(SYS_UNDEPLOY);

#ifdef PLUG_DEVICE
				user_plug_set_status(0);
#endif
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
	os_printf("user_set_timemode 0x%x\n",mode);
	GW_SAVE_DELAY();
}

void ICACHE_FLASH_ATTR
gw_setdelay(uint8 i)
{
	switch (gwinfo->node[i].info.ucNodeType)
	{
	case DEV_CONTROLLER:
		//閿熸枻鎷烽敓鏂ゆ嫹閬ラ敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸澃鐚存嫹閿熸枻鎷烽敓浠嬫櫙妯″紡閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
		gwinfo->node[i].ucDelayStateInhome = 0x01;
		gwinfo->node[i].ucDelayStateNight = 0x01;
		gwinfo->node[i].ucDelayStateOut = 0x01;
		break;

	case DEV_MAGNETIC:
		gwinfo->node[i].ucDelayStateInhome = 0x00;
		gwinfo->node[i].ucDelayStateNight = 0x01;
		gwinfo->node[i].ucDelayStateOut = 0x01;
		break;

	case DEV_INFRARED_SENSOR:
		gwinfo->node[i].ucDelayStateInhome = 0x00;
		gwinfo->node[i].ucDelayStateNight = 0x00;
		gwinfo->node[i].ucDelayStateOut = 0x01;
		break;

	case DEV_SMOKE_SENSOR:
		//閿熸暀闈╂嫹涔熼敓瑙掕鎷烽敓鏉扮尨鎷烽敓鏂ゆ嫹閿熶粙鏅ā寮忛敓鏂ゆ嫹閿熺煫鍖℃嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
		gwinfo->node[i].ucDelayStateInhome = 0x01;
		gwinfo->node[i].ucDelayStateNight = 0x01;
		gwinfo->node[i].ucDelayStateOut = 0x01;
		break;

	case DEV_MAGNETIC_WINDOW:
		gwinfo->node[i].ucDelayStateInhome = 0x00;
		gwinfo->node[i].ucDelayStateNight = 0x01;
		gwinfo->node[i].ucDelayStateOut = 0x01;
		break;

	}
	gwinfo->node[i].info.ucOnlineState = 0x01;
	gwinfo->node[i].info.ucBindState = 0x01;
	switch (gwinfo->ucCurSceneMode)
	{
	case OUTMODE:
		gwinfo->node[i].info.ucDeployStatus = gwinfo->node[i].ucDelayStateOut;
		break;
	case NIGHTMODE:
		gwinfo->node[i].info.ucDeployStatus = gwinfo->node[i].ucDelayStateNight;
		break;

	case INHOMEMODE:
		gwinfo->node[i].info.ucDeployStatus = gwinfo->node[i].ucDelayStateInhome;
		break;
	case CANCELMODE:
		gwinfo->node[i].info.ucDeployStatus = 0;
		break;
	}
}

int ICACHE_FLASH_ATTR
gw_addrtem(Term_Inf *inf)
{
	int i;
	//閿熸枻鎷烽敓鍙柇闈╂嫹ID閿熻鍑ゆ嫹閿熺獤鎾呮嫹閿熸枻鎷烽敓鏂ゆ嫹
	for(i=0;i<NODE_TOTAL;i++)
	{
		if(0==memcmp(inf,gwinfo->node[i].info.ucNodeID,8))
		{
			gwinfo->node[i].info.ucOnlineState = 0x01;
			if(gwinfo->node[i].ucVail == 1)
				return false;
			else if(gwinfo->node[i].ucVail == 0)
			{
				gwinfo->ucNodeNum += 1;
				os_memcpy(gwinfo->node[i].info.ucNodeID,inf,(sizeof (Term_Inf)));
				gwinfo->node[i].ucVail = 1;
				gwinfo->node[i].info.ucOnlineState = 0x01;
				gw_setdelay(i);
				gwinfo->node[i].ulLastTime = system_get_rtc_time();

				return true;
			}
		}

	}
	for(i=0;i<NODE_TOTAL;i++)
	{
		if(gwinfo->node[i].ucVail == 0)
		{
			gwinfo->ucNodeNum += 1;
			os_memcpy(gwinfo->node[i].info.ucNodeID,inf,(sizeof (Term_Inf)));
			gwinfo->node[i].ucVail = 1;
			gwinfo->node[i].info.ucOnlineState = 0x01;
			gw_setdelay(i);

			gwinfo->node[i].ulLastTime = system_get_rtc_time();
			return true;
		}
	}
	return false;
}

int ICACHE_FLASH_ATTR
gw_settname(uint8 *p)
{
	int i;
	for(i=0;i<NODE_TOTAL;i++)
	{
		if(0==memcmp(p,gwinfo->node[i].info.ucNodeID,8))
		{
			os_memcpy(gwinfo->node[i].info.ucNodeName,&p[8],20);
			return true;
		}
	}
	return false;
}

int ICACHE_FLASH_ATTR
gw_setqj(uint8 *p)
{
	int i,j;
	uint8 m = p[0]; //閿熶粙鏅�
	uint8 n = p[1]; //閿熺Ц绔潻鎷烽敓鏂ゆ嫹
	p+=2;
	for(j=0;j<n;j++)
	{
		for(i=0;i<NODE_TOTAL;i++)
		{
			if(0==memcmp(p,gwinfo->node[i].info.ucNodeID,8))
			{
				if(m == 0x01)
					gwinfo->node[i].ucDelayStateOut = p[8];
				else if(m == 0x02)
					gwinfo->node[i].ucDelayStateNight = p[8];
				else if(m == 0x03)
					gwinfo->node[i].ucDelayStateInhome = p[8];
				/*else if(m == 0x04)
					gwinfo->node[i].*/

				p+=9;

				break;
			}
		}
	}

	gw_changemode(gwinfo->ucCurSceneMode);
	return true;
}

int ICACHE_FLASH_ATTR
gw_settimer(TaskParm *parm)
{
	int i = 0,j = 0,len = 0;
	uint8* timer = parm->p;
	GwPrint("CMD_SETTIMER\n");

	for(i= 0; i<TIMER_TOTAL; i++)
	{
		int week;
		if(parm->len-len != 0)
		{
			week = timer[len]-1;

			if(week > 6 )
				return false;

			memset(&gwinfo->timer_inf[week].timerdata_inf,0,sizeof(gwinfo->timer_inf[week].timerdata_inf));

			gwinfo->timer_inf[week].week = timer[len];
			gwinfo->timer_inf[week].timerday_number = timer[len+1];

			if(gwinfo->timer_inf[week].timerday_number > 10)
				return false;

			for(j = 0 ;j<gwinfo->timer_inf[week].timerday_number;j++)
			{
				gwinfo->timer_inf[week].timerdata_inf[j].hour=timer[len+2+j*3];
				gwinfo->timer_inf[week].timerdata_inf[j].min=timer[len+2+j*3+1];
				gwinfo->timer_inf[week].timerdata_inf[j].mode=timer[len+2+j*3+2];

//				os_printf("%d,%d,%d\n\r",gwinfo->timer_inf[week].timerdata_inf[j].hour,
//						gwinfo->timer_inf[week].timerdata_inf[j].min,gwinfo->timer_inf[week].timerdata_inf[j].mode);
			}
			for(j = gwinfo->timer_inf[week].timerday_number ;j<DAYTIMER_NUMBER_TOTAL;j++)
			{
				gwinfo->timer_inf[week].timerdata_inf[j].hour=0;
				gwinfo->timer_inf[week].timerdata_inf[j].min=0;
				gwinfo->timer_inf[week].timerdata_inf[j].mode=0;

//				os_printf("%d,%d,%d\n\r",gwinfo->timer_inf[week].timerdata_inf[j].hour,
//						gwinfo->timer_inf[week].timerdata_inf[j].min,gwinfo->timer_inf[week].timerdata_inf[j].mode);
			}
//			os_printf("%d,%d\n\r",gwinfo->timer_inf[week].week,gwinfo->timer_inf[week].timerday_number);
		}
		else
			break;
		len += 2+3*(gwinfo->timer_inf[week].timerday_number);
	}
	return true;
}

void ICACHE_FLASH_ATTR
gw_reporttimer(struct espconn *pconn,uint32 Number)
{

	uint8 num = 0;
	uint16 i=0,j = 0,len=0;
	uint8 *p;
	init_fddat(fd);
	fd->Number = Number;
	fd->Dir = PKT_CMD_ACK;
	fd->CMDID = T16(CMD_CHECKTIMER);

	p = (uint8 *)&fd->uCmdRsp.timer;

	for(i = 0;i<TIMER_TOTAL;i++)
	{
		if( gwinfo->timer_inf[i].timerday_number > 0 && gwinfo->timer_inf[i].timerday_number <= DAYTIMER_NUMBER_TOTAL)
		{
			num = gwinfo->timer_inf[i].timerday_number;

			os_memcpy(p,&gwinfo->timer_inf[i].week,1);
			os_memcpy(p+1,&gwinfo->timer_inf[i].timerday_number,1);
			len = len+2;
			p = p+2;
//			os_printf("LEN :%d,Num :%d\n\r",len,num);
			for(j=0; j < num ; j++)
			{
				os_memcpy(p,&gwinfo->timer_inf[i].timerdata_inf[j],3);
				p   +=3;
				len +=3;
			}
		}
	}

	fd->PacketLen = T16(PACK_SIZE(len));

	set_timestamp((TimeStamp_t *)p);

//	PktPrint(fd);

	espconn_send(pconn,(uint8*)fd,PACK_SIZE(len));
}

extern void gw_user_upgrade_begin(Updata_URL *p);
void ICACHE_FLASH_ATTR
tcp_rx_op(TaskParm *parm)
{
	uint16 CMDID = parm->CMDID;
	uint32 Number = parm->Number;
	if(CMDID <= CMD_START || CMDID >= CMD_END)
	{
		GwPrint("cmd not suport\n");
		return;
	}
	switch(CMDID)
	{
		case CMD_TIME:
			break;

		case CMD_RESET:
			gw_ackok(Number,CMD_RESET);
			break;

		case CMD_LOGIN:
			break;

		case CMD_TOKEN:
			if(parm->len == 1)
			{
				//姣忛敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓缁撴挱閿熻剼浼欐嫹杩庨敓鏂ゆ嫹
				uint8* p = parm->p;
				if(p[0] == 0x00 )
				{
					link_led_output(1);
					static int flag=0;
					if(flag==0)
					{
						flag = 1;
						if(get_rst_stats()==0)
							yy_bf(WELCOM_WORD);
					}
					gw_state = GW_WORKING;
				}

				if(jc_bf()!=0)
					deploy_led_output(1);
			}
			break;

		case CMD_PASSWORD:
			break;

		case CMD_ADDTERM:
			if(gwinfo->ucNodeNum >= NODE_TOTAL)
				gw_ackfail(Number,CMD_ADDTERM);
			else
			{
				if(gw_addrtem((Term_Inf *)(parm->p)))
				{
					gw_ackok(Number,CMD_ADDTERM);
					os_printf("addTerm!!!\n\r");
					tri_refresh();
				}
				else
					gw_ackfail(Number,CMD_ADDTERM);
			}
			break;

		case CMD_SCANTERM:
			break;

		case CMD_DELTERM:
			if(gw_deltem((uint8*)(parm->p)))
			{
				gw_ackok(Number,CMD_DELTERM);
				os_printf("delTerm!!!\n\r");
				tri_refresh();
			}
			else
				gw_ackfail(Number,CMD_DELTERM);

			break;

		case CMD_BATALARM:
			break;

		case CMD_ARM:
			if(parm->len == 1)
			{
				uint8* p = parm->p;
				os_printf("arm 0x%x\n",p[0]);
				user_set_deploymode(p[0]);
				gw_ackok(Number,CMD_ARM);
			}
			else
				if(gw_setqj(parm->p)==true)
				{
					gw_ackok(Number,CMD_ARM);
					//GW_SAVE_DELAY();
					os_printf("SetQj!!!\n\r");
					tri_refresh();
				}
				else
					gw_ackfail(Number,CMD_ARM);
			break;

		case CMD_REPORTARM:
			gw_reportarm(&gw_conn,Number);
			break;

		case CMD_REPORT:
			gw_report(&gw_conn,Number);
			break;

		case CMD_REPORTTERM:
			break;

		case CMD_ALARM:
			break;

		case CMD_OFFLINE:
			break;

		case CMD_SETNAME:
			os_memcpy(gwinfo->ucGwName,parm->p,20);

			GwPrint("Set Name is %s\n",gwinfo->ucGwName);

			gw_ackok(Number,CMD_SETNAME);
			tri_refresh();
			break;

		case CMD_SETTERMNAME:
			gw_settname(parm->p);
			gw_ackok(Number,CMD_SETTERMNAME);
			os_printf("Set TermName!!!\n\r");
			tri_refresh();
			break;

		case CMD_SENDTOTERM	:
			break;

		case CMD_RELIEVEALARM:
			gw_ackok(Number,CMD_RELIEVEALARM);
			break;

		case CMD_RELIEVEARM:
			gw_changemode(CANCELMODE);
			yy_bf(SYS_UNDEPLOY);
			deploy_led_output(0);
#ifdef PLUG_DEVICE
				user_plug_set_status(0);
#endif
			gw_ackok(Number,CMD_RELIEVEARM);
			GW_SAVE_DELAY();
			break;

		case CMD_UPDATING:
			gw_ackok(Number,CMD_UPDATING);
			yy_bf(UPDATING);
			link_led_output(0);
			deploy_led_output(0);
			gw_user_upgrade_begin((Updata_URL *)parm->p);
			break;

		case CMD_VERNUM:
			gw_vernum(&gw_conn,Number);
			break;

		case CMD_SETTIMER:
			if(gw_settimer(parm)==true)
			{
				gw_ackok(Number,CMD_SETTIMER);
				os_printf("SetTimer Over!\n\r");
				tri_refresh();
			}
			else
				gw_ackfail(Number,CMD_SETTIMER);
			break;

		case CMD_CHECKTIMER:
			gw_reporttimer(&gw_conn,Number);
			break;

		default :
			break;
	}
	GwPrint("paylen = %d\n",parm->len);
		os_free(parm->p);
	if(parm!=NULL)
	os_free(parm);
}

void ICACHE_FLASH_ATTR
tri_reset(void)
{
	if(!reset_flag)
		system_os_post(gw_procTaskPrio,SIG_RESET, 0);
}

//閿熸枻鎷蜂綅閿熸枻鎷峰涔嬮敓鏂ゆ嫹閿熻鐧告嫹閿熻緝顫嫹閿熸帴锟�
void ICACHE_FLASH_ATTR
tri_reset_key(void)
{
	if(!reset_flag)
		system_os_post(gw_procTaskPrio,SIG_KEY_RESET, 0);
}

void ICACHE_FLASH_ATTR
tri_key(void)
{
	if(!reset_flag)
		system_os_post(gw_procTaskPrio,SIG_KEY, 0);
}

void ICACHE_FLASH_ATTR
tri_key_long(void)
{
	if(!reset_flag)
		system_os_post(gw_procTaskPrio,SIG_KEY_LONG, 0);
}

void ICACHE_FLASH_ATTR
tri_rf_recv(uint8* buf)
{
	if(!reset_flag)
		system_os_post(gw_procTaskPrio,SIG_RF_REC, (os_param_t)buf);
}
void ICACHE_FLASH_ATTR
tri_gw_disconn()
{

	if(!reset_flag)
		system_os_post(gw_procTaskPrio,SIG_GW_DISCONN, 0);
}

void ICACHE_FLASH_ATTR
yy_start(uint8 cnt,uint8 * nr)
{
	uint8 buf[20+2];
	uint8 i;
	alarm_cnt=0;
	buf[0] = 0x55;
	buf[1] = cnt;
	for(i=0;i<cnt;i++)
		buf[i+2] = nr[i];
	uart0_tx_buffer(buf,cnt+2);
}

void ICACHE_FLASH_ATTR
yy_changebat(uint8 type0)	//REPLACE_BETTERY
{
	//uint8 i;
	uint8 buf[2];
	switch (type0)
	{
		case DEV_CONTROLLER:
			buf[0] = REMOTE_CTRL;
		break;
		case DEV_MAGNETIC:
			buf[0] = MAGNETIC_SENSOR;
		break;
		case DEV_INFRARED_SENSOR:
			buf[0] = INFRARED_DETECTOR;
		break;
		case DEV_SMOKE_SENSOR:
			buf[0] = SMOKE_DETECTOR;
		break;
		case DEV_MAGNETIC_WINDOW:
			buf[0] = WINDOW_DETECTOR;
		break;
	}

	if(type0>=DEV_CONTROLLER && type0<=DEV_MAGNETIC_WINDOW)
	{
		buf[1] = REPLACE_BETTERY;
		yy_start(2,buf);
	}

}

#if 0
			if((gwinfo->node[i].info.ucAlmStatus & 0xf0) && (gwinfo->node[i].info.ucNodeType == DEV_SMOKE_SENSOR))
			{
				yy_bf(SOS);
				AppPrint("fire\n");
				break;
			}
			if((gwinfo->node[i].info.ucAlmStatus & 0xf0) && (gwinfo->node[i].info.ucNodeType == DEV_CONTROLLER))
			{
				yy_bf(EMERGENCY_CALL);
				AppPrint("EMERGENCY_CALL\n");
				break;
			}
			if((gwinfo->node[i].info.ucAlmStatus & 0xf0) && (gwinfo->node[i].info.ucNodeType == DEV_MAGNETIC))
			{
				yy_bf(SOMEBODY_INTO);
				AppPrint("SOMEBODY_INTO\n");
				break;
			}
			if((gwinfo->node[i].info.ucAlmStatus & 0xf0) && (gwinfo->node[i].info.ucNodeType == DEV_INFRARED_SENSOR))
			{
				yy_bf(SOMEBODY_INTO);
				AppPrint("SOMEBODY_INTO\n");
				break;
			}
			if((gwinfo->node[i].info.ucAlmStatus & 0xf0) && (gwinfo->node[i].info.ucNodeType == DEV_MAGNETIC_WINDOW))
			{
				yy_bf(SOMEBODY_INTO);
				AppPrint("SOMEBODY_INTO\n");
				break;
			}
			if(gwinfo->node[i].info.ucAlmStatus & 0x08)
			{
				yy_changebat(gwinfo->node[i].info.ucNodeType);
				AppPrint("REPLACE_BETTERY\n");
				break;
			}
#endif


//寰敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鎭�
void ICACHE_FLASH_ATTR
alarm_handler(void)
{
	int i;
	uint8 type0 = -1;
	uint8 alm_type = 0;
	uint8 low_flag = 0 ,low_cnt = 0;
	for(i = 0;i<NODE_TOTAL;i++)
	{
		if(0x01 == gwinfo->node[i].ucVail)
		{
			if(gwinfo->node[i].info.ucAlmStatus & 0x01)
			{
				switch (gwinfo->node[i].info.ucNodeType)
				{
					case DEV_SMOKE_SENSOR:
						//閿熸枻鎷烽敓鍙鎷烽敓浠嬫櫙妯″紡閿熼摪璁规嫹瑕侀敓鏂ゆ嫹
						alm_type |= 1<<ALM_FIRE;
						break;
					case DEV_CONTROLLER:
						alm_type |= 1<<ALM_CALL;
						break;
					case DEV_MAGNETIC:
						if(gwinfo->node[i].info.ucDeployStatus && delay_flag == 0)
							alm_type |= 1<<ALM_INTO;
						break;
					case DEV_INFRARED_SENSOR:
						if(gwinfo->node[i].info.ucDeployStatus && delay_flag == 0)
							alm_type |= 1<<ALM_INTO;
						break;
					case DEV_MAGNETIC_WINDOW:
						if(gwinfo->node[i].info.ucDeployStatus)
							alm_type |= 1<<ALM_WINOPEN;
						break;
				}
			}
			if(gwinfo->node[i].ucLastVal & 0x08)
			{
				//if(gwinfo->node[i].info.ucDeployStatus)
				{
					alm_type |= 1<<ALM_LOWVOL;
					type0 = gwinfo->node[i].info.ucNodeType;
					low_cnt = i;
					low_flag = 1;
					gwinfo->node[i].ucLastVal &= 0xf0;
				}

			}

		}
	}
	if(low_flag!=0)
	{
		gw_alarm_send((Term_Inf *)&gwinfo->node[low_cnt].info,DEV_VOL_LOW_ALM);
		yy_changebat(type0);
	}
	if(alm_type & 1<<ALM_FIRE)
	{
		yy_bf(FIRE_SOS_1);
		//yy_fire();
	}
	else if(alm_type & 1<<ALM_CALL)
	{
		yy_bf(EMERGENCY_CALL);
	}
	else if(alm_type & 1<<ALM_INTO)
	{
		yy_bf(SOMEBODY_INTO);
	}
	else if(alm_type & 1<<ALM_WINOPEN)
	{
		yy_bf(NOT_CLOSED_WINDOW);
	}
	alm_type = 0;
}
void ICACHE_FLASH_ATTR
alarm_clear(void)
{
	int i;
	for(i = 0;i<NODE_TOTAL;i++)
	{
		gwinfo->node[i].info.ucAlmStatus =0;
	}

}

//閿熸枻鎷烽敓鏂ゆ嫹涓洪敓鏂ゆ嫹閿熸枻鎷�
volatile uint32 ttb = 0;
uint32 node_tm[NODE_TOTAL];
void ICACHE_FLASH_ATTR
tonline_check(void)
{
	int i;
	uint32 cur_time = ttb;
	for(i = 0;i<NODE_TOTAL;i++)
	{
		if(gwinfo->node[i].ucVail == 0x01  && gwinfo->node[i].info.ucOnlineState == 0x01 && gwinfo->node[i].info.ucNodeType!=DEV_CONTROLLER_ETH)
		{
			uint32 onlinetime = 0;
			if(cur_time < node_tm[i])
			{
				onlinetime = (0xffffffff - node_tm[i] + cur_time);
				os_printf("1 onlinetime = %u cur_time = %u LastTime = %u\n",onlinetime,cur_time,node_tm[i]);
			}
			else
			{
				onlinetime = (cur_time - node_tm[i]);
				os_printf("2 onlinetime = %u cur_time = %u LastTime = %u\n",onlinetime,cur_time,node_tm[i]);
			}

			//os_printf("node%d on line time = %d",i,onlinetime);
			if(onlinetime > offline_time)
			{
				gwinfo->node[i].info.ucOnlineState = 0x00;

				gw_alarm_send((Term_Inf *)&gwinfo->node[i].info,DEV_OFFLINE_ALM);
				os_printf("type = %d,online time = %d offline\n",gwinfo->node[i].info.ucNodeType,onlinetime);
				break;
			}

			//else
			//	gwinfo->node[i].info.ucOnlineState = 0x01;

		//	os_printf("type = %d,online time = %d\n",gwinfo->node[i].info.ucNodeType,onlinetime);
		}
	}
}
volatile uint8 hb_flag = 0;
uint32 pre_tm = 0;
void ICACHE_FLASH_ATTR
update_tm(void)
{
	uint32 tm = system_get_time();
	if(pre_tm>tm)
	{
		ttb += (0xffffffff-pre_tm+tm)/1000;
	}
	else
	{
		ttb += (tm - pre_tm)/1000;
	}
	pre_tm = tm;
}

void ICACHE_FLASH_ATTR
timer_handler(void)
{
	//鍚岄敓鏂ゆ嫹閿熸枻鎷锋伅閿熸枻鎷烽敓鏂ゆ嫹鍙敓鏂ゆ嫹搴斾竴閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷�
	os_timer_disarm(&check_timer);
	update_tm();

	if(hb_cnt++ == 2*60*2)	//2min
	{
		hb_cnt=0;
		hb_flag = 1;
		system_os_post(gw_procTaskPrio,SIG_BEAT, 0);
		goto out;
	}
	if(hb_cnt > 60 && hb_flag == 1)		//10s 閿熸枻鎷烽敓鏂ゆ嫹涔嬮敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熺煫浼欐嫹鐩忛敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰箷棣楅敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
	{
		os_printf("No Heat Reset! \n");
		if(!reset_flag)
			system_os_post(gw_procTaskPrio,SIG_RESET, 0);
	}
	if(state_cnt++ == 15*2)	//15s
	{
		state_cnt = 0;
		if(gw_state == GW_ADD)
		{
			AppPrint("out of add\n");
			yy_bf(ADD_OUT);
		}
		else if(gw_state == GW_DEL)
		{
			AppPrint("out of del\n");
			yy_bf(DEL_OUT);
		}
		gw_state = GW_WORKING;
		goto out;
	}

	if(alarm_cnt++ == 3*2 && gw_state == GW_WORKING)	//寰敓鏂ゆ嫹5s閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷锋伅
	{
		alarm_cnt=0;
		system_os_post(gw_procTaskPrio,SIG_ALARM_HANDLER, 0);
//		alarm_handler();
		goto out;
	}

	if(tonline_cnt++ == 30*2)	//閿熸枻鎷峰崄閿熸枻鎷烽敓鏂ゆ嫹涓�閿熸枻鎷�
	{
		tonline_cnt=0;
		system_os_post(gw_procTaskPrio,SIG_TONLINE_CHECK, 0);
//		tonline_check();
		goto out;

	}

	if(online_cnt++ == 10*2)	//10s閿熸枻鎷烽敓鎻紮鎷烽敓锟�
	{
		//閿熸枻鎷穉p閿熸枻鎷烽敓鏂ゆ嫹閿熷眾甯告椂
		static uint8 pre_sta = STATION_GOT_IP;
		uint8 sta = wifi_station_get_connect_status();
		//os_printf("app check AP\n");
		if(sta == STATION_GOT_IP)
		{
			if(gw_state == GW_INIT)
			{
				os_printf("app check ap gw_reconn\n",sta);
				tri_reset();
			}
		}
		else if(sta==STATION_IDLE || sta==STATION_NO_AP_FOUND || sta==STATION_WRONG_PASSWORD || sta==STATION_CONNECT_FAIL || sta == STATION_CONNECTING)
		{
			gw_state = GW_INIT;
			os_printf("app check NO AP\n");
			tri_reset();
		}
		online_cnt=0;
		goto out;
	}
	if(delay_flag == 1)
	{
		delay_cnt ++;
		if(delay_cnt == 60*2)	//閿熸枻鎷锋椂涓�閿熸枻鎷烽敓鍙枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓渚ワ吉锟�
		{
			int i;
			GwPrint("no delay out mode\n");
			delay_cnt = 0;
			delay_flag = 0;
			if(jc_mc1(&i)!=0)
			{
				system_os_post(gw_procTaskPrio,SIG_DOOR_OPEN_ALM,i);
//				gw_alarm_send((Term_Inf *)&gwinfo->node[i].info,DOOR_OPEN_ALM);
			}
			goto out;
		}
	}
	if(save_flag)
	{
		save_cnt++;
		if(save_cnt == 60*2)	//閿熸枻鎷锋椂涓�閿熸枻鎷烽敓鎺ユ唻鎷烽敓鏂ゆ嫹
		{
			save_cnt = 0;
			save_flag = 0;
			tri_refresh();
		}
	}
out:
	os_timer_arm(&check_timer, 500, 0);
}


void ICACHE_FLASH_ATTR
key_op(void)
{
	state_cnt = 0;
	if(gw_state == GW_WORKING)
	{
		gw_state = GW_ADD;
	}
	else if(gw_state == GW_ADD)
	{
		gw_state = GW_DEL;
	}
	else if(gw_state == GW_DEL)
	{
		gw_state = GW_ADD;
	}

	if(gw_state == GW_ADD)
	{
		yy_bf(ADD);
		GwPrint("into add\n");
	}
	else if(gw_state == GW_DEL)
	{
		yy_bf(DELETE);
		GwPrint("into del\n");
	}
}


void ICACHE_FLASH_ATTR
yy_xlh(uint8 *xlh)	//xlh 閿熸枻鎷�6閿熸枻鎷烽敓琛楄妭纰夋嫹ID閿熸枻鎷� 閿熸枻鎷烽敓鏂ゆ嫹鍙閿熸枻鎷蜂竴閿熸枻鎷烽敓琛楁枻鎷烽敓鏂ゆ嫹閿熷�熷閿熸枻鎷烽敓鏂ゆ嫹
{
	uint8 i;
	uint8 buf[11];
	uint8 type0;
	type0 = xlh[0]&0xf0;
	type0 = type0 >>4;

	alarm_cnt=0;//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸埅纰夋嫹

	os_printf("xlh: ");
	for(i=0;i<6;i++)
		os_printf("%02x ",xlh[i]);
	switch (type0)
	{
		case DEV_CONTROLLER:
			buf[0] = REMOTE_CTRL;
		break;
		case DEV_MAGNETIC:
		case DEV_MAGNETIC_WINDOW:
			buf[0] = MW_SENSOR;
		break;
		case DEV_INFRARED_SENSOR:
			buf[0] = INFRARED_DETECTOR;
		break;
		case DEV_SMOKE_SENSOR:
			buf[0] = SMOKE_DETECTOR;
		break;
		//case DEV_MAGNETIC_WINDOW:
		//	buf[0] = WINDOW_DETECTOR;
		//break;
	}
	buf[1] = ((xlh[1] & 0xF0)>>4) + ZERO;//yy 0 == 0x0f
	buf[2] = ( xlh[1] & 0x0F) + ZERO;
	buf[3] = ((xlh[2] & 0xF0)>>4) + ZERO;
	buf[4] = ( xlh[2] & 0x0F) + ZERO;
	buf[5] = ((xlh[3] & 0xF0)>>4) + ZERO;
	buf[6] = ( xlh[3] & 0x0F) + ZERO;
	buf[7] = ((xlh[4] & 0xF0)>>4) + ZERO;
	buf[8] = ( xlh[4] & 0x0F) + ZERO;
	buf[9] = ((xlh[5] & 0xF0)>>4) + ZERO;
	buf[10] =( xlh[5] & 0x0F) + ZERO;

	GwPrint("type = %d ",type0);


	if(type0>=DEV_CONTROLLER && type0<=DEV_MAGNETIC_WINDOW)
	{
		yy_start(11,buf);
	}

}

int ICACHE_FLASH_ATTR
rf_get_rssi(RFS * rf)
{
	uint8 dB_z = rf->db;
	int t;
	if(dB_z>=128)
	{
		t = dB_z - 256;
		t = t/2;
		t = t - 74;
	}
	else if(dB_z<128)
	{
		t = dB_z/2;
		t = t - 74;
	}

	return t;
}

//閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鎴潻鎷烽敓鏂ゆ嫹鍙板崗閿熸枻鎷烽敓鍊熷ID閿熸枻鎷烽敓鏂ゆ嫹8閿熸枻鎷烽敓琛楄妭锝忔嫹閿熸枻鎷烽敓绉歌鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鎴鎷峰崗閿熸枻鎷稩D閿熸枻鎷烽敓鏂ゆ嫹6閿熸枻鎷烽敓鐨嗙》鎷�
int ICACHE_FLASH_ATTR
rf_addterm(Term_Inf * inf)
{
	inf->term_name[0] = ((inf->term_n[2]&0xF0)>>4) + '0';
	inf->term_name[1] =  (inf->term_n[2]&0x0F) + '0';
	inf->term_name[2] = ((inf->term_n[3]&0xF0)>>4) + '0';
	inf->term_name[3] =  (inf->term_n[3]&0x0F) + '0';
	inf->term_name[4] = ((inf->term_n[4]&0xF0)>>4) + '0';
	inf->term_name[5] =  (inf->term_n[4]&0x0F) + '0';
	inf->term_name[6] = ((inf->term_n[5]&0xF0)>>4) + '0';
	inf->term_name[7] =  (inf->term_n[5]&0x0F) + '0';
	inf->term_name[8] = ((inf->term_n[6]&0xF0)>>4) + '0';
	inf->term_name[9] =  (inf->term_n[6]&0x0F) + '0';
	inf->term_name[10] =((inf->term_n[7]&0xF0)>>4) + '0';
	inf->term_name[11] = (inf->term_n[7]&0x0F) + '0';

	if(gw_addrtem(inf) == true)
	{
		yy_xlh(&inf->term_n[2]);
		tri_refresh();
		GwPrint("add success\n");
	}
	else
	{
		GwPrint("term has added\n");
		yy_bf(NODE_ADD);
	}
}

int ICACHE_FLASH_ATTR
rf_delterm(Term_Inf * inf)
{
	if(gw_deltem(&inf->term_n[0]) == true)
	{
		yy_xlh(&inf->term_n[2]);
		tri_refresh();
		GwPrint("del success\n");
	}
	else
	{
		yy_bf(NODE_DEL);
		GwPrint("term has del\n");
	}
}

int ICACHE_FLASH_ATTR
jc_cc(void)
{
	int i;
	for(i = 0;i<NODE_TOTAL;i++)
	{
		if(0x01 == gwinfo->node[i].ucVail)
		{
			switch (gwinfo->node[i].info.ucNodeType)
			{

				case DEV_MAGNETIC:
				case DEV_MAGNETIC_WINDOW:
					if((gwinfo->node[i].ucLastVal & 0xf0)&&(gwinfo->node[i].info.ucDeployStatus == 0x01))
					{
						return -1;
					}
				break;
			}


		}
	}
	return 0;

}

int ICACHE_FLASH_ATTR
jc_mc1(int *pi)
{
	int i;
	for(i = 0;i<NODE_TOTAL;i++)
	{
		if(0x01 == gwinfo->node[i].ucVail)
		{
			switch (gwinfo->node[i].info.ucNodeType)
			{

				case DEV_MAGNETIC:
				case DEV_MAGNETIC_WINDOW:
					if(gwinfo->node[i].ucLastVal & 0xf0)
					{
						*pi = i;
						return -1;
					}
				break;
			}


		}
	}
	return 0;

}

int ICACHE_FLASH_ATTR
chk_AlmStatus(void)
{
	int i;
	for(i = 0;i<NODE_TOTAL;i++)
	{
		if(0x01 == gwinfo->node[i].ucVail)
		{
			if(gwinfo->node[i].info.ucAlmStatus)
			{
				return 1;
			}
		}
	}
	return 0;
}

#define MDTIME	87500*2//29471
#define HDTIME	65000*2//59373
#define YDTIME	243379*2//52284	233379


void ICACHE_FLASH_ATTR
rf_op(uint8* buf)
{
	RFS * rf = (RFS * )buf;
	Term_Inf tinf;
	int i;

	uint8 bf_flag = 1;
	os_memset(&tinf,0,sizeof(Term_Inf));
	os_memcpy(&tinf.term_n[2],(char *)&rf->type,6);

	tinf.term_type = rf->type & 0xf0;
	tinf.term_type = tinf.term_type >> 4;

	int db = rf_get_rssi(rf);
//	GwPrint("rf_parse %ddB\r\n",db);

	if(db<-110)
		goto out;

	for(i=0;i<NODE_TOTAL;i++)
	{
		if(0==memcmp(rf,&(gwinfo->node[i].info.ucNodeID[2]),6))
		{
			uint32 modechanged = 0;
			uint32 currtime = system_get_rtc_time();//system_get_time();
			os_printf("currtime = 0x%x\n",currtime);
			uint32 dtime = currtime - gwinfo->node[i].ulLastTime;
			TidPrint((uint8*)rf);
			os_printf("dtime = %d\n",dtime);
			gwinfo->node[i].ulLastTime = currtime;
			node_tm[i] = ttb;
			if(gwinfo->node[i].ucLastVal != rf->z)
				gwinfo->node[i].ucLastVal = rf->z;

			if(gwinfo->node[i].info.ucOnlineState == 0x00/* && gwinfo->node[i].ucVail == 1*/)
			{
				gwinfo->node[i].info.ucOnlineState = 0x01;
				gw_alarm_send((Term_Inf *)&gwinfo->node[i].info,DEV_ONLINE_ALM);
				goto out;
			}
			gwinfo->node[i].info.ucOnlineState = 0x01;

			switch (gwinfo->node[i].info.ucNodeType)
			{
			//case DEV_CONTROLLER:
			//	break;
			case DEV_MAGNETIC:
				if(dtime < MDTIME)
					bf_flag = 0;//goto out;
				break;
			case DEV_INFRARED_SENSOR:
				if(dtime < HDTIME)
					bf_flag = 0;//goto out;
				break;
			case DEV_SMOKE_SENSOR:
				if(dtime < YDTIME)
					bf_flag = 0;//goto out;
				break;
			case DEV_MAGNETIC_WINDOW:
				if(dtime < MDTIME)
					bf_flag = 0;//goto out;
				break;
			}

			if(gw_state == GW_WORKING && gwinfo->node[i].ucVail == 1)
			{
				uint8 type = gwinfo->node[i].info.ucNodeType;

				switch (type)
				{

				case DEV_CONTROLLER:
					if(rf->z == 0x01)
					{
						modechanged = 1;
						delay_flag = 1;
						delay_cnt = 0;
						deploy_led_output(1);
#ifdef PLUG_DEVICE
				user_plug_set_status(1);
#endif
						gw_changemode(OUTMODE);
					}
					else if(rf->z == 0x02)
					{
						modechanged = 1;
						delay_flag = 0;
						delay_cnt = 0;
						deploy_led_output(1);
#ifdef PLUG_DEVICE
				user_plug_set_status(1);
#endif
						gw_changemode(INHOMEMODE);

					}
					else if(rf->z == 0x03)
					{
						modechanged = 1;
						delay_flag = 0;
						delay_cnt = 0;
						deploy_led_output(1);
#ifdef PLUG_DEVICE
				user_plug_set_status(1);
#endif
						gw_changemode(NIGHTMODE);

					}
					else if(rf->z == 0x04)
					{
						system_os_post(gw_procTaskPrio,SIG_CANCLE_ARM, 0);
						if(chk_AlmStatus()!=0)
						{
							//璇撮敓鏂ゆ嫹閿熸枻鎷峰墠閿熷彨鎲嬫嫹閿熸枻鎷�
							delay_flag = 0;
							delay_cnt = 0;
							alarm_clear();
						}
						else
						{
							gwinfo->ucCurSceneMode = CANCELMODE;
							gw_changemode(CANCELMODE);
							yy_bf(SYS_UNDEPLOY);

							deploy_led_output(0);
#ifdef PLUG_DEVICE
				user_plug_set_status(0);
#endif
						}
						gwinfo->node[i].info.ucAlmStatus = 0;
					}
					else if((rf->z & 0xf0) == 0xf0)
					{
						gwinfo->node[i].info.ucAlmStatus = 0x01;
						yy_bf(EMERGENCY_CALL);
						gw_alarm_send((Term_Inf *)&gwinfo->node[i].info,DEV_SOS);

					}

					if(modechanged == 1)
					{
						modechanged = 0;
						GW_SAVE_DELAY();
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
							//gw_changemode(OUTMODE);
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
					break;

				case DEV_MAGNETIC:
					if((rf->z & 0xf0) == 0xf0 && gwinfo->node[i].info.ucDeployStatus == 0x01)
					{

						if(delay_flag == 0)
						{
							gwinfo->node[i].info.ucAlmStatus = 0x01;
							if(bf_flag!=0)
							{
								yy_bf(SOMEBODY_INTO);
								gw_alarm_send((Term_Inf *)&gwinfo->node[i].info,DOOR_OPEN_ALM);
							}
						}
						else
						{
							gwinfo->node[i].info.ucAlmStatus = 0x00;
						}


					}
					else
						gwinfo->node[i].info.ucAlmStatus = 0x00;
					break;
				case DEV_INFRARED_SENSOR:
					if((rf->z & 0xf0) == 0xf0 && gwinfo->node[i].info.ucDeployStatus == 0x01)
					{
						if(delay_flag == 0)
						{
							gwinfo->node[i].info.ucAlmStatus = 0x01;
							if(bf_flag!=0)
							{
								yy_bf(SOMEBODY_INTO);
								gw_alarm_send((Term_Inf *)&gwinfo->node[i].info,INFRARED_ALM);
							}
						}
						else
						{
							gwinfo->node[i].info.ucAlmStatus = 0x00;
						}
					}
					else
						gwinfo->node[i].info.ucAlmStatus = 0x00;
					break;

				case DEV_SMOKE_SENSOR:
					if((rf->z & 0xf0) == 0xf0)
					{
						gwinfo->node[i].info.ucAlmStatus = 0x01;
						if(bf_flag!=0)
						{
							yy_bf(FIRE_SOS_1);
							gw_alarm_send((Term_Inf *)&gwinfo->node[i].info,SMOKE_ALM);
						}
					}
					else
						gwinfo->node[i].info.ucAlmStatus = 0x00;
					break;

				case DEV_MAGNETIC_WINDOW:
					if((rf->z & 0xf0) == 0xf0 && gwinfo->node[i].info.ucDeployStatus == 0x01)
					{
						if(delay_flag == 0)
						{
							gwinfo->node[i].info.ucAlmStatus = 0x01;
							if(bf_flag!=0)
							{yy_bf(SOMEBODY_INTO);
							gw_alarm_send((Term_Inf *)&gwinfo->node[i].info,DOOR_OPEN_ALM);}
						}
						else
						{
							gwinfo->node[i].info.ucAlmStatus = 0x00;
						}

					}
					else
						gwinfo->node[i].info.ucAlmStatus = 0x00;
					break;
				}


				GwPrint("RF %0x\n",rf->z);
				goto out;
			}
			else if(gw_state == GW_ADD && db > -60)
			{//閿熸枻鎷烽敓鍊熷閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷峰焊閿燂拷
				state_cnt = 0;
				gwinfo->node[i].info.ucOnlineState = 0x01;
				if(gwinfo->node[i].ucVail == 1 && bf_flag != 0)
				{
					//GwPrint("term has added\n");
					yy_bf(NODE_ADD);
				}
				else if(gwinfo->node[i].ucVail == 0)
				{
					gwinfo->node[i].ucVail = 1;
					gwinfo->ucNodeNum +=1;
					yy_xlh(&tinf.term_n[2]);
				}
				goto out;
			}
			else if(gw_state == GW_DEL && db > -60)
			{
				state_cnt = 0;
				if(gwinfo->node[i].ucVail == 1)
				{
					uint8 * n;
					uint8 *  m;
					gwinfo->node[i].ucVail = 0x00;
					//gwinfo->node[i].info.ucOnlineState = 0x00;
					gwinfo->ucNodeNum -=1;
					n = (uint8 *)&gwinfo->node[i].info.ucNodeName;
					m = (uint8 *)&gwinfo->node[i].info.ucNodeID;
					GwPrint("del node=%d %s",i,gwinfo->node[i].info.ucNodeName);
					n[0] = ((m[2]&0xF0)>>4) + '0';
					n[1] =  (m[2]&0x0F) + '0';
					n[2] = ((m[3]&0xF0)>>4) + '0';
					n[3] =  (m[3]&0x0F) + '0';
					n[4] = ((m[4]&0xF0)>>4) + '0';
					n[5] =  (m[4]&0x0F) + '0';
					n[6] = ((m[5]&0xF0)>>4) + '0';
					n[7] =  (m[5]&0x0F) + '0';
					n[8] = ((m[6]&0xF0)>>4) + '0';
					n[9] =  (m[6]&0x0F) + '0';
					n[10] =((m[7]&0xF0)>>4) + '0';
					n[11] = (m[7]&0x0F) + '0';
					yy_xlh(&tinf.term_n[2]);
					GwPrint("term has del\n");
				}
				else if(gwinfo->node[i].ucVail == 0  && bf_flag != 0)
				{
					yy_bf(NODE_DEL);
					GwPrint("term has del\n");
				}
				goto out;
			}
			else
				goto out;
		}
	}
	//閿熸枻鎷烽敓鍊熷閿熸枻鎷锋湭閿熸枻鎷峰焊閿燂拷
	if(gw_state == GW_ADD)
	{
		if(db > -60)
		{
			state_cnt = 0;
			rf_addterm(&tinf);
		}

	}
out:
	os_free(buf);
}
void ICACHE_FLASH_ATTR
delay_reset_timer(void *arg)
{
	system_restart();
	reset_flag=0;
}

extern void user_esp_platform_clear_param(void);

static void ICACHE_FLASH_ATTR
gw_procTask(os_event_t *events)
{
	//GwPrint("gw_procTask ");
//	TaskParm * par = (TaskParm *)events->par;
	switch(events->sig)
	{
		case SIG_RESET:
			reset_flag = 1;
			tri_refresh();
			os_printf("SIG_RESET\n\r");
			os_timer_disarm(&check_timer);
			os_timer_setfn(&check_timer, (os_timer_func_t *)delay_reset_timer, &gw_conn);
			os_timer_arm(&check_timer, 500, 0);
			break;

		case SIG_TCP_RX:
			GwPrint("SIG_TCP_RX\n");
			tcp_rx_op((TaskParm *)events->par);
			break;

		case SIG_KEY:
			GwPrint("key_op\n");
			key_op();
			break;

		case SIG_KEY_RESET:
			GwPrint("key_reset\n");
			system_restore();
			os_memset(gwinfo,0, sizeof(GWCFGINFOSTRU));
			user_esp_platform_clear_param();
			GwPrint("GW RESET!!!!\n\r");
			tri_reset();
			/*reset_flag = 1;
			set_ap_config(UNCONF);
			os_timer_disarm(&check_timer);
			os_timer_setfn(&check_timer, (os_timer_func_t *)delay_reset_timer, &gw_conn);
			os_timer_arm(&check_timer, 1000, 0);*/
			break;

		case SIG_KEY_LONG:
			GwPrint("long key\n");
			system_restore();

			reset_flag = 1;
			set_ap_config(UNCONF);
		    os_timer_disarm(&check_timer);
		    os_timer_setfn(&check_timer, (os_timer_func_t *)delay_reset_timer, &gw_conn);
		    os_timer_arm(&check_timer, 500, 0);

			break;

		case SIG_RF_REC:
			//GwPrint("RF REC\n");
			rf_op((uint8 *)events->par);
			break;

		case SIG_GW_DISCONN:

			//gw_reconn();
			//閿熸枻鎷烽敓鏂ゆ嫹鐛炬枻鎷锋閿熸枻鎷峰彥閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
			GwPrint("GW DISCON!!!\n\r");   //閿熸枻鎷烽敓鏂ゆ嫹閿熸帴鐨勶綇鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
			tri_reset();
			break;

		case SIG_HEART_BEAT:
			timer_handler();
			break;

/************4/5濞ｈ濮炵涵顑挎鐎规碍妞傞崳銊ф畱post閸濆秴绨�*********/
		case SIG_BEAT:
			heart_beat();
			break;

/************4/7濞ｈ濮炴潪顖欐鐎规碍妞傞崳銊ф畱post閸濆秴绨�*********/
		case SIG_ALARM_HANDLER:
			alarm_handler();
			break;

		case SIG_TONLINE_CHECK:
			tonline_check();
			break;

		case SIG_DOOR_OPEN_ALM:
			gw_alarm_send((Term_Inf *)&gwinfo->node[(int)events->par].info,DOOR_OPEN_ALM);
			break;

		case SIG_GET_NTP_TIME:
//			user_get_ntp_time();
			break;

		case SIG_CANCLE_ARM:
			gw_alarm_cancle();
			break;

		default:
			break;
	}
}

//閿熸枻鎷烽敓鏂ゆ嫹鐜敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鑾呴敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熻顕嗘嫹锠撶シ涓堕敓鏂ゆ嫹閿燂拷
void ICACHE_FLASH_ATTR
packet_op(FrameHeader * packet)
{
	FrameHeader * rsp = packet;
	uint16 CMDID = T16(rsp->CMDID);
	uint16 pack_len = T16(rsp->PacketLen);
	uint16 payload_len = PAYLOAD_SIZE(pack_len);
	PktPrint(packet);
	//static volatile uint32 Number = 0;
	int i;
	TaskParm *parm;
	uint8 * p = (uint8 *)rsp;
	GwPrint("%02d-%02d %02d:%02d:%02d ",p[pack_len-5],p[pack_len-4],p[pack_len-3],p[pack_len-2],p[pack_len-1]);
	GwPrint("Number=0x%x packet_len= %d CMDID=0x%04x\r\n",rsp->Number,pack_len,CMDID);

	if(CMDID <= CMD_START || CMDID >= CMD_END)
	{
		GwPrint("cmd not suport\n");
		return;
	}
	parm = (TaskParm*)os_malloc(sizeof(TaskParm));
	parm->Number = rsp->Number;
	parm->CMDID = T16(rsp->CMDID);
	parm->len = payload_len;
	parm->p = NULL;
//	PktPrint(packet);
	if(!parm)
	{
		GwPrint("no mems \n");
		return ;
	}
	switch(CMDID)
	{
#if 1
		case CMD_TIME:
			GwPrint("CMD_TIME \n");
			break;

		case CMD_RESET:
			GwPrint("CMD_RESET\n");
			break;

		case CMD_LOGIN:
			GwPrint("CMD_LOGIN\n");
			break;

		case CMD_PASSWORD:
			GwPrint("CMD_PASSWORD\n");
			break;

		case CMD_SCANTERM:
			GwPrint("CMD_SCANTERM\n");
			break;

		case CMD_BATALARM:
			GwPrint("CMD_BATALARM\n");
			break;

		case CMD_REPORTARM:
			GwPrint("CMD_REPORTARM\n");
			break;

		case CMD_REPORT:
			GwPrint("CMD_REPORT\n");
			break;

		case CMD_REPORTTERM:
			GwPrint("CMD_REPORTTERM\n");
			break;

		case CMD_ALARM:
			GwPrint("CMD_ALARM\n");
			break;

		case CMD_OFFLINE:
			GwPrint("CMD_OFFLINE\n");
			break;

#endif
		case CMD_TOKEN:
		case CMD_DELTERM:
		case CMD_ARM:
		case CMD_ADDTERM:
		case CMD_SETNAME:
		case CMD_SETTERMNAME:
		case CMD_UPDATING:
		case CMD_SETTIMER:
			//閿熻妭瀛樹笉閿熸枻鎷�
			if(payload_len < 2048)
			{
				parm->p = (uint8*)os_malloc(payload_len);
				os_memcpy(parm->p,rsp->uCmdRsp.pad,payload_len);
			}
			else
			{
				parm->len = 0;
				parm->p = NULL;
			}
			break;
#if 1
		case CMD_SENDTOTERM	:
			GwPrint("CMD_SENDTOTERM\n");
			break;

		case CMD_RELIEVEALARM:
			GwPrint("CMD_RELIEVEALARM\n");
			break;

		case CMD_RELIEVEARM:
			GwPrint("CMD_RELIEVEARM\n");
			break;

		case CMD_CANCLEALARM:
			GwPrint("CMD_CANCLEALARM\n");
			break;

		case CMD_VERNUM:
			GwPrint("CMD_VERNUM\n");
			break;

		case CMD_CHECKTIMER:
			GwPrint("CMD_CHECKTIMER\n");
			break;
#endif
		default :
			GwPrint("cmd is not support\n");
			break;
	}

	system_os_post(gw_procTaskPrio,SIG_TCP_RX, (os_param_t)parm);
}
LOCAL char *precvbuffer;
static uint32 dat_sumlength = 0;
LOCAL bool ICACHE_FLASH_ATTR
parse_data(char *precv, uint16 length)
{
	char * ptmp = precv;
	int len = length;
	int cnt = 0;

	while(len > 0)
	{
		if(ptmp[0] == 0x56)
		{
			TcpPrint("heart beat ack\n");
			ptmp = ptmp + 1;
			len = len - 1;
			hb_flag = 0;
		}
		else if(ptmp[0] == 0x55)
		{
			//閿熸枻鎷烽敓鏂ゆ嫹鎯搭煉鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓琛楀府鎷峰ご娌￠敓鏂ゆ嫹鍏ㄩ敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
			if(len < 14) //鍙閿熸枻鎷烽敓鐭鎷烽敓鏂ゆ嫹閿熸枻鎷�
			{
				TcpPrint("Illegal packet \n");
				precvbuffer = NULL;
				len = 0;
				dat_sumlength = 0;
				return false;
			}
			else
			{
				FrameHeader * rsp = (FrameHeader *)ptmp;
				uint16 pack_len = T16(rsp->PacketLen);
				if(pack_len <= len)
				{	//閿熸枻鎷烽敓鍓垮緱纰夋嫹涓�閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓渚ュ府鎷�
					TcpPrint("Hd=0x%x,ID=0x%x \n",rsp->Header,rsp->CMDID);
					packet_op(rsp);
					len = len - pack_len;
					ptmp = ptmp + pack_len;
				}
				else
				{	//閿熸枻鎷疯閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
					if(precvbuffer == NULL)
					{
						TcpPrint("Cache pack_len=%d,cnt=%d\n",pack_len,cnt);
						precvbuffer = (char *)os_zalloc(pack_len);
						if(precvbuffer != NULL)
							os_memcpy(precvbuffer, ptmp, len);
						dat_sumlength = len;
						len = 0;
						return false;
					}
					else
					{
						precvbuffer = NULL;
						len = 0;
						dat_sumlength = 0;
						TcpPrint("something error3 cnt = %d\n",cnt);
						return false;
					}
				}

			}

		}
		else
		{	//
			if(precvbuffer != NULL && cnt == 0)
			{
				//閿熸枻鎷风ず閿熸枻鎷峰墠閿熸枻鎷烽敓瑙掓枻鎷烽敓鏂ゆ嫹閿熻緝闈╂嫹閿熸枻鎷烽敓鎹峰府鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓锟�
				FrameHeader * rsp = (FrameHeader *)precvbuffer;
				uint16 pack_len = T16(rsp->PacketLen);
				uint16 lx_len = pack_len - dat_sumlength;
				os_memcpy(precvbuffer+dat_sumlength, ptmp, lx_len);
				//閿熸枻鎷烽敓鍓垮緱纰夋嫹涓�閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鎹峰府鎷�
				TcpPrint("lx Hd=0x%x,ID=0x%x \n",rsp->Header,rsp->CMDID);
				packet_op(rsp);
				os_free(precvbuffer);
				precvbuffer = NULL;
				ptmp = ptmp + lx_len;
				len = len - lx_len;
			}
			else
			{
				precvbuffer = NULL;
				len = 0;
				dat_sumlength = 0;
				TcpPrint("something error4 cnt = %d\n",cnt);
				return false;
			}

		}

		cnt ++;
	}
	return true;

}

/*
 * pusrdata鎸囬敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹鑾奸敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹璇ㄦ皭閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹閿熻顔炲府鎷烽敓鎻亷鎷烽敓鏂ゆ嫹閿熸枻鎷风帿閿熸枻鎷烽敓鏂ゆ嫹鑽奸敓鏂ゆ嫹閿熸枻鎷�
 *
 * */
LOCAL void ICACHE_FLASH_ATTR
gw_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	struct espconn *pespconn = arg;
	GwPrint("gw_recv_cb len = %d\n",length);

	parse_data(pusrdata,length);
}

/******************************************************************************
 * FunctionName : user_tcp_sent_cb
 * Description  : data sent callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
gw_sent_cb(void *arg)
{
//   data sent successfully
//    os_printf("tcp sent succeed !!! \r\n");
}

LOCAL void ICACHE_FLASH_ATTR
gw_discon_cb(void *arg)
{
    struct espconn *pespconn = arg;
    os_printf("gw_discon_cb\n");

    if (pespconn == NULL)
    	return;

   	tri_gw_disconn();
}

LOCAL void ICACHE_FLASH_ATTR
gw_recon_cb(void *arg, sint8 err)
{
    struct espconn *pespconn = (struct espconn *)arg;
    os_printf("gw_recon_cb\n");

   	tri_gw_disconn();
    //tri_reset();
}

/******************************************************************************
 * FunctionName : user_tcp_write_finish
 * Description  : Data need to be sent by espconn_sent has been written into write buffer successfully,
                         call espconn_sent to send next packet is allowed.
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/

LOCAL void ICACHE_FLASH_ATTR
gw_write_finish(void *arg)
{
    struct espconn *pespconn = arg;
}

LOCAL void ICACHE_FLASH_ATTR
gw_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;

    TcpPrint("gw_connect_cb\n");
    os_timer_disarm(&check_timer);
    espconn_regist_recvcb(pespconn, gw_recv_cb);

    espconn_regist_sentcb(pespconn,gw_sent_cb);
	espconn_set_opt(pespconn,0x04);
	espconn_regist_write_finish(pespconn,gw_write_finish);

    init_fddat(fd);

    fd->PacketLen = T16(PACK_SIZE(32));
	fd->CMDID = T16(CMD_TOKEN);
	os_memcpy(fd->uCmdRsp.tokeninf.Token,Token,MAX_TOKEN);
	set_timestamp(&fd->uCmdRsp.tokeninf.tm);
//	PktPrint(fd);
	espconn_send(pespconn,(uint8*)fd,PACK_SIZE(32));

    os_timer_setfn(&check_timer, (os_timer_func_t *)heart_beat_timer, &gw_conn);
    os_timer_arm(&check_timer, 500, 0);

}


LOCAL void ICACHE_FLASH_ATTR
login_recv_cb(void *arg, char *pusrdata, unsigned short length)
{

	struct espconn *pespconn = arg;
	os_timer_disarm(&check_timer);
	FrameHeader * rsp = (FrameHeader *)pusrdata;
	uint8 *psrc;

	if(rsp->Header == 0x55 && rsp->CMDID == T16(CMD_LOGIN))
	{
		psrc = (uint8*)rsp->uCmdRsp.ack_logininf.Token;
		os_memcpy(Token,psrc,MAX_TOKEN);

	}
	else
		LogPrint("this packet is not token\n");

	os_timer_setfn(&check_timer, (os_timer_func_t *)tri_reset, &gw_conn);
	os_timer_arm(&check_timer, 20000, 0);
}

LOCAL void ICACHE_FLASH_ATTR
login_discon_cb(void *arg)
{
    struct espconn *pespconn = arg;
    os_timer_disarm(&check_timer);
    LogPrint("login_discon_cb\n");

    if (pespconn == NULL)
    {
    	LogPrint("login discon_cb espconn error\n");
    	 return;
    }

    os_memcpy(pespconn->proto.tcp->remote_ip, &gw_server_ip.addr, 4);
    pespconn->proto.tcp->remote_port = GW_PORT;
    pespconn->proto.tcp->local_port = espconn_port();

	espconn_regist_connectcb(pespconn, gw_connect_cb);
	espconn_regist_disconcb(pespconn, gw_discon_cb);
	espconn_regist_reconcb(pespconn, gw_recon_cb);

	espconn_connect(pespconn);

	os_timer_setfn(&check_timer, (os_timer_func_t *)tri_reset, &gw_conn);
	os_timer_arm(&check_timer, 20000, 0);
}

LOCAL void ICACHE_FLASH_ATTR
login_recon_cb(void *arg, sint8 err)
{
    struct espconn *pespconn = (struct espconn *)arg;
    LogPrint("login_recon_cb\n");
}

LOCAL void ICACHE_FLASH_ATTR
login_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;

    os_timer_disarm(&check_timer);
    LogPrint("login_connect_cb\n");
    espconn_regist_recvcb(pespconn, login_recv_cb);

    espconn_regist_sentcb(pespconn,gw_sent_cb);

    init_fddat(fd);
    fd->PacketLen = T16(PACK_SIZE(22));
	fd->CMDID = T16(CMD_LOGIN);
	os_memcpy(fd->uCmdRsp.loginf.User,gwinfo->ucDevID,6);
	os_memcpy(fd->uCmdRsp.loginf.Password,gwinfo->ucGwPasswd,16);

	espconn_send(pespconn,(uint8*)fd,PACK_SIZE(22));
//	PktPrint(fd);

	os_timer_setfn(&check_timer, (os_timer_func_t *)tri_reset, &gw_conn);
	os_timer_arm(&check_timer, 20000, 0);

	//os_printf("shbt\n");
}

LOCAL void ICACHE_FLASH_ATTR
gw_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;
    static int flag = 0;
    if (ipaddr == NULL)
    {
    	os_printf("gw_dns_found NULL\n");
    	//user_esp_platform_reset_mode();
    	espconn_gethostbyname(&gw_conn, GW_DOMAIN, &gw_server_ip, gw_dns_found);
    	os_timer_disarm(&check_timer);
    	os_timer_setfn(&check_timer, (os_timer_func_t *)system_restart, &gw_conn);
    	os_timer_arm(&check_timer, 20000, 0);
    	return;
    }
    os_printf("gw_dns_found %d.%d.%d.%d\n",
            *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
            *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

    if (gw_server_ip.addr == 0 && ipaddr->addr != 0)
    {
		gw_server_ip.addr = ipaddr->addr;
		os_memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
		pespconn->proto.tcp->remote_port = GW_PORT;
		pespconn->proto.tcp->local_port = espconn_port();


		espconn_regist_connectcb(pespconn, login_connect_cb);
		espconn_regist_disconcb(pespconn, login_discon_cb);
		espconn_regist_reconcb(pespconn, login_recon_cb);

		espconn_connect(pespconn);
    }
	os_timer_disarm(&check_timer);
	os_timer_setfn(&check_timer, (os_timer_func_t *)tri_reset, &gw_conn);
	os_timer_arm(&check_timer, 20000, 0);
    if(flag == 0)
    {
    	init_gwinfo(gwinfo);
    	flag = 1;
    }

}

void ICACHE_FLASH_ATTR
gw_start_dns(void)
{
	//espconn_gethostbyname()
	gw_server_ip.addr = 0;

	gw_conn.proto.tcp = &gw_tcp;
	gw_conn.type = ESPCONN_TCP;
	gw_conn.state = ESPCONN_NONE;

	espconn_gethostbyname(&gw_conn, GW_DOMAIN, &gw_server_ip, gw_dns_found);

	os_timer_disarm(&check_timer);
	//绯荤粺閿熸枻鎷蜂綅
	os_timer_setfn(&check_timer, (os_timer_func_t *)tri_reset, &gw_conn);
	os_timer_arm(&check_timer, 20000, 0);
}


void ICACHE_FLASH_ATTR
gw_start_task(void)
{
	system_os_task(gw_procTask, gw_procTaskPrio, gw_procTaskQueue, gw_procTaskQueueLen);
}

