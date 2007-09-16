#ifndef MXCONFIG_H
#define MXCONFIG_H

#include "dev_interface.h"
#include "ltypes.h"

#define BUSLINES_QUAN 15
#define MAX_IFS 30

typedef struct{
    ifdescr_t *ifd;
    u8 xfs;
    u8 mxrate;
} busline_elem_t;

typedef struct{
    busline_elem_t transm[MAX_IFS],recv[MAX_IFS];
    int tcnt,rcnt;
} busline_t;

#define mxerror(fmt,args...) printf(fmt "\n",  ##args)

#endif