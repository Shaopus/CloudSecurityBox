
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "gw.h"
#include "user_timer.h"
#include "user_sntp.h"


Time_Inf time_inf;


LOCAL int ICACHE_FLASH_ATTR
user_get_ntpweek(Time_Inf *time)
{
	int week =0;
	if(0==memcmp("Mon",(char*)&time->week,3))
			week = 1;
		else if(0==memcmp("Tue",(char*)&time->week,3))
				week = 2;
			else if(0==memcmp("Wed",(char*)&time->week,3))
					week = 3;
				else if(0==memcmp("Thu",(char*)&time->week,3))
						week = 4;
					else if(0==memcmp("Fri",(char*)&time->week,3))
							week = 5;
						else if(0==memcmp("Sta",(char*)&time->week,3))
								week = 6;
							else if(0==memcmp("Sun",(char*)&time->week,3))
									week = 7;
								else
									return -1;
	return week;
}

LOCAL int ICACHE_FLASH_ATTR
user_get_ntphour(Time_Inf *time)
{
	return atoi((char*)&time->hour);
}

LOCAL int ICACHE_FLASH_ATTR
user_get_ntpmin(Time_Inf *time)
{
	return atoi((char*)&time->min);
}

LOCAL int ICACHE_FLASH_ATTR
user_get_ntpsecond(Time_Inf *time)
{
	return atoi((char*)&time->second);
}




LOCAL void ICACHE_FLASH_ATTR
user_check_timer(int week,int hour,int min,int second)
{
	uint8 i = 0,j = 0;

	for(i = 0;i<TIMER_TOTAL;i++)
	{
		if(gwinfo->timer_inf[i].timerday_number != 0)
		{
			for(j =0 ;j < gwinfo->timer_inf[i].timerday_number; j++)
			{
				if( hour== (gwinfo->timer_inf[i].timerdata_inf[j].hour) &&
					 min == (gwinfo->timer_inf[i].timerdata_inf[j].min) &&
					 second ==0)
				{
					if((week == gwinfo->timer_inf[i].week))
					{
						uint8 mode = gwinfo->timer_inf[i].timerdata_inf[j].mode ;
						user_set_deploymode(mode);
					}
				}
			}
		}
	}
}

void ICACHE_FLASH_ATTR
user_check_ntptime(char *real_time)
{
	int week = 0,hour = 0,min = 0,second = 0;

	memset(&time_inf,0,sizeof(time_inf));
	memcpy(&time_inf.week,real_time,3);
	memcpy(&time_inf.hour,real_time+11,2);
	memcpy(&time_inf.min,real_time+14,2);
	memcpy(&time_inf.second,real_time+17,2);

	week = user_get_ntpweek(&time_inf);
	hour = user_get_ntphour(&time_inf);
	min = user_get_ntpmin(&time_inf);
	second = user_get_ntpsecond(&time_inf);

	user_check_timer(week,hour,min,second);
//	os_printf("%s,%s,%s,%d\n\r",time_inf.week,
//		time_inf.hour,time_inf.min,sizeof(time_inf));
}

