#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>

#include "lastwords.h"
#include "lastwords_sym.h"

/* �����������ҽṹ */
struct lws_sym {
	void **addr;		/* ������ַ */
	char *name;			/* ������ */
};

/* �ں˱������� */
int *lw_nr_threads = NULL;									/* nr_threads */	


/* �ں˺������� */
lw__current_kernel_time_fun lw__current_kernel_time;		/* __current_kernel_time */




/* �����������ҽṹ */
struct lws_sym lws_sym_val[] = {
	/* ���� */
	{(void**)&lw_nr_threads, "nr_threads"},
	/* ���� */
	{(void**)&lw__current_kernel_time, "__current_kernel_time"}
};


/* ******************************************
 * �� �� ��: lastwords_init_sym
 * ��������: �������Գ�ʼ������������ַ
 * �������: void
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_init_sym(void)
{
	unsigned long addr = 0;
	int i = 0;
	
	for (i = 0; i < sizeof(lws_sym_val)/sizeof(struct lws_sym); i++) {
		addr = kallsyms_lookup_name(lws_sym_val[i].name);
		if (0 == addr) {
			pr_err("Lastwords cannot find sym %s\n", lws_sym_val[i].name);
			return -1;
		}
		
		*(lws_sym_val[i].addr) = (void *)addr;
	}
		
	return 0;
}

/* ******************************************
 * �� �� ��: lastwords_exit_sym
 * ��������: �������Գ�ȥʼ������������ַ
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_exit_sym(void)
{
	/* to do */
}


