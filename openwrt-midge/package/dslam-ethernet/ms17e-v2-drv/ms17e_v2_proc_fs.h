#ifndef MS17E_V2_PROC_FS_H
#define MS17E_V2_PROC_FS_H

#include <linux/kobject.h>

int ms17e_v2_proc_fs_register(struct ms17e_v2_card *card);
int ms17e_v2_proc_fs_remove(struct ms17e_v2_card *card);

#endif
