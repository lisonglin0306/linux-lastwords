#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>

#include "lastwords.h"
#include "lastwords_record.h"

struct proc_dir_entry *lw_proc = NULL;


/* ******************************************
 * �� �� ��: lastwords_show_proc
 * ��������: ��������show�ӿ�
 * �������: struct seq_file *seq: seq�ļ�������
 			 void *offset: �û�����ƫ��
 * �������: void	
 * �� �� ֵ: ���ض�ȡ�ֽ���
 * ****************************************** */
static int lastwords_show_proc(struct seq_file *seq, void *offset)
{
	int ret = 0;
	/* to do */
	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_write_proc
 * ��������: ��������д��proc�ӿ�
 * �������: struct file *file: proc�ļ�������
 			 const char __user *buffer: �û���������ݻ���
 			 size_t len: �û���������ݳ���
 			 loff_t *offset: �û�����ƫ��
 * �������: void	
 * �� �� ֵ: ����д���ֽ���
 * ****************************************** */
static ssize_t lastwords_write_proc(struct file *file, const char __user *buffer,
									size_t len, loff_t *offset)
{
	ssize_t ret, res;
	char *kbuf = NULL;

	kbuf = vzalloc(len);
	if (NULL == kbuf) 
		return -ENOMEM;

	res = copy_from_user(kbuf, buffer, len);
	if (res == len)
		return -EFAULT;

	/* ����������ж�Ӧ�Ĵ��� */
	if ('r' == kbuf[0]) 
		lastwords_show_record(__LAST_WORDS_ATTR_MAX);
	else if ('w' == kbuf[0] && ' ' == kbuf[1])
		ret = lastwords_write_user(kbuf+2, len-2);
 	else 
		pr_info("User input: %s\n", kbuf);
		
	ret = len;
	vfree(kbuf);
	
	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_open_proc
 * ��������: �������Դ�proc�ӿ�
 * �������: struct inode *inode: proc�ļ������ڵ�
 			 struct file *file: proc�ļ�������
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static int lastwords_open_proc(struct inode *inode, struct file *file)
{
	int ret = 0;

	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	ret = single_open(file, lastwords_show_proc, NULL);
	if (ret)
		module_put(THIS_MODULE);
	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_release_proc
 * ��������: �������Թر�proc�ӿ�
 * �������: struct inode *inode: proc�ļ������ڵ�
 			 struct file *file: proc�ļ�������
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static int lastwords_release_proc(struct inode *inode, struct file *file)
{
	int res = single_release(inode, file);
	module_put(THIS_MODULE);
	return res;
}

static const struct file_operations lastwords_proc_fops = {
	.open		= lastwords_open_proc,
	.read		= seq_read,
	.write		= lastwords_write_proc,
	.llseek		= seq_lseek,
	.release	= lastwords_release_proc,
};


/* ******************************************
 * �� �� ��: lastwords_proc_init
 * ��������: ���������û�Proc�ӿڳ�ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_init_proc(void)
{
	/* ���� lastwords proc Ŀ¼ */
	lw_proc = proc_create_data("lastwords", 0, NULL, &lastwords_proc_fops, NULL);
}

/* ******************************************
 * �� �� ��: lastwords_proc_init
 * ��������: ���������û�Proc�ӿڳ�ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_exit_proc(void)
{
	proc_remove(lw_proc);
}

/* ******************************************
 * �� �� ��: lastwords_init_interface
 * ��������: ���������û��ӿڳ�ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_init_interface(void)
{
	/* ��ʼ��proc�ӿ� */
	lastwords_init_proc();

	return 0;
}

/* ******************************************
 * �� �� ��: lastwords_exit_interface
 * ��������: ���������û��ӿ�ȥ��ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_exit_interface(void)
{
	/* ɾ��proc�ӿ� */
	lastwords_exit_proc();
}