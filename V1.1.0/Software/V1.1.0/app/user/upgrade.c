#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "espconn.h"
#include "user_esp_platform.h"
#include "upgrade.h"
#include "gw.h"

#if 0
#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"
#endif

#define pheadbuffer "Accept:*/*\r\n\
Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/4.0 (compatible; MSIE 5.5; Windows 98)\
\r\n\
\r\n\
"

#define ESP_DEBUG

#ifdef ESP_DEBUG
#define ESP_DBG os_printf
#else
#define ESP_DBG
#endif

LOCAL os_timer_t client_timer;

void user_esp_platform_check_ip(void);




/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_cb
 * Description  : Processing the downloaded data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
extern void clear_soft_rst(void);
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_upgrade_rsp(void *arg)
{
    struct upgrade_server_info *server = arg;
    if (server->upgrade_flag == true) {
    	os_printf("user_esp_platform_upgrade_successful\n");
        os_free(server->url);
        server->url = NULL;
        os_free(server);
        server = NULL;
        clear_soft_rst();
        system_upgrade_reboot();

    } else {
    	os_printf("user_esp_platform_upgrade_failed\n");
    	clear_soft_rst();
    	system_restart();
    }


}

/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_begin
 * Description  : Processing the received data from the server;
 * Parameters   : pespconn -- the espconn used to connetion with the host
 *                server -- upgrade param
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_upgrade_begin(struct espconn *pespconn, struct upgrade_server_info *server)
{
    uint8 user_bin[20] = {0};


    server->pespconn = pespconn;
    server->port = 80;
    server->check_cb = user_esp_platform_upgrade_rsp;
    server->check_times = 120000;
    const char esp_server_ip[4] = {192,168,0,41};
    os_memcpy(server->ip, esp_server_ip, 4);

    if (server->url == NULL) {
        server->url = (uint8 *)os_zalloc(512);
    }
#if 1 // in general, we need to check user bin here
    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
    {
    	os_printf("down user2\n");
        os_memcpy(user_bin, "download/user2.bin", 20);
    } else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
    	os_printf("down user1\n");
        os_memcpy(user_bin, "download/user1.bin", 20);
    }

    os_sprintf(server->url, "GET /%s HTTP/1.0\r\nHost: "IPSTR":%d\r\n"pheadbuffer"",
           user_bin, IP2STR(server->ip),
           80);
    os_printf("%s\n",server->url);

    /*os_sprintf(server->url, "GET /%s HTTP/1.1\r\nHost: "IPSTR":%d\r\n"pheadbuffer"",
           user_bin, IP2STR(server->ip),server->port
           );
    os_printf("port %d url = %s\r\n",server->port,server->url);*/

#else

    os_sprintf(server->url, "GET /%s HTTP/1.0\r\nHost: "IPSTR":%d\r\n"pheadbuffer"",
           "download/file/user1.1024.new.bin", IP2STR(server->ip),
           80);

#endif


    if (system_upgrade_start(server) == false) {

        ESP_DBG("upgrade is already started\n");
    }
}
void ICACHE_FLASH_ATTR
parseUrl(char *url_,char *ipbuff_,int ipbufflen_,char *pathbuff_,int pathbufflen_,int *port_,unsigned char * addr)
{
	char *p,*p1,*p2,*p3;
	//unsigned int ipaddr;
	char header[8];
	strncpy(header,url_,7);
	header[7]=0;
	p=header;
	do{
		if(*p>='A'&&*p<='Z')
			*p=*p+0x20;
	}while(*(++p));

	if(strstr(header,"http://")==header)
	p1=url_+7;
		else
	p1=url_;

	p2=strstr(p1,":");
	p3=strstr(p1,"/");
	if(p2==NULL&&p3==NULL){
		strncpy(ipbuff_,p1,ipbufflen_-1);
		if(pathbufflen_>0){
			pathbuff_[0]='/';
			if(pathbufflen_>1)
			pathbuff_[1]=0;
		}
		*port_=80;
	}
	else
	if(p2!=NULL&&p3==NULL){
		strncpy(ipbuff_,p1,p2-p1>ipbufflen_-1?ipbufflen_-1:p2-p1);
		if(p2-p1<ipbufflen_-1){
			ipbuff_[p2-p1]=0;
		}
		else
			ipbuff_[ipbufflen_-1]=0;
		if(pathbufflen_>0){
			pathbuff_[0]='/';
			if(pathbufflen_>1)
			pathbuff_[1]=0;
		}
		*port_=atoi(p2+1);
	}
	else
	if(p2==NULL&&p3!=NULL){
		strncpy(ipbuff_,p1,p3-p1>ipbufflen_-1?ipbufflen_-1:p3-p1);
		if(p3-p1<ipbufflen_-1){
			ipbuff_[p3-p1]=0;
		}
		else
			ipbuff_[ipbufflen_-1]=0;
		strncpy(pathbuff_,p3,pathbufflen_-1);
		pathbuff_[pathbufflen_-1]=0;
		*port_=80;
	}
	else
	if(p2!=NULL&&p3!=NULL){
		if(p2<p3){
			strncpy(ipbuff_,p1,p2-p1>ipbufflen_-1?ipbufflen_-1:p2-p1);
			if(p2-p1<ipbufflen_-1)
				ipbuff_[p2-p1]=0;
			else
				ipbuff_[ipbufflen_-1]=0;
			strncpy(pathbuff_,p3,pathbufflen_-1);
			pathbuff_[pathbufflen_-1]=0;
			*port_=atoi(p2+1);
		}
		else
		{
			strncpy(ipbuff_,p1,p3-p1>ipbufflen_-1?ipbufflen_-1:p3-p1);
			if(p3-p1<ipbufflen_-1){
				ipbuff_[p3-p1]=0;
			}
			else
				ipbuff_[ipbufflen_-1]=0;
			strncpy(pathbuff_,p3,pathbufflen_-1);
			pathbuff_[pathbufflen_-1]=0;
			*port_=80;
		}
	}

	if(ipbuff_!=NULL&&ipbufflen_>0){
		//*ipaddr=0;
		p=ipbuff_;
		//*ipaddr|=atoi(p);
		addr[0] = atoi(p);
		p=strstr(p,".");
		if(p)
			addr[1] = atoi(p+1);
			//*ipaddr|=atoi(p+1)<<8;

		p=strstr(p+1,".");
		if(p)
			addr[2] = atoi(p+1);
			//*ipaddr|=atoi(p+1)<<16;
		p=strstr(p+1,".");
		if(p)
			addr[3] = atoi(p+1);
			//*ipaddr|=atoi(p+1)<<24;
	}
}

#if 1
void ICACHE_FLASH_ATTR
gw_user_upgrade_begin(Updata_URL *p)
{
	uint8 user_bin[20] = {0};
	uint8 fw_bin;
	char ip[20];
	char path[50];
	int port;
	int len;
	//unsigned int ipaddr;
	unsigned char serv_addr[4];

   	struct espconn *pespconn = NULL;
	struct upgrade_server_info *server = NULL;
	server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));

	parseUrl(p->url,ip,20,path,40,&port,&serv_addr[0]);
	os_memcpy(server->ip, &serv_addr[0], 4);
	server->port = port;

    server->check_cb = user_esp_platform_upgrade_rsp;
    server->check_times = 60000;

    if (server->url == NULL) {
        server->url = (uint8 *)os_zalloc(256);
    }
#if 1 // in general, we need to check user bin here
    fw_bin = system_upgrade_userbin_check();
    len = strlen(path);
    if(path[len-1] != '/')
    {
    	path[len] = '/';
    	path[len+1] = 0;
    }
    if (fw_bin == UPGRADE_FW_BIN1)
    {
    	os_printf("down user2\n");
    	os_sprintf(user_bin,"%suser2.bin",path);
        //os_memcpy(user_bin, "download/user2.bin", 20);
    }
    else if (fw_bin == UPGRADE_FW_BIN2)
    {
    	os_printf("down user1\n");
    	os_sprintf(user_bin,"%suser1.bin",path);
        //os_memcpy(user_bin, "download/user1.bin", 20);
    }

    os_sprintf(server->url, "GET %s HTTP/1.1\r\nHost: "IPSTR":%d\r\n"pheadbuffer"",
           user_bin, IP2STR(server->ip),server->port
           );
    os_printf("port %d url = %s\r\n",server->port,server->url);

#else

    os_sprintf(server->url, "GET /%s HTTP/1.1\r\nHost: "IPSTR"\r\n"pheadbuffer"",
           "download/user1.bin", IP2STR(server->ip)
           );


    os_printf("url = %s\r\n",server->url);

#endif


    if (system_upgrade_start(server) == false) {

        ESP_DBG("upgrade is already started\n");
    }
}

#endif

void ICACHE_FLASH_ATTR
user_upgrade_begin(void)
{
   	struct espconn *pespconn = NULL;
	struct upgrade_server_info *server = NULL;
	server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
	user_esp_platform_upgrade_begin(pespconn , server);

}


/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : espconn struct parame init when get ip addr
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_check_ip(void)
{
    struct ip_info ipconfig;

    os_timer_disarm(&client_timer);

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (ipconfig.ip.addr != 0) {
    	struct espconn *pespconn = NULL;
    	struct upgrade_server_info *server = NULL;
    	server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
    	user_esp_platform_upgrade_begin(pespconn , server);
    } else {
        os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
        os_timer_arm(&client_timer, 100, 0);
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_init
 * Description  : device parame init based on espressif platform
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_init(void)
{
	os_timer_disarm(&client_timer);
	os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
	os_timer_arm(&client_timer, 100, 0);

}
