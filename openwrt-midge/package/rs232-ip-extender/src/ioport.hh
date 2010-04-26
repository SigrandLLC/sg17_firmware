#include "sys_headers.h"
#include "ioport.h"
#include "misc.h"


static ioport_t* ioport_create(void);
static void      ioport_delete(ioport_t* p);

static ioport_vmt_t ioport_vmt {
    .create   = ioport_create,
    .delete   = ioport_delete,
    .open     = NULL,
    .close    = NULL,
    .send     = NULL,
    .send_all = NULL,
    .recv     = NULL,
};

static void ioport_on_exit(int unused, void *arg)
{
    (void)unused;
    ioport_t *p = arg;
    p->vmt->delete(p);
}

static ioport_t* ioport_create(void)
{
    ioport_t *p = xzmalloc(sizeof(ioport_t));

    p->vmt = ioport_vmt;

    p->fd = -1;

    onexit(ioport_on_exit, p);

    return s;
}

static void ioport_delete(ioport_t* p)
{
    p->vmt->close(p);
    free(p);
}

