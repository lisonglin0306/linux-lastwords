#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/utsname.h>

#include "lastwords.h"
#include "lastwords_record.h"
#include "lastwords_sym.h"
#include "lastwords_export.h"

/* ��Ϣ��¼����ָ�� */
typedef int (*lastwords_export_func) (void *data); 

/* ϵͳʱ��ṹ */
struct lws_tm {
	int tm_sec;		/* seconds */
	int tm_min;		/* minutes */
	int tm_hour;	/* hours */
	int tm_mday;	/* day of the month */
	int tm_mon;		/* month */
	int tm_year;	/* year */
};

static DEFINE_SPINLOCK(export_lock);

/* ******************************************
 * �� �� ��: lws_gmtime
 * ��������: ��������ת��ϵͳʱ��(��ֲ���ں�kdb_gmtime()����)
 * �������: struct timespec *tv
 * �������: struct lws_tm *tm
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static void lws_gmtime(struct timespec *tv, struct lws_tm *tm)
{
	/* This will work from 1970-2099, 2100 is not a leap year */
	static int mon_day[] = { 31, 29, 31, 30, 31, 30, 31,
				 31, 30, 31, 30, 31 };
	memset(tm, 0, sizeof(*tm));
	tm->tm_sec  = tv->tv_sec % (24 * 60 * 60);
	tm->tm_mday = tv->tv_sec / (24 * 60 * 60) +
		(2 * 365 + 1); /* shift base from 1970 to 1968 */
	tm->tm_min =  tm->tm_sec / 60 % 60;
	tm->tm_hour = tm->tm_sec / 60 / 60;
	tm->tm_sec =  tm->tm_sec % 60;
	tm->tm_year = 68 + 4*(tm->tm_mday / (4*365+1));
	tm->tm_mday %= (4*365+1);
	mon_day[1] = 29;
	while (tm->tm_mday >= mon_day[tm->tm_mon]) {
		tm->tm_mday -= mon_day[tm->tm_mon];
		if (++tm->tm_mon == 12) {
			tm->tm_mon = 0;
			++tm->tm_year;
			mon_day[1] = 28;
		}
	}
	++tm->tm_mday;
}

/* ******************************************
 * �� �� ��: lastwords_export_time
 * ��������: �������Ի�ȡϵͳ��Ϣ(��ֲ���ں�kdb_sysinfo()����)
 * �������: void
 * �������: struct sysinfo *val
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static void lws_sysinfo(struct sysinfo *val)
{
	struct timespec uptime;
	ktime_get_ts(&uptime);
	memset(val, 0, sizeof(*val));
	val->uptime = uptime.tv_sec;
	val->loads[0] = avenrun[0];
	val->loads[1] = avenrun[1];
	val->loads[2] = avenrun[2];
	val->procs = *lw_nr_threads-1;
	si_meminfo(val);

	return;
}

/* ******************************************
 * �� �� ��: lastwords_export_trigger
 * ��������: �����������������¼��Ϣ
 * �������: void *data
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static int lastwords_export_trigger(void *data)
{
	char * triggr= (char *)data;

	lastwords_print("%s\n", triggr);
	return 0;
}

/* ******************************************
 * �� �� ��: lastwords_export_time
 * ��������: �����������ϵͳʱ�����Ϣ
 * �������: void *data
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static int lastwords_export_time(void *data) 
{
	struct timespec now;
	struct lws_tm tm;

	now = lw__current_kernel_time();
	lws_gmtime(&now, &tm);
	lastwords_print("date       %04d-%02d-%02d %02d:%02d:%02d "
		   			"tz_minuteswest %d\n",
				1900+tm.tm_year, tm.tm_mon+1, tm.tm_mday,
				tm.tm_hour, tm.tm_min, tm.tm_sec,
				sys_tz.tz_minuteswest);
	
	return 0;
}

/* ******************************************
 * �� �� ��: lastwords_export_sysinfo
 * ��������: �����������ϵͳ�Ĳ�����Ϣ
 * �������: void *data
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static int lastwords_export_sysinfo(void *data)
{
	struct timespec now;
	struct lws_tm tm;
	struct sysinfo val;

	lastwords_print("sysname    %s\n", init_uts_ns.name.sysname);
	lastwords_print("release    %s\n", init_uts_ns.name.release);
	lastwords_print("version    %s\n", init_uts_ns.name.version);
	lastwords_print("machine    %s\n", init_uts_ns.name.machine);
	lastwords_print("nodename   %s\n", init_uts_ns.name.nodename);
	lastwords_print("domainname %s\n", init_uts_ns.name.domainname);
	lastwords_print("ccversion  %s\n", __stringify(CCVERSION));

	now = lw__current_kernel_time();
	lws_gmtime(&now, &tm);
	lastwords_print("date       %04d-%02d-%02d %02d:%02d:%02d "
		   			"tz_minuteswest %d\n",
					1900+tm.tm_year, tm.tm_mon+1, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec,
					sys_tz.tz_minuteswest);

	lws_sysinfo(&val);
	lastwords_print("uptime     ");
	if (val.uptime > (24*60*60)) {
		int days = val.uptime / (24*60*60);
		val.uptime %= (24*60*60);
		lastwords_print("%d day%s ", days, days == 1 ? "" : "s");
	}
	lastwords_print("%02ld:%02ld\n", val.uptime/(60*60), (val.uptime/60)%60);

#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)
	lastwords_print("load avg   %ld.%02ld %ld.%02ld %ld.%02ld\n",
		LOAD_INT(val.loads[0]), LOAD_FRAC(val.loads[0]),
		LOAD_INT(val.loads[1]), LOAD_FRAC(val.loads[1]),
		LOAD_INT(val.loads[2]), LOAD_FRAC(val.loads[2]));
#undef LOAD_INT
#undef LOAD_FRAC
	/* Display in kilobytes */
#define K(x) ((x) << (PAGE_SHIFT - 10))
	lastwords_print("\nMemTotal:       %8lu kB\nMemFree:        %8lu kB\n"
		   "Buffers:        %8lu kB\n",
		   K(val.totalram), K(val.freeram), K(val.bufferram));
	return 0;	
}

/* ******************************************
 * �� �� ��: lastwords_export_btrace
 * ��������: �����������Backtrace����Ϣ
 * �������: void
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static int lastwords_export_btrace(void *data)
{
	return 0;
}

/* ******************************************
 * �� �� ��: lastwords_export_record
 * ��������: �����������ָ�����͵ļ�¼��Ϣ
 * �������: __u16 record_type: record����
 			 lastword_export_func *export_func: ��¼����
 			 void *data: ��������
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static int lastwords_export_record(__u16 record_type, 
									lastwords_export_func export_func, 
									void *data)
{
	struct lws_attr *plwa = NULL;
	int ret = 0;

	pr_info("Lastwords export record type:%d\n", record_type);
	
	plwa = lastwords_prepare_attr(record_type);
	if (!plwa) {
		pr_err("Lastwords prepare attr fail\n");
		return -1;
	}
		
	ret = export_func(data);
	lastwords_end_attr(plwa);

	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_export_attr
 * ��������: �����������ڴ����ָ�����͵Ĵ�����Ϣ
 * �������: __u16 attr_type: ��������
 			 __u32 export_type: ��¼��Ϣ����
 			 char *trigger: ������Ϣ
 * �������: void	
 * �� �� ֵ: int:�ɹ�����0
 * ****************************************** */
int lastwords_export_attr(__u16 attr_type, __u32 export_type, char *trigger)
{
	struct lws_attr *plwa = NULL;
	unsigned long flag = 0;
	int ret = 0;

	pr_info("Lastwords export attr type:%d\n", attr_type);
	
	spin_lock_irqsave(&export_lock, flag);

	/* ������־��� */
	plwa = lastwords_prepare_attr(attr_type);
	if (!plwa) {
		pr_err("Lastwords prepare attr fail\n");
		return -1;
	}

	/* ���������Ϣ */
	ret = lastwords_export_record(LAST_WORDS_RECD_TRIGGER, 
								lastwords_export_trigger, trigger);
	if (0 != ret) {
		pr_err("Lastwords export system time error:%d\n", ret);
		return ret;
	}
	
	/* ���ϵͳʱ�� */
	if (LASTWORDS_EXPORT_TIME & export_type) {
		ret = lastwords_export_record(LAST_WORDS_RECD_TIME, 
									lastwords_export_time, NULL);
		if (0 != ret) {
			pr_err("Lastwords export system time error:%d\n", ret);
			return ret;
		}
	}

	/* ���ϵͳ��Ϣ */
	if (LASTWORDS_EXPORT_SYSINFO & export_type) {
		ret = lastwords_export_record(LAST_WORDS_RECD_SYSINFO, 
									lastwords_export_sysinfo, NULL);
		if (0 != ret) {
			pr_err("Lastwords export system info error:%d\n", ret);
			return ret;
		}
	}

	/* ���backtrace��Ϣ */
	if (LASTWORDS_EXPORT_BACKTRACE & export_type) {
		ret = lastwords_export_record(LAST_WORDS_RECD_BACKTRACE, 
									lastwords_export_btrace, NULL);
		if (0 != ret) {
			pr_err("Lastwords export back trace error:%d\n", ret);
			return ret;
		}
	}

	/* ������־��� */
	lastwords_end_attr(plwa);

	spin_unlock_irqrestore(&export_lock, flag);

	return 0;
}

/* ******************************************
 * �� �� ��: lastwords_export_clear
 * ��������: �������������Ϣ
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_export_clear(void)
{
	unsigned long flag = 0;

	spin_lock_irqsave(&export_lock, flag);
	lastwords_format_record();
	spin_unlock_irqrestore(&export_lock, flag);	
}

/* ******************************************
 * �� �� ��: lastwords_export_clear
 * ��������: �������Ի�ȡ��¼��Ϣ
 * �������: __u32 len: �����С
 			 __u32 off: ƫ�Ƴ���
 * �������: char *buf: ����ռ�
 * �� �� ֵ: __u32: ���ػ�ȡ����
 * ****************************************** */
__u32 lastwords_export_dump(char *buf, __u32 len, __u32 off)
{
	unsigned long flag = 0;
	__u32 dump_len = 0;

	spin_lock_irqsave(&export_lock, flag);		
	dump_len = lastwords_dump_record(buf, len, off);
	spin_unlock_irqrestore(&export_lock, flag); 

	return dump_len;
}

