
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "sntp.h"
#include "user_sntp.h"

#include "user_timer.h"


LOCAL os_timer_t sntp_timer;

LOCAL void ICACHE_FLASH_ATTR
user_check_sntp_stamp(void *arg)
{
   uint32 current_stamp;
   current_stamp = sntp_get_current_timestamp();
   if(current_stamp == 0)
   {
		   os_printf("current_stamp---%d\n", current_stamp);
		   os_timer_arm(&sntp_timer, 1000, 1);
   }
   else
   {
	   user_check_ntptime(sntp_get_real_time(current_stamp));
//	   os_printf("sntp: %d, %s \n", current_stamp, sntp_get_real_time(current_stamp));
   }
}

LOCAL void ICACHE_FLASH_ATTR
user_get_ntp_time(void)
{
   os_timer_disarm(&sntp_timer);
   os_timer_setfn(&sntp_timer, (os_timer_func_t *)user_check_sntp_stamp, NULL);
   os_timer_arm(&sntp_timer, 1000, 1);
}

void ICACHE_FLASH_ATTR
user_start_ntp(void)
{
   ip_addr_t *addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));
   sntp_setservername(0, "us.pool.ntp.org"); // set server 0 by domain name
   sntp_setservername(1, "ntp.sjtu.edu.cn"); // set server 1 by domain name
   ipaddr_aton("210.72.145.44", addr);
   sntp_setserver(2, addr); // set server 2 by IP address
   sntp_init();
   os_free(addr);

   user_get_ntp_time();
}

