#ifndef  RS232_TCPEXT_TTY_H
# define RS232_TCPEXT_TTY_H

# include "iobase.h"
# include "misc.h" // uchar


typedef unsigned char modem_state_t;

typedef struct
{
    iobase_t *b;
    struct termios termios;
    modem_state_t last_in_mstate;
    char          last_in_mstate_valid;
} tty_t;


tty_t *tty_create   (void);
void   tty_delete   (tty_t *t);

void   tty_open     (tty_t *t, const char *devname);
void   tty_close    (tty_t *t);
void   tty_close_no_restore_attr(tty_t *t);

int    tty_set_raw  (tty_t *t);
int    tty_set_raw_no_save_attr(tty_t *t);
void   tty_restore  (tty_t *t);


extern inline const char *tty_name(tty_t *t) { return iobase_name(t->b); }
extern inline int         tty_fd  (tty_t *t) { return iobase_fd  (t->b); }


extern inline ssize_t tty_read     (tty_t *t,       char *buf, size_t len)
	{ return iobase_read (t->b, buf, len); }

extern inline ssize_t tty_write    (tty_t *t, const char *buf, size_t len)
	{ return iobase_write(t->b, buf, len); }

extern inline int     tty_write_all(tty_t *t, const char *buf, size_t len)
	{ return iobase_write_all(t->b, buf, len); }


// See tty_ioctl(4), Modem control
enum {
    TTY_MODEM_DTR = 0x01, // > data terminal ready; Tells DCE that DTE is ready to be connected.
    TTY_MODEM_DSR = 0x02, // < data set ready; Tells DTE that DCE is ready to receive commands or data.
    TTY_MODEM_RTS = 0x04, // > request to send; Tells DCE to prepare to accept data from DTE.
    TTY_MODEM_CTS = 0x08, // < clear to send; Acknowledges RTS and allows DTE to transmit.
    TTY_MODEM_CD  = 0x10, // < Carrier Detect; Tells DTE that DCE is connected to telephone line.
    TTY_MODEM_RI  = 0x20, // < Ring Indicator; Tells DTE that DCE has detected a ring signal on the telephone line.
};

int  tty_get_modem_state(tty_t *t, modem_state_t *mstate);
int  tty_set_modem_state(tty_t *t, modem_state_t  mstate);
void tty_log_modem_state(const char *pfx, modem_state_t mstate);

modem_state_t tty_mstate_merge(modem_state_t in_state);



#endif //RS232_TCPEXT_TTY_H
