#ifndef _GW_H_
#define _GW_H_

#include "espconn.h"

#define GW_MAJOR_VERNUM	1		//娑撹崵澧楅張顒�褰�
#define GW_MINOR_VERNUM	0		//濞嗭紕澧楅張顒�褰�
#define GW_REVISION_VERNUM	9	//娣囶喗顒滈悧鍫熸拱閸欙拷

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

//閸涙垝鎶ら崚妤勩��
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
    DEV_GW                      = 0x00, /* 鑴ラ叾楣胯劥 */
    DEV_CONTROLLER              = 0x01, /* 鑴嫝椹磋劥鑴濇 */
    DEV_MAGNETIC                = 0x02, /* 鑴欒劀楹撹劀 */
    DEV_INFRARED_SENSOR         = 0x03, /* 娼炵煕鑴ヨ姃 */
    DEV_SMOKE_SENSOR            = 0x04, /* 鑴╄劋璧傝劏 */
    DEV_MAGNETIC_WINDOW         = 0x05, /* 楹撴幊楹撹劀 */
    DEV_TEMP_HUM                = 0x06, /* 鑴﹁剹鑴㈤檵闇茶劆 */
    DEV_PM2DOT5                 = 0x07, /* PM2.5鑴ら檰铏忚姃鑴濇 */
    DEV_SMART_SOCKET            = 0x08, /* 鑴拌劅鑴涜労铏忔皳鑴抽湁 */
    DEV_TEMP_CTRL               = 0x09, /* 鑴﹁剹闇茶劆椹磋劥鑴拌劃鑴濇 */
    DEV_HUM_CTRL                = 0x0A, /* 鑴㈤檵闇茶劆椹磋劥鑴拌劃鑴濇 */
    DEV_LIGHT_CTRL              = 0x0B, /* 纰屾楣胯姃椹磋劥鑴拌劃 */
    DEV_FISH_TANK_CTRL          = 0x0C, /* 鑴尗璧傝劤椹磋劥鑴拌劃鑴濇 */
};
#if 0
enum
{
    WELCOM_WORD                 = 0x02, /* 绂勯湶鑴鑴㈤箍鑴剻鑴︽鑴拌劰鑴劃鎺宠檹璺剬鎷㈠崲鑴ら搯褰曡劔闅嗗瀯鑴℃埉椴佸獟鑴拌劗闇茶劊纰屾ゼ绂勬楣垮▌鑴涜労褰曠湁鑴╅殕鑴附鎷㈠崲鑴拌劥鑴濅箞WiFi椴侀檱鎺抽簱楣垮▌鑴涜労褰曠湁 */
    WIFI_CFG                    = 0x03, /* 鑴熸瘺鑴㈤箍鑴剻鑴︽鑴劃鑴拌劰鎺宠檹璺剬椹磋劌绂勬悅闇茶劊鑴犻搯褰曢晛鑴滅洸鑴拌剻WiFi */
    NODE_DEL                    = 0x04, /* 鑴拌劗闇茶劊鑴劑鑴℃埉椴佸獟 */
    NODE_NEAR_GW                = 0x05, /* 闄嗚姦鑴拌劗闇茶劊鑴＄尗鍗よ祩椹撮┐闄嗙湁鑴ラ叾楣胯劥鎷㈠崲鑴㈣劙闇茬倝楹撴ゼ璺瀯鑴ラ敋椴佽劇 */
    ADD                         = 0x06, /* 鑴ら搯褰曡劔 */
    DELETE                      = 0x07, /* 鑴℃埉椴佸獟 */
    NODE_ADD                    = 0x08, /* 鑴拌劗闇茶劊鑴劑鑴ら搯褰曡劔 */
    SEQUENCE                    = 0x09, /* 鑴ㄨ矊鑴曡劏娼炶劀 */
    MAGNETIC_SENSOR             = 0x0A, /* 鑴欒劀楹撹劀楹撹姦璧傝劏鑴濇 */
    REMOTE_CTRL                 = 0x0B, /* 鑴嫝椹磋劥鑴濇 */
    INFRARED_DETECTOR           = 0x0C, /* 娼炵煕鑴ヨ姃鑴ら檰铏忚姃鑴濇 */
    SMOKE_DETECTOR              = 0x0D, /* 鑴╄劋璧傝劏鑴ら檰铏忚姃鑴濇 */
    REPLACE_BETTERY             = 0x0E, /* 璧傜湁绂勭纰岃幗椴佽劥 */
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
    OUT_MODE_START_ONE_MIN      = 0x19, /* 鑴ヨ姃椴佹灇鑴涙嫝鑴㈤檰1璺劙鑴拌劔娼炶锤鑴濅箞闇茬倝 */
    NOT_CLOSED_WINDOW           = 0x1A, /* 楹撴幊绂勬悅鑴﹂簱楣胯劥 */
    NIGHT_MODE                  = 0x1B, /* 鑴箍鑴ラ搯鑴涙嫝鑴㈤檰鑴濅箞闇茬倝 */
    INHOME_MODE                 = 0x1C, /* 楣胯劥鎺冲簮鑴涙嫝鑴㈤檰鑴濅箞闇茬倝 */
    SOS                         = 0x1D, /* 绂勫啋鎴倝 */
    SOMEBODY_INTO               = 0x1E, /* 鑴劏鑴犺劊楹撻瞾鑴犳瘺 */
    EMERGENCY_CALL              = 0x1F, /* 闄嗕箞褰曞崵娼炰箞鎴劆 */
    WINDOW_DETECTOR             = 0x20 /* 楹撴幊楹撹劀楹撹姦璧傝劏鑴濇 */
};
#else
enum
{
	WELCOM_WORD_bj 		= 0x03,
    WELCOM_WORD                ,	//濞嗐垼绻嬫担璺ㄦ暏鐟楄儻鍨锋禍鎴濈暔闂冭绱濆ǎ璇插閵嗕礁鍨归梽銈囩矒缁旑垰宕熼崙璇插閼充粙鏁敍宀勫帳缂冾喖鐣ㄩ惄鎺楁毐閹稿濮涢懗浠嬫暛
    NODE_DEL                   ,	   	//缂佸牏顏鎻掑灩闂勶拷
    ADD                        ,	   	//濞ｈ濮�
    DELETE                     ,		//閸掔娀娅�
    NODE_ADD                   ,		//缂佸牏顏鍙夊潑閸旓拷
	ADD_OK					   ,		//濞ｈ濮為幋鎰
	MAGNETIC_SENSOR            ,		//闂傘劎顥嗘导鐘冲妳閸ｏ拷

    SEQUENCE                   ,		//鎼村繐鍨崣锟�
	MW_SENSOR				   ,		//闂傘劎鐛ユ导鐘冲妳閸ｏ拷

    REMOTE_CTRL                ,		//闁儲甯堕崳锟�
    INFRARED_DETECTOR          ,		//缁俱垹顦婚幒銏＄ゴ閸ｏ拷
    SMOKE_DETECTOR             ,		//閻戠喖娴橀幒銏＄ゴ閸ｏ拷
	WINDOW_DETECTOR			   ,		//缁愭顥嗘导鐘冲妳閸ｏ拷
    REPLACE_BETTERY            ,		//鐠囬攱娲块幑銏㈡暩濮癸拷
	OUT_MODE_START_ONE_MIN     ,		//婢舵牕鍤Ο鈥崇础1閸掑棝鎸撻崥搴℃儙閸旓拷
	OUT_MODE_START_30_SEC      ,		//婢舵牕鍤Ο鈥崇础30缁夋帒鎮楅崥顖氬З
	OUT_MODE_START_TWO_MIN     ,		//婢舵牕鍤Ο鈥崇础2閸掑棝鎸撻崥搴℃儙閸旓拷
	NOT_CLOSED_DOOR_WINDOW     ,		//闂傘劎鐛ラ張顏勫彠
	NIGHT_MODE                 ,		//婢舵粍娅勫Ο鈥崇础閸氼垰濮�
	INHOME_MODE                ,		//閸︺劌顔嶅Ο鈥崇础閸氼垰濮�
	SOMEBODY_INTO              ,		//閺堝姹夐梻顖氬弳
	EMERGENCY_CALL             ,		//鐠�锔斤拷銉ユ嚑閺侊拷

	SYS_DEPLOY				   ,		//缁崵绮哄鎻掔闂冿拷
	SYS_UNDEPLOY			   ,		//缁崵绮哄鍙夋寵闂冿拷
	ADD_OUT					   ,		//闁拷閸戠儤鍧婇崝鐘衬佸锟�
	DEL_OUT					   ,		//闁拷閸戝搫鍨归梽銈喣佸锟�
    WIFI_CFG                   ,	   	//鐠囪渹濞囬悽銊ㄣ偪閼哥喍绨�瑰妲籥pp闁板秶鐤哤iFi
	UPDATING				   ,
    NODE_NEAR_GW               ,		//鐏忓棛绮撶粩顖炴浆鏉╂垵鐣ㄩ惄鎺炵礉閹靛濮╃憴锕�褰傜�瑰本鍨�
	FIRE_SOS                   ,		//閻忣偉顒�
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
    WELCOM_WORD                 = 0x03,	//濞嗐垼绻嬫担璺ㄦ暏鐟楄儻鍨锋禍鎴濈暔闂冭绱濆ǎ璇插閵嗕礁鍨归梽銈囩矒缁旑垰宕熼崙璇插閼充粙鏁敍宀勫帳缂冾喖鐣ㄩ惄鎺楁毐閹稿濮涢懗浠嬫暛
    //WIFI_CFG                   ,	   	//鐠囪渹濞囬悽銊ㄣ偪閼哥喍绨�瑰妲籥pp闁板秶鐤哤iFi
    //NODE_NEAR_GW               ,		//鐏忓棛绮撶粩顖炴浆鏉╂垵鐣ㄩ惄鎺炵礉閹靛濮╃憴锕�褰傜�瑰本鍨�
    NODE_DEL                   ,	   	//缂佸牏顏鎻掑灩闂勶拷
    ADD                        ,	   	//濞ｈ濮�
    DELETE                     ,		//閸掔娀娅�
    NODE_ADD                   ,		//缂佸牏顏鍙夊潑閸旓拷
	ADD_OK					   ,		//濞ｈ濮為幋鎰
	MAGNETIC_SENSOR            ,		//闂傘劎顥嗘导鐘冲妳閸ｏ拷

    SEQUENCE                   ,		//鎼村繐鍨崣锟�
	MW_SENSOR				   ,		//闂傘劎鐛ユ导鐘冲妳閸ｏ拷

    REMOTE_CTRL                ,		//闁儲甯堕崳锟�
    INFRARED_DETECTOR          ,		//缁俱垹顦婚幒銏＄ゴ閸ｏ拷
    SMOKE_DETECTOR             ,		//閻戠喖娴橀幒銏＄ゴ閸ｏ拷
	WINDOW_DETECTOR			   ,		//缁愭顥嗘导鐘冲妳閸ｏ拷
    REPLACE_BETTERY            ,		//鐠囬攱娲块幑銏㈡暩濮癸拷
	OUT_MODE_START_ONE_MIN     ,		//婢舵牕鍤Ο鈥崇础1閸掑棝鎸撻崥搴℃儙閸旓拷
	OUT_MODE_START_30_SEC      ,		//婢舵牕鍤Ο鈥崇础30缁夋帒鎮楅崥顖氬З
	OUT_MODE_START_TWO_MIN     ,		//婢舵牕鍤Ο鈥崇础2閸掑棝鎸撻崥搴℃儙閸旓拷
	NOT_CLOSED_WINDOW          ,		//闂傘劎鐛ラ張顏勫彠
	NIGHT_MODE                 ,		//婢舵粍娅勫Ο鈥崇础閸氼垰濮�
	INHOME_MODE                ,		//閸︺劌顔嶅Ο鈥崇础閸氼垰濮�
	SOMEBODY_INTO              ,		//閺堝姹夐梻顖氬弳
	EMERGENCY_CALL             ,		//鐠�锔斤拷銉ユ嚑閺侊拷
	//FIRE_SOS                        ,		//閻忣偉顒�
	//UPDATING				   ,		//濮濓絽婀崡鍥╅獓鐎瑰娲呯粙瀣碍閿涘矁顕崟鎸庢焽閻拷
	SYS_DEPLOY				   ,		//缁崵绮哄鎻掔闂冿拷
	SYS_UNDEPLOY			   ,		//缁崵绮哄鍙夋寵闂冿拷
	ADD_OUT					   ,		//闁拷閸戠儤鍧婇崝鐘衬佸锟�
	DEL_OUT					   ,		//闁拷閸戝搫鍨归梽銈喣佸锟�
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
	ALM_CALL,			//缁毖勶拷銉ユ嚑閸欙拷
	ALM_INTO,			//閺堝姹夐梻顖氬弳
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
CMD_CANCLEALARM			=0x0015,
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
	SIG_BEAT,					//绾兛娆㈢�规碍妞傞崳銊ュ絺闁礁绺剧捄鍐插瘶post閿涳拷4/5
	SIG_ALARM_HANDLER,		    //鏉烆垯娆㈢�规碍妞傞崳銊ゆ叏閺�鐧哥礉娴ｈ法鏁ost閸欐垿锟戒緤绱濈亸鍞�spconn_send閺�鎯у煂娴犺濮熸稉顓ㄧ礉闂冨弶顒泂end闂冭顢ｆ潻娑氣柤 4/7
	SIG_TONLINE_CHECK,			//鏉烆垯娆㈢�规碍妞傞崳銊ゆ叏閺�鐧哥礉娴ｈ法鏁ost閸欐垿锟戒緤绱濈亸鍞�spconn_send閺�鎯у煂娴犺濮熸稉顓ㄧ礉闂冨弶顒泂end闂冭顢ｆ潻娑氣柤 4/7
	SIG_DOOR_OPEN_ALM,			//鏉烆垯娆㈢�规碍妞傞崳銊ゆ叏閺�鐧哥礉娴ｈ法鏁ost閸欐垿锟戒緤绱濈亸鍞�spconn_send閺�鎯у煂娴犺濮熸稉顓ㄧ礉闂冨弶顒泂end闂冭顢ｆ潻娑氣柤 4/7
	SIG_GET_NTP_TIME,
	SIG_CANCLE_ARM
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
	//uint8* pad;	//鐠虹喓娼冮弫鐗堝祦

}TaskParm;

typedef struct{
	uint8 server_ip[4];
	uint16 server_port;
	uint8 url_len;
	uint8 url[255];
}Updata_URL;

/*閻ц缍嶆穱鈩冧紖 閹绘劒姘﹂悽銊﹀煕閸氬秴鐦戦惍锟�*/
typedef struct{
	UCHAR User[6];
	UCHAR Password[16];
	TimeStamp_t tm;
}LoginInf;
/*閻ц缍嶉崶鐐差槻 娑撴槒顩﹂弰顖涘瑏Token 鐠虹喐婀囬崝鈥虫珤IP閸︽澘娼冮崣濠勵伂閸欙拷*/
typedef struct{
	UCHAR CMDLen;
	UCHAR Token[32];
	SEV_ADDR sev_ip[MAX_SEV_NUM];
	TimeStamp_t tm; //閻㈠彉绨琲p閻ㄥ嫰鏆辨惔锔跨瑝鐎规熬绱濋幍锟芥禒銉︽闂傚瓨鍩戦惃鍕秴缂冾喕绡冩稉宥呯暰閿涘奔绗夐悽銊︽￥閹碉拷鐠嬶拷
}ack_LoginInf;
/*閹绘劒姘oken娣団剝浼�*/
typedef struct{
	UCHAR Token[32];
	TimeStamp_t tm;
}TokenInf;
/*閸ョ偛顦叉穱鈩冧紖	娣囶喗鏁肩�靛棛鐖滄径杈Е鏉╂柨娲�0x00閿涚噦绱甸敍锟�*/
typedef struct{
	UCHAR ack;
	TimeStamp_t tm;
}ack_Inf;

//閼疯櫕顒濋惂璇茬秿鐎瑰本鍨�
/*娣囶喗鏁肩�靛棛鐖�*/
typedef struct{
	UCHAR CMDLen;
	UCHAR newPassword[16];
	TimeStamp_t tm;
}Password_Inf;

/*濞ｈ濮炵紒鍫㈩伂*/


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

//缂佸牏顏穱鈩冧紖
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

//缂冩垵鍙ф穱鈩冧紖閺屻儴顕楅崶鐐差槻
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

//閹懏娅欏Ο鈥崇础閺屻儴顕楅崶鐐差槻
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

/*鐠囬攱鐪版穱鈩冧紖*/
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

/*閺佺増宓侀崠鍛仈*/

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
	uint8 second[3];
}Time_Inf;


typedef struct
{
	UCHAR ucDevID[6];	//閺勵垱妫ゅ▔鏇氭叏閺�鍦畱
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

    uint8 plug_status;

    uint8  pad[2];
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

struct token_rst {
	uint8 flag;
	uint8 pad[3];
};

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

void user_set_deploymode(uint8 mode);
void gw_changemode(uint8 mode);
int jc_cc(void);
#endif
