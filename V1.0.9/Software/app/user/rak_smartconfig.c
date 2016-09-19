#include "rak_smartconfig.h"

uint8 smart_lock_flag=0;
uint8 smart_mac[6];
uint8 smart_channel=-1;
uint8 smart_status = SMART_CH_INIT;
os_timer_t channel_timer;
uint8 current_channel;
uint16 channel_bits;


SLIST_HEAD(router_info_head, router_info) router_list;

void ICACHE_FLASH_ATTR rak_promiscuous_rx(uint8 *buf, uint16 len);

void wifi_scan_done(void *arg, STATUS status);
void smartconfig_end();

#define USE_RAK 0

#include "gw.h"

#if USE_RAK == 0
#include "airkiss.h"

airkiss_context_t akcontex;
#if 0
#include "stdarg.h"
#define RT_CONSOLEBUF_SIZE 255
#define rt_int32_t int32
#define rt_uint32_t uint32
#define rt_uint16_t uint16
#define rt_int16_t uint16
#define rt_uint8_t uint8

#define ZEROPAD     (1 << 0)    /* pad with zero */
#define SIGN        (1 << 1)    /* unsigned/signed long */
#define PLUS        (1 << 2)    /* show plus */
#define SPACE       (1 << 3)    /* space if plus */
#define LEFT        (1 << 4)    /* left justified */
#define SPECIAL     (1 << 5)    /* 0x */
#define LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */
#ifdef RT_PRINTF_PRECISION
static char *print_number(char *buf,
                          char *end,
                          long  num,
                          int   base,
                          int   s,
                          int   precision,
                          int   type)
#else
static char *print_number(char *buf,
                          char *end,
                          long  num,
                          int   base,
                          int   s,
                          int   type)
#endif
{
    char c, sign;
#ifdef RT_PRINTF_LONGLONG
    char tmp[32];
#else
    char tmp[16];
#endif
    const char *digits;
    static const char small_digits[] = "0123456789abcdef";
    static const char large_digits[] = "0123456789ABCDEF";
    register int i;
    register int size;

    size = s;

    digits = (type & LARGE) ? large_digits : small_digits;
    if (type & LEFT)
        type &= ~ZEROPAD;

    c = (type & ZEROPAD) ? '0' : ' ';

    /* get sign */
    sign = 0;
    if (type & SIGN)
    {
        if (num < 0)
        {
            sign = '-';
            num = -num;
        }
        else if (type & PLUS)
            sign = '+';
        else if (type & SPACE)
            sign = ' ';
    }

#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }
#endif

    i = 0;
    if (num == 0)
        tmp[i++]='0';
    else
    {
        while (num != 0)
            tmp[i++] = digits[divide(&num, base)];
    }

#ifdef RT_PRINTF_PRECISION
    if (i > precision)
        precision = i;
    size -= precision;
#else
    size -= i;
#endif

    if (!(type&(ZEROPAD | LEFT)))
    {
        if ((sign)&&(size>0))
            size--;

        while (size-->0)
        {
            if (buf <= end)
                *buf = ' ';
            ++ buf;
        }
    }

    if (sign)
    {
        if (buf <= end)
        {
            *buf = sign;
            -- size;
        }
        ++ buf;
    }

#ifdef RT_PRINTF_SPECIAL
    if (type & SPECIAL)
    {
        if (base==8)
        {
            if (buf <= end)
                *buf = '0';
            ++ buf;
        }
        else if (base == 16)
        {
            if (buf <= end)
                *buf = '0';
            ++ buf;
            if (buf <= end)
            {
                *buf = type & LARGE? 'X' : 'x';
            }
            ++ buf;
        }
    }
#endif

    /* no align to the left */
    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            if (buf <= end)
                *buf = c;
            ++ buf;
        }
    }

#ifdef RT_PRINTF_PRECISION
    while (i < precision--)
    {
        if (buf <= end)
            *buf = '0';
        ++ buf;
    }
#endif

    /* put number in the temporary buffer */
    while (i-- > 0)
    {
        if (buf <= end)
            *buf = tmp[i];
        ++ buf;
    }

    while (size-- > 0)
    {
        if (buf <= end)
            *buf = ' ';
        ++ buf;
    }

    return buf;
}


static rt_int32_t vsnprintf(char       *buf,
                            int   size,
                            const char *fmt,
                            va_list     args)
{
#ifdef RT_PRINTF_LONGLONG
    unsigned long long num;
#else
    rt_uint32_t num;
#endif
    int i, len;
    char *str, *end, c;
    const char *s;

    rt_uint8_t base;            /* the base of number */
    rt_uint8_t flags;           /* flags to print number */
    rt_uint8_t qualifier;       /* 'h', 'l', or 'L' for integer fields */
    rt_int32_t field_width;     /* width of output field */

#ifdef RT_PRINTF_PRECISION
    int precision;      /* min. # of digits for integers and max for a string */
#endif

    str = buf;
    end = buf + size - 1;

    /* Make sure end is always >= buf */
    if (end < buf)
    {
        end  = ((char *)-1);
        size = end - buf;
    }

    for (; *fmt ; ++fmt)
    {
        if (*fmt != '%')
        {
            if (str <= end)
                *str = *fmt;
            ++ str;
            continue;
        }

        /* process flags */
        flags = 0;

        while (1)
        {
            /* skips the first '%' also */
            ++ fmt;
            if (*fmt == '-') flags |= LEFT;
            else if (*fmt == '+') flags |= PLUS;
            else if (*fmt == ' ') flags |= SPACE;
            else if (*fmt == '#') flags |= SPECIAL;
            else if (*fmt == '0') flags |= ZEROPAD;
            else break;
        }

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt)) field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            ++ fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

#ifdef RT_PRINTF_PRECISION
        /* get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            ++ fmt;
            if (isdigit(*fmt)) precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                ++ fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0) precision = 0;
        }
#endif
        /* get the conversion qualifier */
        qualifier = 0;
#ifdef RT_PRINTF_LONGLONG
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L')
#else
        if (*fmt == 'h' || *fmt == 'l')
#endif
        {
            qualifier = *fmt;
            ++ fmt;
#ifdef RT_PRINTF_LONGLONG
            if (qualifier == 'l' && *fmt == 'l')
            {
                qualifier = 'L';
                ++ fmt;
            }
#endif
        }

        /* the default base */
        base = 10;

        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))
            {
                while (--field_width > 0)
                {
                    if (str <= end) *str = ' ';
                    ++ str;
                }
            }

            /* get character */
            c = (rt_uint8_t)va_arg(args, int);
            if (str <= end) *str = c;
            ++ str;

            /* put width */
            while (--field_width > 0)
            {
                if (str <= end) *str = ' ';
                ++ str;
            }
            continue;

        case 's':
            s = va_arg(args, char *);
            if (!s) s = "(NULL)";

            len = os_strlen(s);
#ifdef RT_PRINTF_PRECISION
            if (precision > 0 && len > precision) len = precision;
#endif

            if (!(flags & LEFT))
            {
                while (len < field_width--)
                {
                    if (str <= end) *str = ' ';
                    ++ str;
                }
            }

            for (i = 0; i < len; ++i)
            {
                if (str <= end) *str = *s;
                ++ str;
                ++ s;
            }

            while (len < field_width--)
            {
                if (str <= end) *str = ' ';
                ++ str;
            }
            continue;

        case 'p':
            if (field_width == -1)
            {
                field_width = sizeof(void *) << 1;
                flags |= ZEROPAD;
            }
#ifdef RT_PRINTF_PRECISION
            str = print_number(str, end,
                               (long)va_arg(args, void *),
                               16, field_width, precision, flags);
#else
            str = print_number(str, end,
                               (long)va_arg(args, void *),
                               16, field_width, flags);
#endif
            continue;

        case '%':
            if (str <= end) *str = '%';
            ++ str;
            continue;

            /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            if (str <= end) *str = '%';
            ++ str;

            if (*fmt)
            {
                if (str <= end) *str = *fmt;
                ++ str;
            }
            else
            {
                -- fmt;
            }
            continue;
        }

#ifdef RT_PRINTF_LONGLONG
        if (qualifier == 'L') num = va_arg(args, long long);
        else if (qualifier == 'l')
#else
        if (qualifier == 'l')
#endif
        {
            num = va_arg(args, rt_uint32_t);
            if (flags & SIGN) num = (rt_int32_t)num;
        }
        else if (qualifier == 'h')
        {
            num = (rt_uint16_t)va_arg(args, rt_int32_t);
            if (flags & SIGN) num = (rt_int16_t)num;
        }
        else
        {
            num = va_arg(args, rt_uint32_t);
            if (flags & SIGN) num = (rt_int32_t)num;
        }
#ifdef RT_PRINTF_PRECISION
        str = print_number(str, end, num, base, field_width, precision, flags);
#else
        str = print_number(str, end, num, base, field_width, flags);
#endif
    }

    if (str <= end) *str = '\0';
    else *end = '\0';

    /* the trailing null byte doesn't count towards the total
    * ++str;
    */
    return str - buf;
}

int airkiss_printf(const char *fmt, ...)
{
    va_list args;
    int length;
    static char rt_log_buf[256];

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;
    os_printf("%s",rt_log_buf);
    va_end(args);
}
#endif
const airkiss_config_t akconf =
{
  (airkiss_memset_fn)&memset,
  (airkiss_memcpy_fn)&memcpy,
  (airkiss_memcmp_fn)&memcmp,
  0
};

void start_airkiss(void)
{
  int8_t ret;
  //如果有开启AES功能，定义AES密码，注意与手机端的密码一致
  const char* key = "xdnb123456789012";

  os_printf("Start airkiss!\r\n");
  //调用接口初始化AirKiss流程，每次调用该接口，流程重新开始， akconf需要预先设置好参数

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
}

//extern void connect_ap(void);
static void airkiss_finish(void)
{
  int8_t err;
  uint8 buffer[256];
  airkiss_result_t result;
  err = airkiss_get_result(&akcontex, &result);

  if (err == 0)
  {


	struct station_config conf1;
	memcpy(&conf1.ssid[0],result.ssid,32);
	memcpy(&conf1.password[0],result.pwd,64);
	smartconfig_end();

	//wifi_station_disconnect();

	//wifi_set_opmode(STATION_MODE);
	wifi_station_set_config(&conf1);
	wifi_station_set_auto_connect(1);

	set_ap_config(CONFIGING);

	deploy_led_output(0);
    //os_printf("airkiss_get_result() ok!");
    //os_sprintf(buffer, "ssid = \"%s\", pwd = \"%s\", ssid_length = %d, "pwd_length = %d, random = 0x%02x\r\n", result.ssid, result.pwd, result.ssid_length, result.pwd_length, result.random);
    //os_printf(buffer);
	os_printf("airkiss_get_result() ok!\r\n");
    os_printf("ssid = %s, pwd = %s, ssid_length = %d,pwd_length = %d, random = 0x%02x\r\n", result.ssid, result.pwd, result.ssid_length, result.pwd_length, result.random);

    os_timer_disarm(&channel_timer);
	os_timer_setfn(&channel_timer, system_restart, NULL);
	os_timer_arm(&channel_timer, 1500, 0);
  }
  else
  {
	  os_printf("airkiss_get_result() failed !\r\n");
  }
}

uint32 err_flag = 0;
void ICACHE_FLASH_ATTR
rak_promiscuous_rx(uint8 *buf, uint16 len)
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
	else
		return;
	//将网络帧传入airkiss库进行处理
	//os_printf("%d , %s\r\n",len,buf);
	ret = airkiss_recv(&akcontex, hd, hd_len);
	//判断返回值，确定是否锁定信道或者读取结果
	if ( ret == AIRKISS_STATUS_CHANNEL_LOCKED)
	{
		os_timer_disarm(&channel_timer);
		smart_lock_flag = 1;
		os_printf("ch locked\r\n");
	}
	else if ( ret == AIRKISS_STATUS_COMPLETE )
	{
		os_printf("airkiss complete\r\n");
		os_printf("airkiss version:%s\r\n",airkiss_version());
		airkiss_finish();
		wifi_promiscuous_enable(0);//关闭混杂模式，平台相关
	}
	else if(ret<0)
	{
		err_flag = 1;
		os_printf("ret=%d\r\n",ret);
	}
}
#endif

/******************************************************************************
 * FunctionName : channel_timer_cb
 * Description  : change channel to sniffer the current packet.
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
channel_timer_cb(void *arg)
{
  uint8 i;
  static uint32 cnt=0;
  //1.th If find one channel send smartdata,lock on this.
  cnt++;
  if(cnt&0x01)
	  deploy_led_output(1);
  else
	  deploy_led_output(0);
  if(cnt == 200)
  {
	  cnt=0;
	  yy_bf(WIFI_CFG);
  }


  if( smart_channel > 0 && smart_lock_flag == 1)
  {
	os_timer_disarm(&channel_timer);
    //wifi_set_channel(smart_channel);
    //RakPrint("[smart] locked Smartlink channel=%d\n",smart_channel);
    return;
  }

#if 0
  if(err_flag != 0)
  {
	  err_flag = 0;
	  start_airkiss();
  }
#endif
  //2.th scan channel by timer 
  for (i = current_channel; i < 14; i++) {
    if ((channel_bits & (1 << i)) != 0) {
      current_channel = i + 1;

#if USE_RAK == 0
      airkiss_change_channel(&akcontex);//清缓存
#endif
      wifi_set_channel(i);

       RakPrint("[smart] current channel2 %d---%d\n", i, system_get_time());
      os_timer_arm(&channel_timer, SCAN_TIME, 0);
      break;
    }
  }

  if (i == 14) {
    current_channel = 1;
    for (i = current_channel; i < 14; i++) {
      if ((channel_bits & (1 << i)) != 0) {
        current_channel = i + 1;
#if USE_RAK == 0
    	airkiss_change_channel(&akcontex);//清缓存
#endif
        wifi_set_channel(i);

         RakPrint("[smart] current channel3%d---%d\n", i, system_get_time());
        os_timer_arm(&channel_timer, SCAN_TIME, 0);
        break;
      }
    }
  }
}

/******************************************************************************
 * FunctionName : wifi_scan_done
 * Description  : after scan done ,get the ap info, start scan channel
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
wifi_scan_done(void *arg, STATUS status)
{
  uint8 ssid[33];

  channel_bits = 0;
  current_channel = 1;

  struct router_info *info = NULL;
  
  //1.th check the router list. 
  while((info = SLIST_FIRST(&router_list)) != NULL){
    SLIST_REMOVE_HEAD(&router_list, next);

    os_free(info);
  }

  //2.th get the bss info. 
  if (status == OK) {
    uint8 i;
    struct bss_info *bss = (struct bss_info *)arg;

    while (bss != NULL) {
      os_memset(ssid, 0, 33);

      if (os_strlen(bss->ssid) <= 32) {
        os_memcpy(ssid, bss->ssid, os_strlen(bss->ssid));
      } else {
        os_memcpy(ssid, bss->ssid, 32);
      }

      if (bss->channel != 0) {
        struct router_info *info = NULL;

        RakPrint("[smart] ssid %s, channel %d, authmode %d, rssi %d\n",
            ssid, bss->channel, bss->authmode, bss->rssi);

        RakPrint(MACSTR "\n", MAC2STR(bss->bssid));

        channel_bits |= 1 << (bss->channel);

        info = (struct router_info *)os_zalloc(sizeof(struct router_info));
        info->authmode = bss->authmode;
        info->channel = bss->channel;
        os_memcpy(info->ssid,bss->ssid,32);
        os_memcpy(info->bssid, bss->bssid, 6);

        SLIST_INSERT_HEAD(&router_list, info, next);
      }
      bss = STAILQ_NEXT(bss, next);
    }

    for (i = current_channel; i < 14; i++) {
      if ((channel_bits & (1 << i)) != 0) {
        current_channel = i + 1;
        wifi_set_channel(i);
        RakPrint("[smart] current channel1 %d---%d\n", i, system_get_time());
        break;
      }
    }
#if USE_RAK == 0
    start_airkiss();
#endif
    //3.th start sniffer and scan channel. 
    wifi_promiscuous_enable(1);
    //wifi_set_promiscuous_rx_cb(wifi_promiscuous_rx);
    wifi_set_promiscuous_rx_cb(rak_promiscuous_rx);

    os_timer_disarm(&channel_timer);
    os_timer_setfn(&channel_timer, channel_timer_cb, NULL);
    os_timer_arm(&channel_timer, SCAN_TIME, 0);
  } else {
    RakPrint("[smart] err, scan status %d\n", status);
  }
}


/******************************************************************************
 * FunctionName : smartconfig_init
 * Description  : smartconfig_init
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
smartconfig_init(void)
{
  //1.th init the router info 
	//struct smart_pkt * pkt;
	//pkt = (struct smart_pkt *)os_malloc(sizeof(struct smart_pkt));
	RakPrint("rak smart config start\n");
	SLIST_INIT(&router_list);



  //2.th scan wifi 
	wifi_station_scan(NULL,wifi_scan_done);
}


/******************************************************************************
 * FunctionName : smartconfig_end
 * Description  : smartconfig_end
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
smartconfig_end()
{
  wifi_promiscuous_enable(0);
  //os_free(pkt);
}

