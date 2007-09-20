#ifndef MXCONFIG_H
#define MXCONFIG_H

#include "dev_interface.h"
#include "ltypes.h"

#define MAX_IFS 100
#define MX_LINES 15
#define MX_LWIDTH 256

// Errors
#define ELSPACE 1
#define EDOMAIN 2


typedef enum { clkA, clkB } domain_t;

typedef struct{
    ifdescr_t *ifd;
    u8 xfs;
    u8 mxrate;
} mxelem_t;

typedef struct{
    domain_t domain;
    u8 domain_err;
    mxelem_t devs[MAX_IFS];
    int devcnt;
} mxline_t;

mxline_t *mxline_init();
int mxline_add(mxline_t *l,domain_t domain,mxelem_t el);
void mxline_free(mxline_t *l);

#define mxerror(fmt,args...) printf("ERROR: " fmt "\n",  ##args)
#define mxwarn(fmt,args...) printf("WARNING " fmt "\n",  ##args)

#endif

