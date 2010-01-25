#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/select.h>

#include "md5.h"
#include "kdb.h"

#define SOCKET_NAME "/tmp/socket"
#define NUM_SOCK 10
#define MAX_DATA 4*1024*1024

struct port
{
	int fd;
	int tbuf;
	int hbuf;
	char *buf;
	struct termios pots;
	int esc_s;
	char cmd[100];
};
