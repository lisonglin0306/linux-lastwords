#include <asm/io.h>

#include "lastwords.h"


/* ʹ���ڴ�ռ�(0x1E000000~0x2000000)��32MB�ռ� */
#define LASTWORDS_MEM_ADDR		0x1E000000
#define LASTWORDS_MEM_SIZE		0x60//0x2000000			/* 32MB */

static phys_addr_t phyaddr = (phys_addr_t)LASTWORDS_MEM_ADDR;
static int memsize = LASTWORDS_MEM_SIZE;
static void __iomem *memaddr = NULL;



/* ******************************************
 * �� �� ��: lastwords_clean_mem
 * ��������: ������������ڴ������ڴ�
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_clean_mem(void)
{
	if (memaddr)
		memset(memaddr, 0, memsize);
}

/* ******************************************
 * �� �� ��: lastwords_get_membase
 * ��������: �������Ի�ȡ�ڴ��������ַ
 * �������: void
 * �������: void	
 * �� �� ֵ: �ɹ����ػ���ַ��ʧ�ܷ���NULL
 * ****************************************** */
void * lastwords_get_membase(void)
{
	return memaddr;
}

/* ******************************************
 * �� �� ��: lastwords_get_memsize
 * ��������: �������Ի�ȡ�ڴ泤��
 * �������: void
 * �������: void	
 * �� �� ֵ: �����ڴ泤��
 * ****************************************** */
int lastwords_get_memsize(void)
{
	return memsize;
}

/* ******************************************
 * �� �� ��: lastwords_init_mem
 * ��������: ���������ڴ��ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_init_mem(void)
{
	/* ӳ���ڴ��ַ�ռ� */
	memaddr = ioremap_nocache(phyaddr, memsize);
	if (NULL == memaddr) {
		pr_err("failed to allocate char dev region\n");
		return -ENOMEM;
	}	

	pr_info("mem phyaddr = 0x%x, size = 0x%x, "
				"viraddr = 0x%p\n", phyaddr, memsize, memaddr);

	return 0;
}

/* ******************************************
 * �� �� ��: lastwords_exit_mem
 * ��������: ���������ڴ�ȥ��ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_exit_mem(void)
{
	/* ����ڴ�ӳ�� */
	if (memaddr) {
		iounmap(memaddr);
		memaddr = NULL;
	}
}

