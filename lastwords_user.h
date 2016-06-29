
#ifndef	__LASTWORDS_USER__
#define __LASTWORDS_USER__

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

#define LAST_WORDS_ATTR_MAX 		(__LAST_WORDS_ATTR_MAX - LAST_WORDS_ATTR_UNSPEC - 1)
#define LAST_WORDS_ATTR_N(num)		((num) - LAST_WORDS_ATTR_UNSPEC - 1)

/* ��¼�������� */
enum {
	LAST_WORDS_RECD_UNSPEC = 100,
	LAST_WORDS_RECD_TRIGGER,		/* ������Ϣ */
	LAST_WORDS_RECD_TIME,			/* ϵͳʱ�� */
	LAST_WORDS_RECD_SYSINFO,		/* ϵͳ��Ϣ */
	LAST_WORDS_RECD_DMESG,			/* dmesg��Ϣ */
	LAST_WORDS_RECD_PS,				/* ps������Ϣ */
	LAST_WORDS_RECD_BACKTRACE,		/* back trace��Ϣ */
	__LAST_WORDS_RECD_MAX,
};

#define LAST_WORDS_RECD_MAX			(__LAST_WORDS_RECD_MAX - LAST_WORDS_RECD_UNSPEC - 1)
#define LAST_WORDS_RECD_N(num)		((num) - LAST_WORDS_RECD_UNSPEC - 1)

struct lws_head {
	unsigned short lh_magic;			/* ħ�� */
	unsigned short lh_ecc;				/* ECCУ���� */
	unsigned int lh_len;				/* ��¼��Ϣ����(������Ϣͷ) */
	unsigned int reserved;				/* �������� */
} __attribute__((aligned(4)));

struct lws_attr {
	unsigned short type;				/* ��¼�������� */
	unsigned short len;					/* ��¼���ݳ���(��������ͷ) */
} __attribute__((aligned(4)));


#define LASTWORDS_MAGIC			'L'

/* ��ȡͷ����Ϣ */
#define LASTWORDS_READ_HEAD			_IOR(LASTWORDS_MAGIC, 0x00, struct lws_head)
/* ��ȡ�ڴ�ռ��С */
#define LASTWORDS_GET_MEMSIZE		_IOR(LASTWORDS_MAGIC, 0x01, unsigned int)
/* ��ʽ���ڴ�ռ� */
#define LASTWORDS_FORMAT_MEM		_IO(LASTWORDS_MAGIC, 0x02)
/* ǿ���û�������¼ */
#define LASTWORDS_TRIGGER_RECORD	_IOW(LASTWORDS_MAGIC, 0x03, char *)

#endif

