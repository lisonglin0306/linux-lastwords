
#ifndef	__LASTWORDS_RECORD__
#define __LASTWORDS_RECORD__

/* ��¼�������� */
enum {
	LAST_WORDS_ATTR_UNSPEC = 0,
	LAST_WORDS_ATTR_USER,			/* �û�ǿ�ƴ��� */	
	LAST_WORDS_ATTR_NETADR,			/* �����豸��ַ�ı� */
	LAST_WORDS_ATTR_MODULE,			/* �ں˼���ж��ģ�� */
	LAST_WORDS_ATTR_REBOOT,			/* �豸ָ��������(reboot) */
	LAST_WORDS_ATTR_PANIC,			/* �ں�panic(kernel panic) */
	LAST_WORDS_ATTR_DIE,			/* �ں�die(kernel die) */
	LAST_WORDS_ATTR_OOM,			/* �ں�oom(kernel oom) */
	__LAST_WORDS_ATTR_MAX,
};

#define LAST_WORDS_ATTR_MAX (__LAST_WORDS_ATTR_MAX - LAST_WORDS_ATTR_UNSPEC - 1)

/* ��¼�������� */
enum {
	LAST_WORDS_RECD_UNSPEC = 100,
	LAST_WORDS_RECD_TRIGGER,		/* �û���Ϣ */
	LAST_WORDS_RECD_TIME,			/* ϵͳʱ�� */
	LAST_WORDS_RECD_SYSINFO,		/* ϵͳ��Ϣ */
	LAST_WORDS_RECD_DMESG,			/* dmesg��Ϣ */
	LAST_WORDS_RECD_PS,				/* ps������Ϣ */
	LAST_WORDS_RECD_BACKTRACE,		/* back trace��Ϣ */
	__LAST_WORDS_RECD_MAX,
};

#define LAST_WORDS_RECD_MAX (__LAST_WORDS_RECD_MAX - LAST_WORDS_RECD_UNSPEC - 1)


struct lws_head {
	__u16 lh_magic;				/* ħ�� */
	__u16 lh_ecc;				/* ECCУ���� */
	__u32 lh_len;				/* ��¼��Ϣ����(������Ϣͷ) */
	__u32 reserved;				/* �������� */
} __attribute__((aligned(4)));

struct lws_attr {
	__u16 type;					/* ��¼�������� */
	__u16 len;					/* ��¼���ݳ���(��������ͷ) */
} __attribute__((aligned(4)));


struct lws_attr * lastwords_prepare_attr(__u16 attr_type);
int lastwords_print(const char *fmt, ...);
void lastwords_end_attr(struct lws_attr *plwa);
__u32 lastwords_dump_record(char *buf, __u32 len, __u32 off);
__u32 lastwords_get_recordlen(void);
void lastwords_format_record(void);

#endif

