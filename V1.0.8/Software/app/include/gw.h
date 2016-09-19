#ifndef _GW_H_
#define _GW_H_

#include "espconn.h"

#define GW_MAJOR_VERNUM	1		//涓荤増鏈彿
#define GW_MINOR_VERNUM	0		//娆＄増鏈彿
#define GW_REVISION_VERNUM	8	//淇鐗堟湰鍙�

#define GW_PORT			8002
#define GW_DOMAIN		"cloud.xd-ship.com"

#define LINK_LED_IO_MUX     	PERIPHS_IO_MUX_MTCK_U //PERIPHS_IO_MUX_MTMS_U //14
#define LINK_LED_IO_NUM     	13
#define LINK_LED_IO_FUNC    	FUNC_GPIO13

#define DEPLOY_LED_IO_MUX     	PERIPHS_IO_MUX_MTDI_U //12
#define DEPLOY_LED_IO_NUM     	12
#define DEPLOY_LED_IO_FUNC    	FUNC_GPIO12

//#define RECONBUG
#ifdef RECONBUG
#define RconPrint	os_printf
#else
#define RconPrint(...)
#endif

//#define DEBUG
#ifdef DEBUG
#define DbgPrint	os_printf
#else
#define DbgPrint(...)
#endif

//#define DNSBUG
#ifdef DNSBUG
#define DnsPrint	os_printf
#else
#define DnsPrint(...)
#endif

//#define LOGINBUG
#ifdef LOGINBUG
#define LogPrint	os_printf
#else
#define LogPrint(...)
#endif

#define TCPBUG
#ifdef TCPBUG
#define TcpPrint	os_printf
#else
#define TcpPrint(...)
#endif

#define GWBUG
#ifdef GWBUG
#define GwPrint	os_printf
#else
#define GwPrint(...)
#endif

#define APPBUG
#ifdef APPBUG
#define AppPrint	os_printf
#else
#define AppPrint(...)
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef BOOL
#define BOOL int
#endif

#ifndef UCHAR
#define UCHAR unsigned char
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef ULONG
#define ULONG unsigned int
#endif

#ifndef UINT
#define UINT unsigned int
#endif


#pragma pack(1)

typedef union NetWord{
    unsigned short   word;
    unsigned char    bytes[2];
}NetWord_t;
#define T32(x) (((x&0xff)<<24)|((x&0xff00)<<8)|((x&0xff0000)>>8)|((x&0xff000000)>>24))
#define T16(x) (((x&0xff)<<8)|((x&0xff00)>>8))

//鍛戒护鍒楄〃
/*
#define CMD_TIME			0x0001
#define CMD_RESET			0x0002
#define CMD_LOGIN			0x0003
#define CMD_TOKEN			0x0004
#define CMD_PASSWORD		0x0005
#define CMD_ADDTERM			0x0006
#define CMD_SCANTERM		0x0007
#define CMD_DELTERM			0x0008
#define CMD_BATALARM		0x0009
#define CMD_ARM				0x000a
#define CMD_REPORTARM		0x000b
#define CMD_REPORT			0x000c
#define CMD_REPORTTERM		0x000d
#define CMD_ALARM			0x000e
#define CMD_OFFLINE			0x000f
#define CMD_SETNAME			0x0010
#define CMD_SETTERMNAME		0x0011
#define CMD_SENDTOTERM		0x0012
#define CMD_RELIEVEALARM	0x0013
#define CMD_RELIEVEARM		0x0014
*/
enum
{
    DEV_GW                      = 0x00, /* 脥酶鹿脴 */
    DEV_CONTROLLER              = 0x01, /* 脪拢驴脴脝梅 */
    DEV_MAGNETIC                = 0x02, /* 脙脜麓脜 */
    DEV_INFRARED_SENSOR         = 0x03, /* 潞矛脥芒 */
    DEV_SMOKE_SENSOR            = 0x04, /* 脩脤赂脨 */
    DEV_MAGNETIC_WINDOW         = 0x05, /* 麓掳麓脜 */
    DEV_TEMP_HUM                = 0x06, /* 脦脗脢陋露脠 */
    DEV_PM2DOT5                 = 0x07, /* PM2.5脤陆虏芒脝梅 */
    DEV_SMART_SOCKET            = 0x08, /* 脰脟脛脺虏氓脳霉 */
    DEV_TEMP_CTRL               = 0x09, /* 脦脗露脠驴脴脰脝脝梅 */
    DEV_HUM_CTRL                = 0x0A, /* 脢陋露脠驴脴脰脝脝梅 */
    DEV_LIGHT_CTRL              = 0x0B, /* 碌梅鹿芒驴脴脰脝 */
    DEV_FISH_TANK_CTRL          = 0x0C, /* 脫茫赂脳驴脴脰脝脝梅 */
};
#if 0
enum
{
    WELCOM_WORD                 = 0x02, /* 禄露脫颅脢鹿脫脙脦梅脰脹脭脝掳虏路脌拢卢脤铆录脫隆垄脡戮鲁媒脰脮露脣碌楼禄梅鹿娄脛脺录眉脩隆脭帽拢卢脰脴脝么WiFi鲁陇掳麓鹿娄脛脺录眉 */
    WIFI_CFG                    = 0x03, /* 脟毛脢鹿脫脙脦梅脭脝脰脹掳虏路脌驴脥禄搂露脣脠铆录镁脜盲脰脙WiFi */
    NODE_DEL                    = 0x04, /* 脰脮露脣脪脩脡戮鲁媒 */
    NODE_NEAR_GW                = 0x05, /* 陆芦脰脮露脣脡猫卤赂驴驴陆眉脥酶鹿脴拢卢脢脰露炉麓楼路垄脥锚鲁脡 */
    ADD                         = 0x06, /* 脤铆录脫 */
    DELETE                      = 0x07, /* 脡戮鲁媒 */
    NODE_ADD                    = 0x08, /* 脰脮露脣脪脩脤铆录脫 */
    SEQUENCE                    = 0x09, /* 脨貌脕脨潞脜 */
    MAGNETIC_SENSOR             = 0x0A, /* 脙脜麓脜麓芦赂脨脝梅 */
    REMOTE_CTRL                 = 0x0B, /* 脪拢驴脴脝梅 */
    INFRARED_DETECTOR           = 0x0C, /* 潞矛脥芒脤陆虏芒脝梅 */
    SMOKE_DETECTOR              = 0x0D, /* 脩脤赂脨脤陆虏芒脝梅 */
    REPLACE_BETTERY             = 0x0E, /* 赂眉禄禄碌莽鲁脴 */
    ZERO                        = 0x0F, /* 0 */
    ONE                         = 0x10, /* 1 */
    TWO                         = 0x11, /* 2 */
    THREE                       = 0x12, /* 3 */
    FOUR                        = 0x13, /* 4 */
    FIVE                        = 0x14, /* 5 */
    SIX                         = 0x15, /* 6 */
    SEVEN                       = 0x16, /* 7 */
    EIGHT                       = 0x17, /* 8 */
    NINE                        = 0x18, /* 9 */
    OUT_MODE_START_ONE_MIN      = 0x19, /* 脥芒鲁枚脛拢脢陆1路脰脰脫潞贸脝么露炉 */
    NOT_CLOSED_WINDOW           = 0x1A, /* 麓掳禄搂脦麓鹿脴 */
    NIGHT_MODE                  = 0x1B, /* 脪鹿脥铆脛拢脢陆脝么露炉 */
    INHOME_MODE                 = 0x1C, /* 鹿脴掳庐脛拢脢陆脝么露炉 */
    SOS                         = 0x1D, /* 禄冒戮炉 */
    SOMEBODY_INTO               = 0x1E, /* 脫脨脠脣麓鲁脠毛 */
    EMERGENCY_CALL              = 0x1F, /* 陆么录卤潞么戮脠 */
    WINDOW_DETECTOR             = 0x20 /* 麓掳麓脜麓芦赂脨脝梅 */
};
#else
enum
{
	WELCOM_WORD_bj 		= 0x03,
    WELCOM_WORD                ,	//娆㈣繋浣跨敤瑗胯垷浜戝畨闃诧紝娣诲姞銆佸垹闄ょ粓绔崟鍑诲姛鑳介敭锛岄厤缃畨鐩掗暱鎸夊姛鑳介敭
    NODE_DEL                   ,	   	//缁堢宸插垹闄�
    ADD                        ,	   	//娣诲姞
    DELETE                     ,		//鍒犻櫎
    NODE_ADD                   ,		//缁堢宸叉坊鍔�
	ADD_OK					   ,		//娣诲姞鎴愬姛
	MAGNETIC_SENSOR            ,		//闂ㄧ浼犳劅鍣�

    SEQUENCE                   ,		//搴忓垪鍙�
	MW_SENSOR				   ,		//闂ㄧ獥浼犳劅鍣�

    REMOTE_CTRL                ,		//閬ユ帶鍣�
    INFRARED_DETECTOR          ,		//绾㈠鎺㈡祴鍣�
    SMOKE_DETECTOR             ,		//鐑熼浘鎺㈡祴鍣�
	WINDOW_DETECTOR			   ,		//绐楃浼犳劅鍣�
    REPLACE_BETTERY            ,		//璇锋洿鎹㈢數姹�
	OUT_MODE_START_ONE_MIN     ,		//澶栧嚭妯″紡1鍒嗛挓鍚庡惎鍔�
	OUT_MODE_START_30_SEC      ,		//澶栧嚭妯″紡30绉掑悗鍚姩
	OUT_MODE_START_TWO_MIN     ,		//澶栧嚭妯″紡2鍒嗛挓鍚庡惎鍔�
	NOT_CLOSED_DOOR_WINDOW     ,		//闂ㄧ獥鏈叧
	NIGHT_MODE                 ,		//澶滄櫄妯″紡鍚姩
	INHOME_MODE                ,		//鍦ㄥ妯″紡鍚姩
	SOMEBODY_INTO              ,		//鏈変汉闂叆
	EMERGENCY_CALL             ,		//璀︽�ュ懠鏁�

	SYS_DEPLOY				   ,		//绯荤粺宸插竷闃�
	SYS_UNDEPLOY			   ,		//绯荤粺宸叉挙闃�
	ADD_OUT					   ,		//閫�鍑烘坊鍔犳ā寮�
	DEL_OUT					   ,		//閫�鍑哄垹闄ゆā寮�
    WIFI_CFG                   ,	   	//璇蜂娇鐢ㄨタ鑸熶簯瀹夐槻app閰嶇疆WiFi
	UPDATING				   ,
    NODE_NEAR_GW               ,		//灏嗙粓绔潬杩戝畨鐩掞紝鎵嬪姩瑙﹀彂瀹屾垚
	FIRE_SOS                   ,		//鐏
	FIRE_SOS_1				   ,
	NOT_CLOSED_DOOR			   ,
	NOT_CLOSED_WINDOW	   	   ,
    ZERO                       ,
    ONE                        ,
    TWO                        ,
    THREE                      ,
    FOUR                       ,
    FIVE                       ,
    SIX                        ,
    SEVEN                      ,
    EIGHT                      ,
    NINE                       ,

	AA						   ,
	BB						   ,
};
/*
enum
{
    WELCOM_WORD                 = 0x03,	//娆㈣繋浣跨敤瑗胯垷浜戝畨闃诧紝娣诲姞銆佸垹闄ょ粓绔崟鍑诲姛鑳介敭锛岄厤缃畨鐩掗暱鎸夊姛鑳介敭
    //WIFI_CFG                   ,	   	//璇蜂娇鐢ㄨタ鑸熶簯瀹夐槻app閰嶇疆WiFi
    //NODE_NEAR_GW               ,		//灏嗙粓绔潬杩戝畨鐩掞紝鎵嬪姩瑙﹀彂瀹屾垚
    NODE_DEL                   ,	   	//缁堢宸插垹闄�
    ADD                        ,	   	//娣诲姞
    DELETE                     ,		//鍒犻櫎
    NODE_ADD                   ,		//缁堢宸叉坊鍔�
	ADD_OK					   ,		//娣诲姞鎴愬姛
	MAGNETIC_SENSOR            ,		//闂ㄧ浼犳劅鍣�

    SEQUENCE                   ,		//搴忓垪鍙�
	MW_SENSOR				   ,		//闂ㄧ獥浼犳劅鍣�

    REMOTE_CTRL                ,		//閬ユ帶鍣�
    INFRARED_DETECTOR          ,		//绾㈠鎺㈡祴鍣�
    SMOKE_DETECTOR             ,		//鐑熼浘鎺㈡祴鍣�
	WINDOW_DETECTOR			   ,		//绐楃浼犳劅鍣�
    REPLACE_BETTERY            ,		//璇锋洿鎹㈢數姹�
	OUT_MODE_START_ONE_MIN     ,		//澶栧嚭妯″紡1鍒嗛挓鍚庡惎鍔�
	OUT_MODE_START_30_SEC      ,		//澶栧嚭妯″紡30绉掑悗鍚姩
	OUT_MODE_START_TWO_MIN     ,		//澶栧嚭妯″紡2鍒嗛挓鍚庡惎鍔�
	NOT_CLOSED_WINDOW          ,		//闂ㄧ獥鏈叧
	NIGHT_MODE                 ,		//澶滄櫄妯″紡鍚姩
	INHOME_MODE                ,		//鍦ㄥ妯″紡鍚姩
	SOMEBODY_INTO              ,		//鏈変汉闂叆
	EMERGENCY_CALL             ,		//璀︽�ュ懠鏁�
	//FIRE_SOS                        ,		//鐏
	//UPDATING				   ,		//姝ｅ湪鍗囩骇瀹夌洅绋嬪簭锛岃鍕挎柇鐢�
	SYS_DEPLOY				   ,		//绯荤粺宸插竷闃�
	SYS_UNDEPLOY			   ,		//绯荤粺宸叉挙闃�
	ADD_OUT					   ,		//閫�鍑烘坊鍔犳ā寮�
	DEL_OUT					   ,		//閫�鍑哄垹闄ゆā寮�
    ZERO                       ,
    ONE                        ,
    TWO                        ,
    THREE                      ,
    FOUR                       ,
    FIVE                       ,
    SIX                        ,
    SEVEN                      ,
    EIGHT                      ,
    NINE                       ,

	AA						   ,
	BB						   ,
};*/
//#define WIFI_CFG	AA
//#define FIRE_SOS    BB
#endif
enum
{
	ALM_FIRE  = 0x01,
	ALM_CALL,			//绱ф�ュ懠鍙�
	ALM_INTO,			//鏈変汉闂叆
	ALM_WINOPEN,
	ALM_LOWVOL,

};

enum CMD_Z{
CMD_START			=0x0000,

CMD_TIME			=0x0001,
CMD_RESET			=0x0002,
CMD_LOGIN			=0x0003,
CMD_TOKEN			=0x0004,
CMD_PASSWORD		=0x0005,
CMD_ADDTERM			=0x0006,
CMD_SCANTERM		=0x0007,
CMD_DELTERM			=0x0008,
CMD_BATALARM		=0x0009,
CMD_ARM				=0x000a,
CMD_REPORTARM		=0x000b,
CMD_REPORT			=0x000c,
CMD_REPORTTERM		=0x000d,
CMD_ALARM			=0x000e,
CMD_OFFLINE			=0x000f,
CMD_SETNAME			=0x0010,
CMD_SETTERMNAME		=0x0011,
CMD_SENDTOTERM		=0x0012,
CMD_RELIEVEALARM	=0x0013,
CMD_RELIEVEARM		=0x0014,
CMD_UPDATING		=0x0017,
CMD_VERNUM			=0x0019,
CMD_TONLINE			=0x0020,
CMD_SETTIMER		=0x0021,
CMD_CHECKTIMER		=0x0022,

CMD_END
};



enum
{
    SMOKE_ALM                   = 0x01,
    SMOKE_NOR_ALM               = 0x02,
    DOOR_OPEN_ALM               = 0x03,
    DOOR_CLOSE_ALM              = 0x04,
    INFRARED_ALM                = 0x05,
    DEV_OFFLINE_ALM             = 0x06,
    DEV_FAULT_ALM               = 0x07,
    DEV_ONLINE_ALM              = 0x08,
    DEV_VOL_LOW_ALM             = 0x09,
    DEV_VOL_NOR_ALM             = 0x0A,
    DEV_ALM_NORMAL              = 0x0B,
    DEV_SOS                     = 0x0C,
};

enum
{
    OUTMODE                     = 0x01,
    NIGHTMODE                   = 0x02,
    INHOMEMODE                  = 0x03,
    CANCELMODE                  = 0x04,
};
enum
{
    NOBINDED                    = 0x00,
    BINDED                      = 0x01,
};

enum
{
    OFFLINE                     = 0x00,
    ONLINE                      = 0x01,
};

enum
{
    NODEPLOY                    = 0x00,
    DEPLOY                      = 0x01,
};

enum
{
    DEV_CONTROLLER_ETH          = 0x01,
    DEV_MAGNETIC_ETH            = 0x02,
    DEV_INFRARED_SENSOR_ETH     = 0x03,
    DEV_SMOKE_SENSOR_ETH        = 0x04,
    DEV_MAGNETIC_WINDOW_ETH     = 0x05,
};

enum
{
    PKT_UP_REQ                  = 0x01,
    PKT_REQ_ACK                 = 0x02,
    PKT_DOWN_CMD                = 0x03,
    PKT_CMD_ACK                 = 0x04
};

enum sig_type {
	SIG_TCP_RX    = 1,
	SIG_RESET,
	SIG_HEART_BEAT,
	SIG_KEY,
	SIG_KEY_LONG,
	SIG_KEY_RESET,
	SIG_RF_REC,
	SIG_GW_DISCONN,
	SIG_BEAT,					//纭欢瀹氭椂鍣ㄥ彂閫佸績璺冲寘post锛�4/5
	SIG_ALARM_HANDLER,		    //杞欢瀹氭椂鍣ㄤ慨鏀癸紝浣跨敤post鍙戦�侊紝灏唀spconn_send鏀惧埌浠诲姟涓紝闃叉send闃诲杩涚▼ 4/7
	SIG_TONLINE_CHECK,			//杞欢瀹氭椂鍣ㄤ慨鏀癸紝浣跨敤post鍙戦�侊紝灏唀spconn_send鏀惧埌浠诲姟涓紝闃叉send闃诲杩涚▼ 4/7
	SIG_DOOR_OPEN_ALM,			//杞欢瀹氭椂鍣ㄤ慨鏀癸紝浣跨敤post鍙戦�侊紝灏唀spconn_send鏀惧埌浠诲姟涓紝闃叉send闃诲杩涚▼ 4/7
	SIG_GET_NTP_TIME
	/*SIG_YGBJ,
	SIG_DELAY_30S
	SIG_UART_RX,
	SIG_REPORT,
	SIG_REPORTARM,
	SIG_ACKOK*/
};

enum {
	GW_INIT	= 0,
	GW_CONFIG,
	GW_WORKING,
	GW_ADD,
	GW_DEL
};
enum{
	UNCONF = 0,
	CONFIGING,
	CONFIGED
};

#define MAC_SIZE		6
#define MAX_USERNAME	6
#define MAX_PASSWORD	16
#define MAX_TOKEN		32

#define NODE_TOTAL              30

#define DAYTIMER_NUMBER_TOTAL	10
#define TIMER_TOTAL				7

#define MAX_SEV_NUM	4

#define PACK_HDSIZE			16
#define PACK_TMSTAMPSIZE	7
#define PACK_SIZE(CMD_LEN)	(PACK_HDSIZE+CMD_LEN+PACK_TMSTAMPSIZE)
#define PAYLOAD_SIZE(LEN)	(LEN - PACK_HDSIZE - PACK_TMSTAMPSIZE)

typedef struct  {
        union {
			struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
			struct { USHORT s_w1,s_w2; } S_un_w;
			ULONG S_addr;
        } S_un;
		USHORT port;
}SEV_ADDR;



typedef struct{
	USHORT tm_year; //
	UCHAR tm_month; //
	UCHAR tm_day;
	UCHAR tm_hour;
	UCHAR tm_min;
	UCHAR tm_sec;
}TimeStamp_t;

typedef union{
	uint8 pad[4];

}Parm;

typedef struct{
	uint32 Number;
	uint16 CMDID;
	uint16 len;
	uint8 * p;
	//Parm parm;
	//uint8* pad;	//璺熺潃鏁版嵁

}TaskParm;

typedef struct{
	uint8 server_ip[4];
	uint16 server_port;
	uint8 url_len;
	uint8 url[255];
}Updata_URL;

/*鐧诲綍淇℃伅 鎻愪氦鐢ㄦ埛鍚嶅瘑鐮�*/
typedef struct{
	UCHAR User[6];
	UCHAR Password[16];
	TimeStamp_t tm;
}LoginInf;
/*鐧诲綍鍥炲 涓昏鏄嬁Token 璺熸湇鍔″櫒IP鍦板潃鍙婄鍙�*/
typedef struct{
	UCHAR CMDLen;
	UCHAR Token[32];
	SEV_ADDR sev_ip[MAX_SEV_NUM];
	TimeStamp_t tm; //鐢变簬ip鐨勯暱搴︿笉瀹氾紝鎵�浠ユ椂闂存埑鐨勪綅缃篃涓嶅畾锛屼笉鐢ㄦ棤鎵�璋�
}ack_LoginInf;
/*鎻愪氦Token淇℃伅*/
typedef struct{
	UCHAR Token[32];
	TimeStamp_t tm;
}TokenInf;
/*鍥炲淇℃伅	淇敼瀵嗙爜澶辫触杩斿洖0x00锛燂紵锛�*/
typedef struct{
	UCHAR ack;
	TimeStamp_t tm;
}ack_Inf;

//鑷虫鐧诲綍瀹屾垚
/*淇敼瀵嗙爜*/
typedef struct{
	UCHAR CMDLen;
	UCHAR newPassword[16];
	TimeStamp_t tm;
}Password_Inf;

/*娣诲姞缁堢*/


typedef struct{
	UCHAR term_n[8];
	UCHAR term_type;
	UCHAR term_name[20];
}Term_Inf;



typedef struct{
	Term_Inf term_inf;
	UCHAR arm_type;
	TimeStamp_t tm;
}Alarm_Inf;

//缁堢淇℃伅
typedef struct
{
	UCHAR ucNodeID[8];
	UCHAR ucNodeType;
	UCHAR ucNodeName[20];
	UCHAR ucOnlineState;
	UCHAR ucBindState;
	UCHAR ucDeployStatus;
	UCHAR ucAlmStatus;
}NodeInf;

//缃戝叧淇℃伅鏌ヨ鍥炲
typedef struct{
	UCHAR ucDevID[6];
	UCHAR ucGwName[20];
	UCHAR ucNodeN;
	UCHAR ucNDInfSize;
	NodeInf node[NODE_TOTAL];
}Report_Inf;

typedef struct
{
	UCHAR ucNodeID[8];
	UCHAR ucNodeType;
	UCHAR ucNodeName[20];
	UCHAR ucDeployStatus;
}NodeBaseInf;

typedef struct
{
	UCHAR ucCurMode;
	UCHAR ucSel;
	UCHAR ucNodeN;
	NodeBaseInf nodebase[NODE_TOTAL];
}SceneMode;

//鎯呮櫙妯″紡鏌ヨ鍥炲
typedef struct{
	UCHAR ucDevID[6];
	UCHAR ucGwName[20];
	UCHAR ucNodeN;
	UCHAR ucNDInfSize;
	NodeInf node[NODE_TOTAL];
}ReportArm_Inf;

typedef struct{
	uint8 hour;
	uint8 min;
	uint8 mode;
}TimerData_Inf;

typedef struct{
	uint8 week;
	uint8 timerday_number;
	TimerData_Inf timerdata_inf[DAYTIMER_NUMBER_TOTAL];
}Timer_Inf;

typedef struct
{
	Timer_Inf timer[TIMER_TOTAL];
}Timer;

/*璇锋眰淇℃伅*/
typedef union{
	UCHAR pad[4096];
	LoginInf loginf;
	ack_LoginInf ack_logininf;
	TokenInf tokeninf;
	ack_Inf ack_inf;
	Password_Inf password_inf;
	Term_Inf addterm_inf;
	Alarm_Inf alarm_inf;
	Report_Inf report_inf;
	ReportArm_Inf reporta_inf;
	SceneMode scmode;

	Timer timer;

}xd_uCmdRsp;

/*鏁版嵁鍖呭ご*/

typedef struct {
	UCHAR Header;
	ULONG Number;
	UCHAR Dir;
	UCHAR MAC[6];
	USHORT PacketLen;
	USHORT CMDID;
	xd_uCmdRsp uCmdRsp;
}FrameHeader;


typedef struct
{
	NodeInf info;
	UCHAR ucDelayStateOut;
	UCHAR ucDelayStateNight;
	UCHAR ucDelayStateInhome;
	UCHAR ucVail;
	UCHAR ucLastVal;
	uint32 ulLastTime;
}NODEINFOSTRU;

typedef struct{
	uint8 week[4];
	uint8 hour[3];
	uint8 min[3];
}Time_Inf;


typedef struct
{
	UCHAR ucDevID[6];	//鏄棤娉曚慨鏀圭殑
	UCHAR ucGwName[20];
	UCHAR ucGwPasswd[16];

	UCHAR ucNodeNum;
	UCHAR ucCurSceneMode;
    //SERVERLISTSTRU serverListStru[SERVER_NUM];
    NODEINFOSTRU node[NODE_TOTAL];
    uint32 saved;
    uint32 saved_cnt;
    uint8  smartconf;

    Timer_Inf timer_inf[TIMER_TOTAL];

    uint8  pad[3];
/*	UCHAR ucSrcIP[4];
	UCHAR ucNetMask[4];
	UCHAR ucDefaultGw[4];
	UCHAR ucDstIP[4];
	USHORT usDstPort;
	UCHAR ucCurTime[7];
	UCHAR ucSocketID;
	ULONG ulSerialNum;
	UCHAR ucToken[32];
	UCHAR ucGwLoginFlag;
	UCHAR ucGwConnFlag;
	UCHAR ucAlmStatus;
	UCHAR ucDeployStatus;
	UCHAR ucSocketFlag;*/

    //SCENEMODESTRU sceneModeStru;
}GWCFGINFOSTRU;

extern GWCFGINFOSTRU  * gwinfo;

typedef struct
{
	uint8 type;
	uint8 id[5];
	uint8 z;
	uint8 db;
}RFS;



#pragma pack()
void init_ts(void);
void gw_start_dns(void);
void gw_start_task(void);
void yy_bf(uint8 nr);
void set_offline_time(void);

uint8 get_ap_config(void);
void set_ap_config(uint8 conf);


void link_led_init(void);
void link_led_output(uint8 level);
void deploy_led_init(void);
void deploy_led_output(uint8 level);

void gw_changemode(uint8 mode);
int jc_cc(void);
#endif
