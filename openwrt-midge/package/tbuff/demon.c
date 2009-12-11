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

#include "md5.h"
#include "kdb.h"

#define SOCKET_NAME "/tmp/socket"
#define NUM_SOCK 10
#define MAX_DATA 4*1024*1024
#define LOG_FILE "/var/log/demon.log"

char log_str[100];
char iface_name[10];
int log;
int buf_size = 128;
int sock; // socket for accept connections
int sockets[NUM_SOCK];
int numsock;

struct port
{
	int fd;
	int tbuf;
	int hbuf;
	char *buf;
	struct termios pots;
} ports[16];


int pb(int n)
{
	int i;
	sprintf(log_str, "\n-----------------------------------\n------------------buffer:\n");
	write(log, log_str, strlen(log_str));
	for (i = ports[n].tbuf; i != ports[n].hbuf; i++)
	{
		write(log, &ports[n].buf[i], 1);
		if (i == buf_size) i = 0;
	}
	sprintf(log_str, "\n-----------------------------------\n---------t = %i h = %i-----------\n", ports[n].tbuf, ports[n].hbuf);
	write(log, log_str, strlen(log_str));
	return 0;
}


int sprit_escape()
{
}

int init_port(char * port_name)
{
	int i = 0, n;
	speed_t speed = B115200;
	struct termios pts, sts;
	char file_name[50] = "/dev/";
	char *opt;
	int ret;
	
	while (port_name[i] != 0)
	{
		file_name[i + 5] = port_name[i];
		i++;
	}
	file_name[i + 5] = 0;
	if ((port_name[i - 2] <= '9') && (port_name[i - 2] >= '0'))
	{
		n = (port_name[i - 2] - '0') * 10 + port_name[i - 1] - '0';
	} else {
		n = port_name[i - 1] - '0';
	}
	// in n now we have number of port
	
	ports[n].fd =  open(file_name, O_RDWR);
	tcgetattr(ports[n].fd, &pts);
	memcpy(&ports[n].pots, &pts, sizeof(ports[n].pots));
	
	/* some things we want to set arbitrarily */
	pts.c_lflag &= ~ICANON; 
	pts.c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
	pts.c_cflag |= HUPCL;
	pts.c_cc[VMIN] = 1;
	pts.c_cc[VTIME] = 0;
	
	/* Standard CR/LF handling: this is a dumb terminal.
	* Do no translation:
	*  no NL -> CR/NL mapping on output, and
	*  no CR -> NL mapping on input.
	*/
	pts.c_oflag |= ONLCR;
	pts.c_iflag &= ~ICRNL;
	/* set hardware flow control by default */
	pts.c_cflag |= CRTSCTS;
	pts.c_iflag &= ~(IXON | IXOFF | IXANY);
	/* set speed */
	sprintf(file_name, "sys_demon_%s_speed", port_name);
	ret = kdb_appget(file_name, &opt);
	switch (atoi(opt))
	{
		case 1200:
			speed = B1200;
		case 2400:
			speed = B2400;
		case 4800:
			speed = B4800;
		case 9600:
			speed = B9600;
		case 19200:
			speed = B19200;
		case 38400:
			speed = B38400;
		case 57600:
			speed = B57600;
		case 115200:
			speed = B115200;
		case 230400:
			speed = B230400;
		case 460800:
			speed = B460800;
	}
	cfsetospeed(&pts, speed);
	cfsetispeed(&pts, speed);
	tcsetattr(ports[n].fd, TCSANOW, &pts);

	sprintf(log_str, "Port %s opened speed = %s\n", port_name, opt);
	write(log, log_str, strlen(log_str));
	
	ports[n].buf = malloc(buf_size);
	ports[n].hbuf = 0;
	ports[n].tbuf = 0;
	return 0;
}

// load params from kdb
int load_params ()
{
	char *opt;
	int i, j;
	
	kdbinit();
	kdb_appget("sys_demon_buf_size", &opt);
	buf_size = atoi(opt);
	sprintf(log_str, "buf_size = %i\n", buf_size);
	write(log, log_str, strlen(log_str));
	kdb_appget("sys_demon_iface_name", &opt);
	
	i = 0;
	while (1)
	{
		if ((opt[i] <= '9') && (opt[i] >= '0'))	break;
		i++;
	}
	opt[i] = 0;
	strcpy(iface_name, opt);
	sprintf(log_str, "iface_name = %s\n", iface_name);
	write(log, log_str, strlen(log_str));
	
	for (i = 0; i < 16; i++)
	{
		char tstr[50];
		int ret;
		sprintf(tstr, "sys_demon_%s%i_enable", iface_name, i);
		ret = kdb_appget(tstr, &opt);

		if ((atoi(opt) == 1) && (ret == 1))
		{
			sprintf(tstr, "%s%i", iface_name, i);
			init_port(tstr);
		}
	}
	db_close();
	return 0;
}

int loop()
{
	fd_set  ready;
	int i, s;
	int q = 0;
	struct sockaddr saddr_out;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	
	socklen_t sl = sizeof(saddr_out);


	int k, n, p, w;
	char *tstr;

	while (1) {
		s = -1;
		s = accept(sock, &saddr_out, &sl);
		if (s > 0)
		{
			if (numsock < NUM_SOCK)
			{
				sockets[numsock] = s;
				numsock++;
			} else {
				int k;
				for (k = 0; k < NUM_SOCK; k++)
				{
					if (sockets[k] == 0)
					{
						sockets[k] = s;
						break;
					}
				}
				if (k == NUM_SOCK)
				{
//					printf("Error: we have maximum connections\n");
					exit(-1);
				}
			}
		}

		FD_ZERO(&ready);
		int maxfd = 0, ii;
		for (i = 0; i < 16; i++)
		{
			if (ports[i].fd > maxfd) maxfd = ports[i].fd;
			if (ports[i].fd > 0) FD_SET(ports[i].fd, &ready);
		}
		for (i = 0; i < NUM_SOCK; i++)
		{
			if (sockets[i] > maxfd) maxfd = sockets[i];
			if (sockets[i] > 0) FD_SET(sockets[i], &ready);
		}

		select(maxfd + 1, &ready, NULL, NULL, &tv);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		for (i = 0; i < NUM_SOCK; i++)
		{
			if ((sockets[i] > 0) && (FD_ISSET(sockets[i], &ready)))
			{
				char rq[MAX_DATA];
				int r = read(sockets[i], rq, MAX_DATA);
				if (rq[0] != '1')
				{
					sprintf(log_str, "From socket %i read %i byte [%s]\n", i, r, rq);
					write(log, log_str, strlen(log_str));
				}
				if (r <= 0)
				{
//					printf("Error: cannot read from socket\n");
					exit(-1);
				}
//				printf(">>Debug: from socket %i readed: [%s]\n", i, rq);
				switch (rq[0])
				{
					// requets to read
					case '1':
						if (ports[atoi(&rq[2])].fd != -1)
						{
							tstr = malloc(buf_size);
							p = atoi(&rq[2]);
//							pb(p);
							k = 0;
							while (rq[k] != ';') k++;
							k++;
							while (rq[k] != ';') k++;
							n = atoi(&rq[k + 1]);
							
//							printf(">>Debug: n = %i p = %i\n", n, p);
							
							k = 0; w = ports[p].tbuf;
							
/////////////////							
							if (ports[p].tbuf == ports[p].hbuf)
							{
								tstr[0] = -1;
								write(sockets[i], tstr, 1);
//								sprintf(log_str, ">>>buf is empty\n");
//								write(log, log_str, strlen(log_str));
								
							} else {

								sprintf(log_str, ">>>buf-----\n");
								write(log, log_str, strlen(log_str));

								for (ii = 0; ii < n; ii++)
								{
									if (ports[p].tbuf != ports[p].hbuf)
									{
										tstr[k] = ports[p].buf[ports[p].tbuf];
//										write(log, &tstr[k], 1);
										k++;
										ports[p].tbuf++;
										if (ports[p].tbuf == buf_size)
										{
											ports[p].tbuf = 0;
										}
									} else {
										break;
									}
								}
								sprintf(log_str, ">>>\nt = %i h = %i-----\n", ports[p].tbuf, ports[p].hbuf);
								write(log, log_str, strlen(log_str));

								k = write(sockets[i], tstr, k);
								sprintf(log_str, "From buf %i to prog readed %i byte\n", p, k);
								write(log, log_str, strlen(log_str));

							}
//							pb(p);
//							printf(">>Debug: write return %i\n", k);
							free(tstr);
							if (close(sockets[i]))
							{
//								printf("Error: cannot close socket\n");
								exit(-1);
							}
							sockets[i] = 0;
						} else {
							tstr = malloc(100);
							sprintf(tstr, "We do not listen requsted port, enable it in kdb\n");
							write(sockets[i], tstr, strlen(tstr)+1);
							free(tstr);
							if (close(sockets[i]))
							{
//								printf("Error: cannot close socket\n");
								exit(-1);
							}
							sockets[i] = 0;
						}
					break;
					// request to write
					case '2':
						if (ports[atoi(&rq[2])].fd != -1)
						{
							p = atoi(&rq[2]);
//							pb(p);
							k = 2;
							while (rq[k] != ';') k++;
							k++;
							w = k;
							while (rq[k] != ';') k++;
							rq[k] = 0;
							k++;
							n = atoi(&rq[k]) - 1;
							
/////////////////////////// write data from prog in port
							char ch = 13;
							r = write(ports[p].fd, &rq[w], n + 1);
//							printf("---[%s]\n", &rq[w]);
							write(ports[p].fd, &ch, 1);
//							printf(">>Debug: In port %i write [%s] %i byte\n", p, &rq[w], n + 1);
							sprintf(log_str, "In port %i from prog write [%s] %i byte\n", p, &rq[w], n + 1);
							write(log, log_str, strlen(log_str));
							
							if (r != n + 1)
							{
//								printf("Error: cannot write in port\n");
								exit(-1);
							}
							sprintf(rq, "OK");
							r = write(sockets[i], rq, 3);
							if (r != 3)
							{
//								printf("Error: cannot write in socket\n");
								exit(-1);
							}
						} else {
							tstr = malloc(100);
							sprintf(tstr, "We do not listen requsted port, enable it in kdb\n");
							write(sockets[i], tstr, strlen(tstr)+1);
							free(tstr);
						}
						if (close(sockets[i]))
						{
//							printf("Error: cannot close socket\n");
							exit(-1);
						}
						sockets[i] = 0;
//						pb(p);
					break;
					case '3':
						char *tbuff;
						int cnt = 0;
						tbuff = malloc(buf_size*16);
						tbuff[cnt] = '{';
						cnt++;
						tbuff[cnt] = '[';
						cnt++;
						for (j = 0; j < 16; j++)
						{
							if ((ports[j].fd > 0) && (ports[j].tbuf != ports[j].hbuf))
							{
								sprintf(&tbuff[cnt], "{\"dev\":\"");
								cnt+=8;
								sprintf(&tbuff[cnt], "%s", iface_name);
								cnt+=strlen(iface_name);
								sprintf(&tbuff[cnt], "{\",\"text\":\"");
								cnt+=11;
								while (1)
								{
									if (ports[j].tbuf != ports[j].hbuf)
									{
										tbuff[cnt] = ports[j].buf[ports[j].tbuf];
										cnt++;
										ports[j].tbuf++;
										if (ports[j].tbuf == buf_size)
										{
											ports[j].tbuf = 0;
										}
									} else {
										break;
									}
								}
								sprintf(&tbuff[cnt], "},");
								cnt+=2;
							}
						}
						cnt--;
						sprintf(&tbuff[cnt], "]}");
						cnt+=2;
						
					break;
				}
			}
		}


		//read data from device on ports
		for (i = 0; i < 16; i++)
		{
			if ((ports[i].fd > 0) && (FD_ISSET(ports[i].fd, &ready)))
			{
				char tbuf[buf_size];
				int qw, qweqwe;
//				pb(i);
				int rb = read(ports[i].fd, tbuf, buf_size);
				sprintf(log_str, "From port %i in buf readed %i byte\n", i, rb);
				write(log, log_str, strlen(log_str));
				q += rb;
				if (rb > 0)
				{
					for (qw = 0; qw < rb; qw++)
					{
							if (tbuf[qw] == 27)
							{
								qweqwe = qw;
								qw++;
								if (tbuf[qw] == '[') {
									qw++;
//									while (tbuf[qw] != ';') qw++;
									while (tbuf[qw] != 'm') qw++;
									qw++;
									if (qw >= rb) break;
								}
							}
							sprintf(log_str, "strip escape %i\n", qw-qweqwe);
							write(log, log_str, strlen(log_str));
							
							ports[i].buf[ports[i].hbuf] = tbuf[qw];
							ports[i].hbuf++;
							if (ports[i].hbuf == buf_size)
							{
								ports[i].hbuf = 0;
							}
							if (ports[i].tbuf == ports[i].hbuf)
							{
								ports[i].tbuf++;
								if (ports[i].tbuf == buf_size) ports[i].tbuf = 0;
							}
						
/*
							ports[i].buf[ports[i].hbuf] = tbuf[qw];
							ports[i].hbuf++;
							if (ports[i].hbuf == buf_size)
							{
								ports[i].hbuf = 0;
								ports[i].full = 1;
							}
*/

					}
//					if (ports[i].full) ports[i].tbuf = ports[i].hbuf;
					
//					printf(">>Debug: From port ttyRS%i to buf readed %i byte hbuf = %i tbuf = %i\n", i, rb, ports[i].hbuf, ports[i].tbuf);
					// Print buf at screen			
//					printf("-------buf-------\n");
					sprintf(log_str, ">>>------buf------\n");
					write(log, log_str, strlen(log_str));
					for (qw = ports[i].tbuf; qw != ports[i].hbuf; qw++)
					{
						if (qw == buf_size) qw = 0;
//						printf("%c", ports[i].buf[qw]);
//						write(log, &ports[i].buf[qw], 1);
						if (qw == ports[i].hbuf) break;
					}
					sprintf(log_str, "\n>>> t = %i h = %i-----\n", ports[i].tbuf, ports[i].hbuf);
					write(log, log_str, strlen(log_str));

//					printf("\n-----------\n");
					for (qw = ports[i].tbuf; qw != ports[i].hbuf; qw++)
					{
						if (qw == buf_size) qw = 0;
//						printf("%i ", ports[i].buf[qw]);
						if (qw == ports[i].hbuf) break;
					}
//					printf("\n-----------\n");
				}
				if (rb < 0)
				{
//					printf(">>Debug: read return value < 0!!!!!!! errno = %s hbuf = %i\n", strerror(errno), ports[0].hbuf);
				}
//				pb(i);
			}
		}
	}
}

int init_socket()
{
	struct sockaddr saddr_in;

	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock == -1)
	{
		sprintf(log_str, "Error: cannot create socket\n");
		write(log, log_str, strlen(log_str));
		exit(-1);
	}
	if (fcntl(sock, F_SETFL, O_NONBLOCK))
	{
		sprintf(log_str, "Error: cannot set socket options\n");
		write(log, log_str, strlen(log_str));
		exit(-1);
	}
	saddr_in.sa_family = AF_LOCAL;
	strcpy(saddr_in.sa_data, SOCKET_NAME);
	if (bind(sock, &saddr_in, sizeof(saddr_in)))
	{
		sprintf(log_str, "Error: cannot bind socket\n");
		write(log, log_str, strlen(log_str));
		exit(-1);
	}
	if (listen(sock, 10))
	{
		sprintf(log_str, "Error: cannot listen socket\n");
		write(log, log_str, strlen(log_str));
		exit(-1);
	}
}

int close_port(int p)
{
	if (ports[p].fd != 0)
	{
		if (close(ports[p].fd) < 0) return -1;
		ports[p].hbuf = 0;
		free(ports[p].buf);
	}
	return 0;
}

int close_all_ports()
{
	int i;
	for (i = 0; i< 16; i++)
	{
		close_port(i);
	}
	return 0;
}

int close_all_sock ()
{
	int i;
	close(sock);
	unlink(SOCKET_NAME);
	for (i = 0; i < NUM_SOCK; i++)
	{
		close(sockets[i]);
	}
}



void handler (int i)
{
//	printf("Recieved signal TERM\n");
	close_all_ports ();
	close_all_sock ();
	close(log);
	exit(0);
}

int main (int argc, char **argv)
{
	int pid, i;
	struct sigaction sa;

	pid = fork();
	if (pid == 0)
	{
		log = open(LOG_FILE, O_WRONLY | O_APPEND | O_CREAT);
		printf("%s\n", strerror(errno));
		if (log == -1) return 0;
		setsid();
		chdir("/");
		close(0);
		close(1);
		close(2);
		sa.sa_handler = handler;
		sigaction(SIGTERM, &sa, 0);
		sprintf(log_str, "------------demon started------------------");
		write(log, log_str, strlen(log_str));

		for (i = 0; i < 16; i++)
		{
			ports[i].fd = -1;
		}
		load_params();
		init_socket();
		loop();
	}
	return 0;
}
