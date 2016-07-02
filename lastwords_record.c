#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/types.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include "lastwords.h"
#include "lastwords_record.h"
#include "lastwords_mem.h"

/* ��¼����ħ�� */
#define LWRECD_MAGIC_NUM		0x5818

/* �������Լ�¼���� */
#define LWRECD_ATTR_SIZE_MAX	2048

/* ��¼���ݽṹ����� */
#define LWRECD_ALIGNTO		4U
#define LWRECD_ALIGN(len)	(((len)+LWRECD_ALIGNTO-1) & ~(LWRECD_ALIGNTO-1))	/* ���ݶ��� */

#define LWRECD_HDRLEN		((int) LWRECD_ALIGN(sizeof(struct lws_head)))		/* ����ͷռ�ó��� */
#define LWRECD_LENGTH(len)	((len) + LWRECD_HDRLEN)								/* �����ܳ�(��������ͷ) */
#define LWRECD_DATA(lwh)	((void *)(((char*)lwh) + LWRECD_HDRLEN))			/* ��ȡ�����غɵ�ַ */
#define LWRECD_NEXT(lwh)	((void *)(((char*)(lwh)) + LWRECD_ALIGN((lwh)->lh_len)))	/* ��ȡ����β */
#define LWRECD_PAYLOAD(lwh)	((lwh)->lh_len - LWRECD_HDRLEN)						/* �����غɳ��� */

#define LWA_HDRLEN			((int) LWRECD_ALIGN(sizeof(struct lws_attr)))		/* ����ͷ���� */
#define LWA_LENGTH(len)		((len) + LWA_HDRLEN)								/* �����ܳ�(��������ͷ) */
#define LWA_DATA(lwa)		((void *)((char*)(lwa) + LWA_HDRLEN))				/* ��ȡ�����غ��׵�ַ */
#define LWA_PAYLOAD(len)	(len - LWA_HDRLEN)									/* �����غɳ��� */

static void __iomem *record_base = NULL;
static int record_size = 0;
static DEFINE_SPINLOCK(record_lock);


/* ******************************************
 * �� �� ��: lastwords_prepare_attr
 * ��������: ��������׼����ʼд��������(��ʼ��ͷ��)
 * �������: __u16 attr_type: ��������
 * �������: void	
 * �� �� ֵ: struct lws_attr * ���������ɵ�attrͷ��
 * ****************************************** */
struct lws_attr * lastwords_prepare_attr(__u16 attr_type)
{
	struct lws_head *plwh = record_base;
	struct lws_attr *plwa = NULL;
	unsigned long flag = 0;

	spin_lock_irqsave(&record_lock, flag);	

	/* ���ݳ��ȱ��� */
	if ((plwh->lh_len + LWA_HDRLEN) > record_size) {
		pr_err("Lastwords record mem not enough\n");
		goto out;
	}

	/* ��ʼ����¼��ͷ����������ͷ���� */
	plwa = (struct lws_attr *) LWRECD_NEXT(plwh);
	plwa->type = attr_type;
	plwa->len = LWA_HDRLEN;

	plwh->lh_len += LWRECD_ALIGN(plwa->len);
	
out:
	spin_unlock_irqrestore(&record_lock, flag);
	return plwa;
}

/* ******************************************
 * �� �� ��: lastwords_print
 * ��������: ��������׼����ʼд��������(��ʼ��ͷ��)
 * �������: lw_attr_t attr_type: ��������
 * �������: void	
 * �� �� ֵ: int д����ֽ���
 * ****************************************** */
int lastwords_print(const char *fmt, ...)
{
	struct lws_head *plwh = record_base;
	__u32 size = LWRECD_ATTR_SIZE_MAX;
	unsigned long flag = 0;
	char *buf = NULL;	
	va_list args;
	int ret = 0;

	buf = vzalloc(size);
	if (NULL == buf) 
		return -ENOMEM;
	
	va_start(args, fmt);
	size = vscnprintf(buf, size, fmt, args);
	va_end(args);
	
	/* add '\0' end */
	//size++;	

	spin_lock_irqsave(&record_lock, flag);	

	/* ���ݳ��ȱ��� */
	if ((plwh->lh_len + LWRECD_ALIGN(size)) > record_size) {
		pr_err("Lastwords record mem not enough\n");
		ret = -ENOMEM;
		goto out;
	}

	/* д������ */
	memset(((char*)plwh + plwh->lh_len), 0, size);
	memcpy(((char*)plwh + plwh->lh_len), buf, size);

	plwh->lh_len += size;
	ret = size;

out:
	spin_unlock_irqrestore(&record_lock, flag);
	vfree(buf);
	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_end_record
 * ��������: �������Խ���һ��д��������
 * �������: struct lws_attr *plwa: ����ͷ
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_end_attr(struct lws_attr *plwa)
{
	struct lws_head *plwh = record_base;	
	char *p = NULL;
	__u32 payload = 0;
	unsigned long flag = 0;	
	int i = 0;

	spin_lock_irqsave(&record_lock, flag);	

	/* ���³�����Ϣ */
	plwa->len = ((char*)plwh + plwh->lh_len) - (char *)plwa;

	/* ����attr�е�'\0'ת��Ϊ'\n'(����������'\0') */
	payload = LWA_PAYLOAD(plwa->len);
	p = (char*)LWA_DATA(plwa);

	pr_debug("attr payload=%d\n", payload);
	
	if (0 < payload && LAST_WORDS_RECD_UNSPEC < plwa->type &&
			__LAST_WORDS_RECD_MAX > plwa->type) {
		for (i = 0; i < payload-1; i++) {
			*p = ('\0' == *p) ? ' ' : *p;
			p++;
		}
		*p = '\0';
	}

	/* ���²��� */
	plwh->lh_len = LWRECD_ALIGN(plwh->lh_len);
	plwh->lh_ecc = 0;	/* to do */
	
	spin_unlock_irqrestore(&record_lock, flag);
}

#if 0
/* ******************************************
 * �� �� ��: lastwords_write_record
 * ��������: �����������һ��������д��������
 * �������: __u16 attr_type: ��������
 			 char *buf: ��������
 			 __u16 len: ���Գ���
 * �������: void	
 * �� �� ֵ: �ɹ�����д�볤�ȣ�ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_write_record(__u16 attr_type, char *buf, __u16 len)
{
	struct lws_head *plwh = record_base;
	struct lws_attr *plwa = NULL;
	unsigned long flag = 0;	
	int ret;

	spin_lock_irqsave(&record_lock, flag);	

	/* ���ݳ��ȱ��� */
	if ((plwh->lh_len + LWRECD_ALIGN(len)) > record_size) {
		pr_err("Lastwords record mem not enough\n");
		ret = -ENOMEM;
		goto out;
	}

	plwa = (struct lws_attr *) LWRECD_NEXT(plwh);
	plwa->type = attr_type;
	plwa->len = LWA_LENGTH(len);

	plwh->lh_len += LWRECD_ALIGN(plwa->len);
	plwh->lh_ecc = 0;	/* to do */

	/* Ϊ���ڴ�������һ��д���ڴ����ݿռ� */
	memcpy(LWA_DATA(plwa), buf, len);
	ret = len;

out:
	spin_unlock_irqrestore(&record_lock, flag);
	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_print_record
 * ��������: �����������һ������д��������
 * �������: lw_attr_t attr_type: ��������
 			 const char *fmt: ��������
 * �������: void	
 * �� �� ֵ: int д�볤��
 * ****************************************** */
int lastwords_print_record(__u16 attr_type, const char *fmt, ...)
{
	char *buf = NULL;
	int size = LWRECD_ATTR_SIZE_MAX;
	va_list args;
	int ret;

	buf = vzalloc(size);
	if (NULL == buf)
		return -ENOMEM;
	
	va_start(args, fmt);
	size = vscnprintf(buf, size, fmt, args);
	va_end(args);

	ret = lastwords_write_record(attr_type, buf, size+1);

	vfree(buf);

	return ret;
}
#endif




/* ******************************************
 * �� �� ��: lastwords_format_record
 * ��������: �������Ը�ʽ�����ݼ�¼��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_format_record(void)
{
	struct lws_head lwh = {0};
	unsigned long flag = 0;
	
	spin_lock_irqsave(&record_lock, flag);	
	
	/* ��������� */
	lastwords_clean_mem();

	/* ��ʽ������������������ͷ */
	lwh.lh_magic	= LWRECD_MAGIC_NUM;
	lwh.lh_len		= LWRECD_HDRLEN;
	lwh.lh_ecc		= 0;			/* to do */

	memcpy(record_base, &lwh, sizeof(lwh));

	spin_unlock_irqrestore(&record_lock, flag);
}

/* ******************************************
 * �� �� ��: lastwords_dump_record
 * ��������: �������Ի�ȡ���м�¼��Ϣ
 * �������: __u32 len: �����С
 			 __u32 off: ƫ�Ƴ���
 * �������: char *buf: ����ռ�
 * �� �� ֵ: __u32: ���ػ�ȡ����
 * ****************************************** */
__u32 lastwords_dump_record(char *buf, __u32 len, __u32 off)
{
	struct lws_head *plwh = record_base;
	__u32 recordlen = 0;
	unsigned long flag = 0;
	
	spin_lock_irqsave(&record_lock, flag);	
	
	recordlen = plwh->lh_len;
	if ((off + len) > recordlen) {
		spin_unlock_irqrestore(&record_lock, flag);
		return 0;
	}
	
	len = (len < (recordlen - off)) ? len : (recordlen - off);
	memcpy(buf, record_base + off, len);

	spin_unlock_irqrestore(&record_lock, flag);
	return len;
}

/* ******************************************
 * �� �� ��: lastwords_dump_record
 * ��������: �������Ի�ȡ���м�¼��Ϣ
 * �������: __u32 len: �����С
 * �������: char *buf: ����ռ�
 * �� �� ֵ: void: �ɹ�����0
 * ****************************************** */
__u32 lastwords_get_recordlen(void)
{
	struct lws_head *plwh = record_base;	
	__u32 record_len = 0;	
	unsigned long flag = 0;
	
	spin_lock_irqsave(&record_lock, flag);	
	record_len = plwh->lh_len;
	spin_unlock_irqrestore(&record_lock, flag);

	return record_len;
}



/* ******************************************
 * �� �� ��: lastwords_init_record
 * ��������: �������Գ�ʼ�����ݴ洢����
 * �������: void
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_init_record(void)
{
	struct lws_head *plwh = NULL;
	
	/* 1����ȡ�ڴ����ַ�ͳ��� */
	record_base = lastwords_get_membase();
	record_size = lastwords_get_memsize();

	if (NULL == record_base || LWRECD_HDRLEN > record_size) {
		pr_err("Lastwords do not have enough record memory\n");
		return -ENOMEM;
	}

	/* 2����ʽ���洢�ռ�(���Ѵ����򲻸�ʽ��) */
	plwh = (struct lws_head *) record_base;
	
	if (LWRECD_MAGIC_NUM != plwh->lh_magic) 
		lastwords_format_record();
	
	return 0;
}

/* ******************************************
 * �� �� ��: lastwords_exit_record
 * ��������: ��������ȥ��ʼ�����ݴ洢����
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_exit_record(void)
{
	/* to do */
}

