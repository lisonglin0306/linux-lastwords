#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>

#include "lastwords.h"
#include "lastwords_mem.h"
#include "lastwords_record.h"
#include "lastwords_export.h"
#include "lastwords_interface.h"


/* �û�������� */
#define LASTWORDS_USER_EXPORT			( LASTWORDS_EXPORT_TIME \
										| LASTWORDS_EXPORT_SYSINFO \
										| LASTWORDS_EXPORT_DMESG \
										| LASTWORDS_EXPORT_PS )

static struct proc_dir_entry *lw_proc = NULL;


/* ******************************************
 * �� �� ��: lastwords_write_user
 * ��������: ���������û������ӿ�
 * �������: char *buf: �û�������Ϣ
 			 size_t len: �û���������ݳ���
 * �������: void	
 * �� �� ֵ: int: �ɹ�����0
 * ****************************************** */
static int lastwords_write_user(char *data)
{
	return lastwords_export_attr(LAST_WORDS_ATTR_USER, LASTWORDS_USER_EXPORT, data);
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

	/* �����û���¼ */
	ret = lastwords_write_user(kbuf);	
	if (ret)
		return -EFAULT;
	
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
	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	return 0;
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
	module_put(THIS_MODULE);
	return 0;
}

static const struct file_operations lastwords_proc_fops = {
	.open		= lastwords_open_proc,
	.write		= lastwords_write_proc,
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
 * �� �� ��: lastwords_write_dev
 * ��������: ���������û��ַ��豸����
 * �������: struct file *file: �ļ�����
 			 unsigned int cmd: ������
 			 unsigned long arg: �������
 * �������: void	
 * �� �� ֵ: long: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
static long lastwords_ioctl_dev(struct file *file, unsigned int cmd, 
					unsigned long arg)
{
	int ret = 0;
	
	switch (cmd) {
	/* ��ȡ��¼��Ϣ���ܳ��� */
	case LASTWORDS_GET_MEMSIZE: {
		__u32 len = 0;
		
		len = lastwords_get_recordlen();
		ret = put_user(len, (unsigned int __user *)arg);
		break;
	}
	/* �����������Թ��� */
	case LASTWORDS_CTRL_ONOFF: {
		/* to do */		
		break;
	}
	/* ��ʽ���ڴ�ռ� */
	case LASTWORDS_FORMAT_MEM: {
		lastwords_export_clear();
		break;
	}	
	/* �����û���¼ */
	case LASTWORDS_TRIGGER_RECORD: {
		/* to do */		
		break;
	}	
	default:
		pr_err("Lastwords unknown ioctl cmd = 0x%x\n", cmd);
	}
	
	return ret;
}

#if 1
/* ******************************************
 * �� �� ��: lastwords_mmap_dev
 * ��������: ���������û��ַ��豸ӳ���ڴ�
 * �������: struct file *file: �ļ�����
 			 struct vm_area_struct *vm: �ڴ�ṹ
 * �������: void	
 * �� �� ֵ: int: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_mmap_dev(struct file *file, struct vm_area_struct *vma)
{
	phys_addr_t phyaddr = 0;
	int ret = 0;

	phyaddr = lastwords_get_memphy();
	
	ret = remap_pfn_range(vma, vma->vm_start, phyaddr >> PAGE_SHIFT,
						vma->vm_end - vma->vm_start, vma->vm_page_prot);
	if (ret)
		pr_err("Lastwords remap pfn range fail (err=%d)\n", ret);
	
	return ret;
}
#endif

/* ******************************************
 * �� �� ��: lastwords_write_dev
 * ��������: ���������û��ַ��豸��ȡ�ӿ�
 * �������: struct file *file: �ļ�����
 			 char __user *buf: �û��ڴ�ռ仺��
 			 size_t len: ��ȡ��С
 			 loff_t *off: ƫ��
 * �������: void	
 * �� �� ֵ: long: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
ssize_t lastwords_read_dev(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	char *kbuf = NULL;
	__u32 read_len = 0;

	if (0 == len)
		return 0;
	
	kbuf = vmalloc(len);
	if (NULL == kbuf) {
		pr_err("Lastwords vmalloc error\n");
		return -ENOMEM;
	}

	/* ��ȡ�������Լ�¼���� */
	read_len = lastwords_export_dump(kbuf, (__u32)len, *off);
	if (0 > read_len) {
		vfree(kbuf);
		return read_len;
	}

	/* �������û��ռ� */
	if (copy_to_user(buf, kbuf, read_len)) 
		pr_err("Lastwords copy to user error\n");

	vfree(kbuf);
	*off += read_len;
	
	return read_len;
}

static const struct file_operations lastwords_dev_fops = {
	.owner		= THIS_MODULE,
	.llseek		= default_llseek,
	.read		= lastwords_read_dev,
	.mmap		= lastwords_mmap_dev,
	.unlocked_ioctl	= lastwords_ioctl_dev,
};

static struct miscdevice lastwords_miscdev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= "lastwords",
	.fops		= &lastwords_dev_fops,
};

/* ******************************************
 * �� �� ��: lastwords_init_dev
 * ��������: ���������û��ַ��豸��ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: int: �ɹ�����0��ʧ�ܷ��ش�����
 * ****************************************** */
int lastwords_init_dev(void)
{	
	int ret = 0;
	
	/* �����ַ��豸 */
	ret = misc_register(&lastwords_miscdev);
	if (ret) 
		pr_err("cannot register lastwords miscdev (err=%d)\n", ret);

	return ret;
}

/* ******************************************
 * �� �� ��: lastwords_proc_init
 * ��������: ���������û��ַ��豸ȥ��ʼ��
 * �������: void
 * �������: void	
 * �� �� ֵ: void
 * ****************************************** */
void lastwords_exit_dev(void)
{
	misc_deregister(&lastwords_miscdev);
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
	int ret = 0;
	
	/* ��ʼ��proc�ӿ� */
	lastwords_init_proc();
	
	/* ע���ַ��豸 */
	ret = lastwords_init_dev();
	if (ret) {
		lastwords_exit_proc();
		return ret;
	}
	
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
	/* ɾ���ַ��豸 */
	lastwords_exit_dev();
	
	/* ɾ��proc�ӿ� */
	lastwords_exit_proc();
}