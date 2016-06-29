#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/notifier.h>
#include <linux/netdevice.h>
#include <linux/reboot.h>
#include <linux/net.h>
#include <linux/kdebug.h>
#include <linux/oom.h>
#include <net/netevent.h>

#include "lastwords.h"
#include "lastwords_record.h"
#include "lastwords_export.h"


/* oom������� */										
									

/* die������� */


/* reboot������� */
#define LASTWORDS_REBOOT_EXPORT			( LASTWORDS_EXPORT_TIME \
										| LASTWORDS_EXPORT_SYSINFO \
										| LASTWORDS_EXPORT_DMESG \
										| LASTWORDS_EXPORT_PS )





/* ******************************************
 * �� �� ��: lastwords_oom_callback
 * ��������: ��������oom�ں�֪ͨ���ص�
 * �������: struct notifier_block *this: ֪ͨ���ṹ
 			 unsigned long event: ��������
 			 void *ptr: ��������
 * �������: void	
 * �� �� ֵ: ����NOTIFY_DONE
 * ****************************************** */
static int lastwords_oom_callback(struct notifier_block *this, 
				unsigned long event, void *ptr)
{
	return NOTIFY_DONE;
}

static struct notifier_block lastwords_oom_notifier = {
	.notifier_call = lastwords_oom_callback,
};


/* ******************************************
 * �� �� ��: lastwords_die_callback
 * ��������: ��������die�ں�֪ͨ���ص�
 * �������: struct notifier_block *this: ֪ͨ���ṹ
 			 unsigned long event: ��������
 			 void *ptr: ��������
 * �������: void	
 * �� �� ֵ: ����NOTIFY_DONE
 * ****************************************** */
static int lastwords_die_callback(struct notifier_block *this, 
				unsigned long event, void *ptr)
{
	return NOTIFY_DONE;
}

static struct notifier_block lastwords_die_notifier = {
	.notifier_call = lastwords_die_callback,
};


/* ******************************************
 * �� �� ��: lastwords_netevent_callback
 * ��������: ������������״̬�仯�ں�֪ͨ���ص�
 * �������: struct notifier_block *this: ֪ͨ���ṹ
 			 unsigned long event: ��������
 			 void *ptr: ��������
 * �������: void	
 * �� �� ֵ: ����NOTIFY_DONE
 * ****************************************** */
static int lastwords_netevent_callback(struct notifier_block *this, 
				unsigned long event, void *ptr)
{
	return NOTIFY_DONE;
}

static struct notifier_block lastwords_netevent_notifier = {
	.notifier_call = lastwords_netevent_callback
};


/* ******************************************
 * �� �� ��: lastwords_netdev_event
 * ��������: �������������豸�ں�֪ͨ���ص�
 * �������: struct notifier_block *this: ֪ͨ���ṹ
 			 unsigned long event: ��������
 			 void *ptr: ��������
 * �������: void	
 * �� �� ֵ: ����NOTIFY_DONE
 * ****************************************** */
static int lastwords_netdev_callback(struct notifier_block *this, 
				unsigned long event, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	struct net *net = dev_net(dev);

	switch (event) {
	/* �ı������豸��ַ */
	case NETDEV_CHANGEADDR:
		pr_info("net device change addr\n");
		
		break;
	/* ͣ�������豸 */
	case NETDEV_DOWN:
		pr_info("net device down\n");
		
		break;
	/* ���������豸 */
	case NETDEV_UP:
		pr_info("net device up\n");

		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block lastwords_netdev_notifier = {
	.notifier_call = lastwords_netdev_callback,
};


/* ******************************************
 * �� �� ��: lastwords_reboot_callback
 * ��������: ��������reboot�ں�֪ͨ���ص�
 * �������: struct notifier_block *this: ֪ͨ���ṹ
 			 unsigned long event: ��������
 			 void *ptr: ��������
 * �������: void	
 * �� �� ֵ: ����NOTIFY_DONE
 * ****************************************** */
static int lastwords_reboot_callback(struct notifier_block *this, 
				unsigned long event, void *ptr)
{
	(void)lastwords_export_attr(LAST_WORDS_ATTR_REBOOT, LASTWORDS_REBOOT_EXPORT, 
							"System reboot");

	return NOTIFY_DONE;
}

static struct notifier_block lastwords_reboot_notifier = {
	.notifier_call = lastwords_reboot_callback,
};


/* ******************************************
 * �� �� ��: lastwords_panic_callback
 * ��������: ������panic�ں�֪ͨ���ص�
 * �������: struct notifier_block *this: ֪ͨ���ṹ
 			 unsigned long event: ��������
 			 void *ptr: ��������
 * �������: void	
 * �� �� ֵ: ����NOTIFY_DONE
 * ****************************************** */
static int lastwords_panic_callback(struct notifier_block *this, 
				unsigned long event, void *ptr)
{
	return NOTIFY_DONE;
}

static struct notifier_block lastwords_panic_notifier = { 
	.notifier_call = lastwords_panic_callback, 
};




/* ******************************************
 * �� �� ��: lastwords_init_monitor
 * ��������: �������Լ�������ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_init_monitor(void)
{
	int ret = 0;
	
	/* ע�������豸֪ͨ�� */
	ret = register_netdevice_notifier(&lastwords_netdev_notifier);
	if (ret) {
		pr_err("Faile to register lastwords netdev notifier!\n");
		goto err_netdev;
	}

	/* ע������֪ͨ�� */
	ret = register_netevent_notifier(&lastwords_netevent_notifier);
	if (ret) {
		pr_err("Faile to register lastwords netevent notifier!\n");
		goto err_netevent;
	}
	
	/* ע��reboot֪ͨ�� */
	ret = register_reboot_notifier(&lastwords_reboot_notifier);
	if (ret) {
		pr_err("Faile to register reboot notifier!\n");
		goto err_reboot;
	}

	/* ע��die֪ͨ�� */
	ret = register_die_notifier(&lastwords_die_notifier);
	if (ret) {
		pr_err("Faile to register die notifier!\n");
		goto err_die;
	}

	/* ע��oom֪ͨ�� */
	ret = register_oom_notifier(&lastwords_oom_notifier);
	if (ret) {
		pr_err("Faile to register oom notifier!\n");
		goto err_oom;
	}	

	/* ע��panic֪ͨ�� */
	atomic_notifier_chain_register(&panic_notifier_list, &lastwords_panic_notifier);

	/* to do */
	
	return 0;

err_oom:
	unregister_die_notifier(&lastwords_die_notifier);
err_die:
	unregister_reboot_notifier(&lastwords_reboot_notifier);	
err_reboot:
	unregister_netevent_notifier(&lastwords_netevent_notifier);
err_netevent:
	unregister_netdevice_notifier(&lastwords_netdev_notifier);	
err_netdev:
	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_exit_monitor
 * ��������: �������Լ�����ȥ��ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_exit_monitor(void)
{
	/* ע��֪ͨ�� */
	atomic_notifier_chain_unregister(&panic_notifier_list, &lastwords_panic_notifier);	
	unregister_oom_notifier(&lastwords_oom_notifier);
	unregister_die_notifier(&lastwords_die_notifier);
	unregister_reboot_notifier(&lastwords_reboot_notifier);	
	unregister_netevent_notifier(&lastwords_netevent_notifier);	
	unregister_netdevice_notifier(&lastwords_netdev_notifier);
}


