#ifndef MXCONFIG_H
#define MXCONFIG_H

#include "dev_interface.h"
#include "ltypes.h"

#define MAX_IFS 100
#define MX_LINES 15
#define MX_LWIDTH 256

typedef struct{
    ifdescr_t *ifd;
    u8 xfs;
    u8 mxrate;
} mxelem_t;

typedef struct{
    mxelem_t *read[MAX_IFS],*write[MAX_IFS];
    int rcnt,wcnt;
} mxline_t;

typedef struct{
    mxline_t *a[MX_LINES];
    int lnum;
} mxdomain_t;

typedef struct{
    u8 ints[MXLWIDTH/2][2];
    int inum;
}mxints_t;


#define mxerror(fmt,args...) printf("ERROR: " fmt "\n",  ##args)
#define mxwarn(fmt,args...) printf("WARNING " fmt "\n",  ##args)

#endif
