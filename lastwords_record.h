
typedef enum {
	LAST_WORDS_ATTR_UNSPEC,
	LAST_WORDS_ATTR_USER,			/* �û���Ϣ */	
	LAST_WORDS_ATTR_NETADR,			/* �����豸��ַ�ı� */
	LAST_WORDS_ATTR_MODULE,			/* �ں˼���ж��ģ�� */
	LAST_WORDS_ATTR_REBOOT,			/* �豸ָ��������(reboot) */
	LAST_WORDS_ATTR_PANIC,			/* �ں˿ֻ�(kernel panic) */
	__LAST_WORDS_ATTR_MAX,
} lw_attr_t;

#define LAST_WORDS_ATTR_MAX (__LAST_WORDS_ATTR_MAX - 1)

void lastwords_show_head(void);
void lastwords_show_record(lw_attr_t attr_type);
int lastwords_write_record(lw_attr_t attr_type, char *buf, int len);
int lastwords_print_record(lw_attr_t attr_type, const char *fmt, ...);

#define lastwords_print_user(fmt, ...) \
			lastwords_print_record(LAST_WORDS_ATTR_USER, fmt, ##__VA_ARGS__)
#define lastwords_print_netaddr(fmt, ...) \
			lastwords_print_record(LAST_WORDS_ATTR_NETADR, fmt, ##__VA_ARGS__)

#define lastwords_write_user(buf,len) \
			lastwords_write_record(LAST_WORDS_ATTR_USER, buf, len)
#define lastwords_write_netaddr(buf,len) \
			lastwords_write_record(LAST_WORDS_ATTR_NETADR, buf, len)

