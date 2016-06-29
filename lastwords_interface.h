
#ifndef	__LASTWORDS_INTERFACE__
#define __LASTWORDS_INTERFACE__


#define LASTWORDS_MAGIC		'L'

#define LASTWORDS_CTRL_ONOFF		_IOW(LASTWORDS_MAGIC, 0x00, int)
#define LASTWORDS_GET_MEMSIZE		_IOR(LASTWORDS_MAGIC, 0x01, unsigned int)
#define LASTWORDS_FORMAT_MEM		_IO(LASTWORDS_MAGIC, 0x02)
#define LASTWORDS_TRIGGER_RECORD	_IOW(LASTWORDS_MAGIC, 0x03, char *)

#endif
