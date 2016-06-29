
#ifndef	__LASTWORDS_EXPORT__
#define __LASTWORDS_EXPORT__


#define LASTWORDS_BIT(n)				(1 << n)
#define LASTWORDS_EXPORT_TIME			LASTWORDS_BIT(0)		/* ϵͳʱ�� */
#define LASTWORDS_EXPORT_SYSINFO		LASTWORDS_BIT(1)		/* ϵͳ��Ϣ */
#define LASTWORDS_EXPORT_DMESG			LASTWORDS_BIT(2)		/* dmesg��Ϣ */
#define LASTWORDS_EXPORT_PS 			LASTWORDS_BIT(3)		/* ps������Ϣ */
#define LASTWORDS_EXPORT_BACKTRACE		LASTWORDS_BIT(4)		/* backtrace��Ϣ */

int lastwords_export_attr(__u16 attr_type, __u32 export_type, char *trigger);
void lastwords_export_clear(void);
__u32 lastwords_export_dump(char *buf, __u32 len, __u32 off);

#endif

