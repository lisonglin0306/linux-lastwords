#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/types.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include "lastwords.h"
#include "lastwords_record.h"
#include "lastwords_mem.h"

struct lastwords_head {
	__u16 lh_magic;				/* ħ�� */
	__u16 lh_ecc;				/* ECCУ���� */
	__u32 lh_len;				/* ��¼��Ϣ����(������Ϣͷ) */
	__u32 reserved;				/* �������� */
} __attribute__((aligned(4)));

struct lastwords_attr {
	__u16 type;					/* ��¼�������� */
	__u16 len;					/* ��¼���ݳ���(��������ͷ) */
} __attribute__((aligned(4)));

/* ��¼����ħ�� */
#define LWRECD_MAGIC_NUM		0x5818

/* �������Լ�¼���� */
#define LWRECD_ATTR_SIZE_MAX	2048

/* ��¼���ݽṹ����� */
#define LWRECD_ALIGNTO		4U
#define LWRECD_ALIGN(len)	(((len)+LWRECD_ALIGNTO-1) & ~(LWRECD_ALIGNTO-1))	/* ���ݶ��� */

#define LWRECD_HDRLEN		((int) LWRECD_ALIGN(sizeof(struct lastwords_head)))	/* ����ͷռ�ó��� */
#define LWRECD_LENGTH(len)	((len) + LWRECD_HDRLEN)								/* �����ܳ�(��������ͷ) */
#define LWRECD_DATA(lwh)	((void*)(((char*)lwh) + LWRECD_HDRLEN))				/* ��ȡ�����غɵ�ַ */
#define LWRECD_NEXT(lwh)	((void *)(((char*)(lwh)) + LWRECD_ALIGN((lwh)->lh_len)))	/* ��ȡ����β */
#define LWRECD_PAYLOAD(lwh)	((lwh)->lh_len - LWRECD_HDRLEN)						/* �����غɳ��� */

#define LWA_HDRLEN			((int) LWRECD_ALIGN(sizeof(struct lastwords_attr)))	/* ����ͷ���� */
#define LWA_LENGTH(len)		((len) + LWA_HDRLEN)								/* �����ܳ�(��������ͷ) */
#define LWA_DATA(lwa)		((void *)((char*)(lwa) + LWA_HDRLEN))				/* ��ȡ�����غ��׵�ַ */
#define LWA_CDATA(lwa,len)	((void *)((char*)(lwa) + LWA_HDRLEN + len))			/* ��ȡ�����غ�ָ����ַ */
#define LWA_PAYLOAD(len)	(len - LWA_HDRLEN)									/* �����غɳ��� */


static char *record_name[LAST_WORDS_ATTR_MAX] = {
		"attr_user_info",				/* LAST_WORDS_ATTR_USER */
		"attr_net_addr_info",			/* LAST_WORDS_ATTR_NETADR */
		"attr_module_info",				/* LAST_WORDS_ATTR_MODULE */
		"attr_reboot_info",				/* LAST_WORDS_ATTR_REBOOT */
		"attr_panic_info",				/* LAST_WORDS_ATTR_PANIC */
};

static void __iomem *record_base = NULL;
static int record_size = 0;
static struct lastwords_attr *pcur_lwa = NULL;
static DEFINE_SPINLOCK(record_lock);


/* ******************************************
 * �� �� ��: lastwords_format_record
 * ��������: �������Ը�ʽ�����ݼ�¼��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
static void lastwords_format_record(void)
{
	struct lastwords_head lwh = {0};
	unsigned long flag = 0;

	spin_lock_irqsave(&record_lock, flag);
	
	/* ��������� */
	lastwords_clean_mem();

	/* ��ʽ������������������ͷ */
	lwh.lh_magic = LWRECD_MAGIC_NUM;
	lwh.lh_len = LWRECD_HDRLEN;
	lwh.lh_ecc = 0;			/* todo */

	memcpy(record_base, &lwh, sizeof(lwh));

	spin_unlock_irqrestore(&record_lock, flag);
}

/* ******************************************
 * �� �� ��: lastwords_format_record
 * ��������: �������Ը�ʽ�����ݼ�¼��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
static void lastwords_delete_top_record(void)
{
	struct lastwords_head *plwh = record_base;
	struct lastwords_attr *plwa = NULL, *plwan = NULL;
	int lw_len = 0, lwa_len = 0;

	lw_len = LWRECD_PAYLOAD(plwh);
	plwa = (struct lastwords_attr *) LWRECD_DATA(plwh);
	
	if (0 == lw_len) 
		return;

	lwa_len = LWRECD_ALIGN(plwa->len);
	if (lw_len > lwa_len) {
		plwan = (struct lastwords_attr *)((char *)plwa + lwa_len);
		lw_len -= lwa_len;
		memmove(plwa, plwan, lw_len);
	}

	plwh->lh_len -= lwa_len;
	plwh->lh_ecc = 0;	/* to do */

	if (NULL != pcur_lwa)
		pcur_lwa = (struct lastwords_attr *)((char *)pcur_lwa - lwa_len);

	return;
}

/* ******************************************
 * �� �� ��: lastwords_prepare_attr
 * ��������: ��������׼����ʼд��������(��ʼ��ͷ��)
 * �������: lw_attr_t attr_type: ��������
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_prepare_attr(lw_attr_t attr_type)
{
	struct lastwords_head *plwh = record_base;
	struct lastwords_attr *plwa = NULL;
	unsigned long flag = 0;

	spin_lock_irqsave(&record_lock, flag);

	/* ���ڴ�ռ䲻��,��ѭ��ɾ������ļ�¼�� */
	while ((plwh->lh_len + LWA_HDRLEN) > record_size)
		lastwords_delete_top_record();

	/* ��ʼ����¼��ͷ����������ͷ���� */
	plwa = (struct lastwords_attr *) LWRECD_NEXT(plwh);
	plwa->type = attr_type;
	plwa->len = LWA_HDRLEN;

	plwh->lh_len += LWRECD_ALIGN(plwa->len);

	/* ���浱ǰ��¼ͷ */
	pcur_lwa = plwa;
	
	spin_unlock_irqrestore(&record_lock, flag);
}

/* ******************************************
 * �� �� ��: lastwords_prepare_attr
 * ��������: ��������׼����ʼд��������(��ʼ��ͷ��)
 * �������: lw_attr_t attr_type: ��������
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
int lastwords_print_attr(const char *fmt, ...)
{
	struct lastwords_head *plwh = record_base;
	struct lastwords_attr *plwa = NULL;	
	int payload = 0;
	int size = LWRECD_ATTR_SIZE_MAX;
	char *buf = NULL;	
	va_list args;
	unsigned long flag = 0;
	int ret = 0;

	spin_lock_irqsave(&record_lock, flag);

	if (NULL == pcur_lwa) {
		pr_err("Lastwords current attr is not prepared\n");
		ret = 0;
		goto out;
	}

	buf = vzalloc(size);
	if (NULL == buf) {
		ret = -ENOMEM;
		goto out;
	}
	
	va_start(args, fmt);
	size = vscnprintf(buf, size, fmt, args);
	va_end(args);
	
	/* add '\0' end */
	size++;	

	/* ���ݳ��ȱ��� */
	if ((LWRECD_HDRLEN + LWRECD_ALIGN(LWA_LENGTH(size))) > record_size) {
		pr_err("Lastword record attr size = %d too large!\n", size);
		vfree(buf);
		ret = -ENOBUFS;
		goto out;		
	}

	/* ���ڴ�ռ䲻��,��ѭ��ɾ������ļ�¼�� */
	while ((plwh->lh_len + LWRECD_ALIGN(LWA_LENGTH(size))) > record_size)
		lastwords_delete_top_record();

	/* д������ */
	plwa = pcur_lwa;
	payload = LWA_PAYLOAD(plwa->len);
	memcpy(LWA_CDATA(plwa,payload), buf, size);

	plwh->lh_len -= LWRECD_ALIGN(plwa->len);
	plwa->len += size;
	plwh->lh_len += LWRECD_ALIGN(plwa->len);

	vfree(buf);
	ret = size;

out:
	spin_unlock_irqrestore(&record_lock, flag);
	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_end_record
 * ��������: �������Խ���һ��д��������
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_end_attr(void)
{
	struct lastwords_head *plwh = record_base;
	unsigned long flag = 0;

	spin_lock_irqsave(&record_lock, flag);	
	
	plwh->lh_ecc = 0;	/* to do */
	pcur_lwa = NULL;
	
	spin_unlock_irqrestore(&record_lock, flag);
}

/* ******************************************
 * �� �� ��: lastwords_write_record
 * ��������: �����������һ��������д��������
 * �������: lw_attr_t attr_type: ��������
 			 char *buf: ��������
 			 int len: ���Գ���
 * �������: void	
 * �� �� ֵ: �ɹ�����д�볤�ȣ�ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_write_record(lw_attr_t attr_type, char *buf, int len)
{
	struct lastwords_head *plwh = record_base;
	struct lastwords_attr *plwa = NULL;
	unsigned long flag = 0;
	int ret;

	spin_lock_irqsave(&record_lock, flag);	

	if ((LWRECD_HDRLEN + LWRECD_ALIGN(LWA_LENGTH(len))) > record_size) {
		pr_err("Lastword record attr size = %d too large!\n", len);
		ret = -ENOBUFS;
		goto out;
	}

	/* ���ڴ�ռ䲻��,��ѭ��ɾ������ļ�¼�� */
	while ((plwh->lh_len + LWRECD_ALIGN(LWA_LENGTH(len))) > record_size)
		lastwords_delete_top_record();
	
	plwa = (struct lastwords_attr *) LWRECD_NEXT(plwh);
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
int lastwords_print_record(lw_attr_t attr_type, const char *fmt, ...)
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

/* ******************************************
 * �� �� ��: lastwords_show_head
 * ��������: �������Դ�ӡ����ͷ������
 * �������: void
 * �������: void
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_show_head(void)
{
	struct lastwords_head *plwh = record_base;

	pr_info("/============Lastwords Heard Info============/\n");
	pr_info("Magic Num = 0x%x\n", plwh->lh_magic);
	pr_info("ECC Value= 0x%x\n", plwh->lh_ecc);
	pr_info("Record Len = 0x%x\n", plwh->lh_len);
	pr_info("/======================================/\n");

}

/* ******************************************
 * �� �� ��: lastwords_show_record
 * ��������: �������Դ�ӡ��������
 * �������: lw_attr_t attr_type: ��������
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_show_record(lw_attr_t attr_type)
{
	struct lastwords_head *plwh = record_base;
	struct lastwords_attr *plwa = NULL;	
	int len = 0, lw_len = 0;

	lw_len = LWRECD_PAYLOAD(plwh);
	plwa = (struct lastwords_attr *) LWRECD_DATA(plwh);

	/* ���ͷ����Ϣ */
	lastwords_show_head();

	/* ѭ�����ָ�����Եļ�¼��Ϣ */
	while (len < lw_len) {
		len += LWRECD_ALIGN(plwa->len);
		if (plwa->type == attr_type || __LAST_WORDS_ATTR_MAX == attr_type) {
			pr_info("/-------------%s-------------/\n", record_name[plwa->type-1]);
			pr_info("Attr Type = %d\n", plwa->type);
			pr_info("Attr Len = 0x%x\n\n", plwa->len);
			pr_info("%s\n", (char *) LWA_DATA(plwa));
			pr_info("/--------------------------------/\n");
		}
		plwa = (struct lastwords_attr *) (LWRECD_DATA(plwh) + len);
	}
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
	struct lastwords_head *plwh = NULL;
	
	/* 1����ȡ�ڴ����ַ�ͳ��� */
	record_base = lastwords_get_membase();
	record_size = lastwords_get_memsize();

	if (NULL == record_base || LWRECD_HDRLEN > record_size) {
		pr_err("Lastwords do not have enough record memory\n");
		return -ENOMEM;
	}

	/* 2����ʽ���洢�ռ�(���Ѵ����򲻸�ʽ��) */
	plwh = (struct lastwords_head *) record_base;
	
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

}

