#ifndef __AB_IOCTL_H__
#define __AB_IOCTL_H__

#include <linux/ioctl.h>

#define SGATAB_BOARD_NAME_LENGTH 50

#define SGATAB_IOC_MAGIC 's'

#define SGAB_GET_INIT_PRMS	_IO(SGATAB_IOC_MAGIC, 1)
#define SGAB_BASIC_INIT_TYPES	_IO(SGATAB_IOC_MAGIC, 2)

typedef enum dev_types_e
{
	dev_type_UNKNOWN,
	dev_type_FXO,
	dev_type_FXS
} dev_type_t;

typedef struct ab_init_params_s
{
	unsigned long nBaseAddress;
	long nIrqNum;
	unsigned char AccessMode;

	char name [ SGATAB_BOARD_NAME_LENGTH ];
	unsigned long region_size;
	unsigned char devs_count;
	unsigned char chans_per_dev;
	unsigned char first_chan_num;
} ab_init_params_t;

typedef struct ab_dev_types_s
{
	unsigned char devs_count;
	enum dev_types_e * dev_type;
} ab_dev_types_t;

#endif /* __AB_IOCTL_H__ */

