#ifndef DEV_INTERFACE_H
#define DEV_INTERFACE_H

#include "ltypes.h"

#define MAX_FNAME 512
#define BUF_SIZE 256

typedef enum { unknown_ts,continual_ts,selective_ts } iftype_t;

typedef struct {
    iftype_t type;
    u32 mx_slotmap;
    s32 mxrate;
    s8 rline,tline;
    s16 tfs,rfs;
    s8 clkm;
    s8 clkab;
    s8 clkr;
    s8 mxen;
} devsetup_t;


typedef struct {
    char *name;
    devsetup_t settings;    
} ifdescr_t;


int check_for_mxsupport(char *conf_path,ifdescr_t *ifd);
int fill_iflist(char *conf_path,ifdescr_t **iflist,int ifcnt);
int set_dev_option(char *conf_path,char *name,int val);
int get_dev_option(char *conf_path,char *name,int *val);
u32 str2slotmap(char *str,size_t size,int *err);
int slotmap2str(u32 smap, char *buf);
u8 slotmap2mxrate(u32 smap);
int apply_settings(char *conf_path,ifdescr_t *ifd);
int accum_settings(char *conf_path,ifdescr_t *ifd);


#endif
