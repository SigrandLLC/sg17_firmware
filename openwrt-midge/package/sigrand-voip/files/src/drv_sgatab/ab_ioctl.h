#ifndef __AB_IOCTL_H__
#define __AB_IOCTL_H__

#include <linux/ioctl.h>

#define BOARD_SLOT_FREE  (-1)
#define BOARDS_MAX         4	
#define DEVS_PER_BOARD_MAX 4
#define CHANS_PER_DEV      2

#define DEV_TYPE_MASK    0x3
#define DEV_TYPE_LENGTH    2

#define SGATAB_IOC_MAGIC 's'

#define SGAB_GET_BOARDS_PRESENCE _IO(SGATAB_IOC_MAGIC, 1)
#define SGAB_GET_INIT_PARAMS	 _IO(SGATAB_IOC_MAGIC, 2)

typedef struct ab_boards_presence_s
{
	long slots [BOARDS_MAX]; /*< -1 means absence */
} ab_boards_presence_t;

typedef enum dev_type_e
{
	dev_type_ABSENT = 0x0,
	dev_type_FXO = 0x1,
	dev_type_RESERVED = 0x2,
	dev_type_FXS = 0x3
} dev_type_t;

typedef struct ab_init_params_s
{
	long requested_board_slot;
	unsigned long nBaseAddress;
	long nIrqNum;
	unsigned char AccessMode;
	unsigned long region_size;
	unsigned char first_chan_idx;
	dev_type_t devices [DEVS_PER_BOARD_MAX];
} ab_init_params_t;


#endif /* __AB_IOCTL_H__ */

