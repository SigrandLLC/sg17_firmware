#include "demon.h"

int lock_file;
char iface_name[10];
int buf_size = 4096;
int sock; // socket for accept connections
int sockets[NUM_SOCK];
int numsock;
struct timeval tv1, tv2;
int v = 0;
FILE *log_file;

int bs = 0;

struct port ports[MAX_PORTS];

int init_port(char * port_name)
{
	int i, n;
	speed_t speed = B115200;
	struct termios pts, sts;
	char file_name[50] = "/dev/";
	char *opt;
	int ret;
	
	i = 0;
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
	if (cfsetospeed(&pts, speed)) if (v) printf("Error to set output speed\n");
	if (cfsetispeed(&pts, speed)) if (v) printf("Error to set input speed\n");
	tcsetattr(ports[n].fd, TCSANOW, &pts);

	ports[n].buf = malloc(buf_size);
	ports[n].hbuf = 0;
	ports[n].tbuf = 0;
//	ports[n].esc_s = 0;
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
	kdb_appget("sys_demon_iface_name", &opt);
	
	i = 0;
	while (1)
	{
		if ((opt[i] <= '9') && (opt[i] >= '0'))	break;
		i++;
	}
	opt[i] = 0;
	strcpy(iface_name, opt);
	
	for (i = 0; i < MAX_PORTS; i++)
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

int read_from_sockets(fd_set ready)
{
	int i, p, k, n, w, cnt, ii, po, j;
	char * tstr, *tbuff;
// read data from sockets		
		for (i = 0; i < NUM_SOCK; i++)
		{
			if ((sockets[i] > 0) && (FD_ISSET(sockets[i], &ready)))
			{
				char rq[MAX_DATA];
				int r = read(sockets[i], rq, MAX_DATA);
				if (v) printf("from socket readed:(%i byte) %s\n", r, rq);
//				fprintf(log_file, "from socket readed:(%i byte) %s\n", r, rq);
//				fflush(log_file);
				switch (rq[0])
				{
					// requets to read
					case 'r':
						if (ports[atoi(&rq[2])].fd != -1)
						{
							tstr = malloc(buf_size);
							p = atoi(&rq[2]);
							k = 0;
							while (rq[k] != ';') k++;
							k++;
							while (rq[k] != ';') k++;
							n = atoi(&rq[k + 1]);
							k = 0; w = ports[p].tbuf;
/////////////////							
							if (ports[p].tbuf == ports[p].hbuf)
							{
								tstr[0] = -1;
								write(sockets[i], tstr, 1);
							} else {
								for (ii = 0; ii < n; ii++)
								{
									if (ports[p].tbuf != ports[p].hbuf)
									{
										tstr[k] = ports[p].buf[ports[p].tbuf];
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
								k = write(sockets[i], tstr, k);
							}
							free(tstr);
							close(sockets[i]);
							sockets[i] = 0;
						} else {
							tstr = malloc(100);
							sprintf(tstr, "We do not listen requsted port, enable it in kdb\n");
							write(sockets[i], tstr, strlen(tstr)+1);
							free(tstr);
							close(sockets[i]);
							sockets[i] = 0;
						}
					break;
					// request to write
					case 'w':
						if (ports[atoi(&rq[2])].fd != -1)
						{
							p = atoi(&rq[2]);
							k = 2;
							while (rq[k] != ';') k++;
							k++;
							w = k;
							while (rq[k] != ';') k++;
							rq[k] = 0;
							k++;
							n = atoi(&rq[k]) - 1;
							
							strcpy(ports[p].cmd, &rq[w]);
							r = strlen(ports[p].cmd);
							ports[p].cmd[r] = 13;
							ports[p].cmd[r+1] = 10;
							ports[p].cmd[r+2] = 0;
							char ch = 13;
//							fprintf(log_file, "In port write %i byte: [", r);
//							for (r = 0; r < n + 1; r++)
//							{
//								fprintf(log_file, "%x ", rq[w+r]);
//							}
							r = write(ports[p].fd, &rq[w], n + 1);
							write(ports[p].fd, &ch, 1);
//							fprintf(log_file, "]\n");
//							fflush(log_file);
							
							sprintf(rq, "OK");
							r = write(sockets[i], rq, 3);
						} else {
							tstr = malloc(100);
							sprintf(tstr, "We do not listen requsted port, enable it in kdb\n");
							write(sockets[i], tstr, strlen(tstr)+1);
							free(tstr);
						}
						close(sockets[i]);
						sockets[i] = 0;
					break;
					// request to write with '\t'
					case 't':
						if (ports[atoi(&rq[2])].fd != -1)
						{
							p = atoi(&rq[2]);
							k = 2;
							while (rq[k] != ';') k++;
							k++;
							w = k;
							while (rq[k] != ';') k++;
							rq[k] = 0;
							k++;
							n = atoi(&rq[k]) - 1;
							
							r = write(ports[p].fd, &rq[w], n); // n + 1
							char ch = '\t';
///							write(ports[p].fd, &ch, 1);
//							rq[0] = 0x1B;
//							rq[1] = 0x5B;
//							rq[2] = 0x35;
//							rq[3] = 0x44;
//							rq[4] = 0x0D;
//							rq[5] = 0x7A;
//							rq[6] = 0x65;
//							rq[7] = 0x6C;
//							rq[8] = 0x61;
//							rq[9] = 0x78;
//							rq[10] = 0x23;
//							rq[11] = 0x20;
//							write(ports[p].fd, rq, 4);
							
							sprintf(rq, "OK");
							r = write(sockets[i], rq, 3);
						} else {
							tstr = malloc(100);
							sprintf(tstr, "We do not listen requsted port, enable it in kdb\n");
							write(sockets[i], tstr, strlen(tstr)+1);
							free(tstr);
						}
						close(sockets[i]);
						sockets[i] = 0;
					break;
					// requets to read all
					case 'a':
						cnt = 0;
						tbuff = malloc(buf_size*MAX_PORTS);
						po = sprintf(&tbuff[cnt], "{\"ttylist\":[");
						cnt+=po;
						for (j = 0; j < MAX_PORTS; j++)
						{
//							if (v) printf("port %i fd = %i tbuf = %i hbuf = %i\n", j, ports[j].fd, ports[j].tbuf, ports[j].hbuf);
							if ((ports[j].fd > 0) && (ports[j].tbuf != ports[j].hbuf))
							{
								po = sprintf(&tbuff[cnt], "{\"dev\":\"");
								cnt+=po;
								po = sprintf(&tbuff[cnt], "%s%i", iface_name, j);
								cnt+=po;
								po = sprintf(&tbuff[cnt], "\",\"text\":\"");
								cnt+=po;
								while (1)
								{
									if (ports[j].tbuf != ports[j].hbuf)
									{
										switch (ports[j].buf[ports[j].tbuf])
										{
											case 13:
											break;
											case 10:
												tbuff[cnt] = 10;
												cnt++;
											break;
											default:
												tbuff[cnt] = ports[j].buf[ports[j].tbuf];
												cnt++;
											break;
											case '\\':
												tbuff[cnt] = '\\';
												cnt++;
												tbuff[cnt] = '\\';
												cnt++;
											break;
											case '\"':
												tbuff[cnt] = '\\';
												cnt++;
												tbuff[cnt] = '\"';
												cnt++;
											break;
											
										}
										ports[j].tbuf++;
										if (ports[j].tbuf == buf_size)
										{
											ports[j].tbuf = 0;
										}
									} else {
										break;
									}
								} // while(1)
								po = sprintf(&tbuff[cnt], "\"},");
								cnt+=po;
							} else {
								if ((ports[j].fd > 0) && (ports[j].tbuf == ports[j].hbuf))
								{
									po = sprintf(&tbuff[cnt], "{\"dev\":\"");
									cnt+=po;
									po = sprintf(&tbuff[cnt], "%s%i", iface_name, j);
									cnt+=po;
									po = sprintf(&tbuff[cnt], "\",\"text\":\"\"},");
									cnt+=po;
								}
							}
						}
						if (tbuff[cnt-1] != '[') cnt--;
						po = sprintf(&tbuff[cnt], "]}");
						cnt+=po;
						tbuff[cnt] = 0;

						write(sockets[i], tbuff, cnt);
						
						free(tbuff);
						close(sockets[i]);
						sockets[i] = 0;
						gettimeofday(&tv1, NULL);
//						if (v) printf("time to send = %.6f sec.\n", (tv1.tv_sec * 1E6 + tv1.tv_usec - tv2.tv_sec * 1E6 - tv2.tv_usec) / 1E6);

					break;
				}
			}
		}
	
}

void buff_add(struct port * p, char c)
{
	p->buf[p->hbuf] = c;
	p->hbuf++;
	if (p->hbuf == buf_size)
	{
		p->hbuf = 0;
	}
	if (p->tbuf == p->hbuf)
	{
		p->tbuf++;
		if (p->tbuf == buf_size) p->tbuf = 0;
	}	
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


	int k, n, p, w, po;
	char *tstr;
	char *tbuff;
	int cnt = 0, j;

	while (1) {

		FD_ZERO(&ready);
		int maxfd = 0, ii;

		FD_SET(sock, &ready);
		maxfd = sock;
	
		for (i = 0; i < MAX_PORTS; i++)
		{
			if (ports[i].fd > maxfd) maxfd = ports[i].fd;
			if (ports[i].fd > 0) FD_SET(ports[i].fd, &ready);
		}
		for (i = 0; i < NUM_SOCK; i++)
		{
			if (sockets[i] > maxfd) maxfd = sockets[i];
			if (sockets[i] > 0) FD_SET(sockets[i], &ready);
		}
		gettimeofday(&tv1, NULL);
//		tv.tv_sec = 1;
//		tv.tv_usec = 0;

		select(maxfd + 1, &ready, NULL, NULL, &tv);
		
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		gettimeofday(&tv2, NULL);
		
		
		
		s = -1;
		s = accept(sock, &saddr_out, &sl);
		while ((s != EAGAIN) && (s > 0))
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
			}
			s = accept(sock, &saddr_out, &sl);
		}

		read_from_sockets(ready);
		
		//read data from device on ports
		for (i = 0; i < MAX_PORTS; i++)
		{
			if ((ports[i].fd > 0) && (FD_ISSET(ports[i].fd, &ready)))
			{
				char tbuf[buf_size];
				int qw, qweqwe;
				int rb = read(ports[i].fd, tbuf, buf_size);
				q += rb;
				if (rb > 0)
				{
					qweqwe = 0;
					while (ports[i].cmd[qweqwe]) qweqwe++;
					if (v) printf("cmd = %s\n", ports[i].cmd);
					if (v)
					{
						int wer;
						for (wer = 0; wer < qweqwe; wer++)
						{
							printf("%x ", ports[i].cmd[wer]);
						}
						printf("\n");
					}
					qw = 0;
					if (qweqwe)
					if (rb > qweqwe)
					{
						int l;
						for (l = 0; l < qweqwe; l++)
						{
							if (tbuf[l] != ports[i].cmd[l]) break;
						}
						if (l >= qweqwe)
						{
							qw = qw + qweqwe;
							if (tbuf[qw] == 13) qw += 2;
							strcpy(ports[i].cmd, "");
						}
					} else {
						int l;
						for (l = 0; l < rb; l++)
						{
							if (tbuf[l] != ports[i].cmd[l]) break;
						}
						if (l >= rb)
						{
							qw = qw + rb;
							strcpy(ports[i].cmd, &ports[i].cmd[l]);
//							if (tbuf[qw] == 13) qw += 2;
						}
					}
					if (v) printf("from port readed (%i byte): ", rb);
//					fprintf(log_file, "from port readed (%i byte): ", rb);
//					fflush(log_file);
					for (; qw < rb; qw++)
					{
							char a = tbuf[qw];
							
							if (v) printf("%x", tbuf[qw]);
							if ((v) && ((a >= '0') && (a <= '9') || (a >= 'a') && (a <= 'z') || (a >= 'A') && (a <= 'Z') || (a == '_')))
							if (v) printf("[%c]", a);
							if (v) printf(" ");
//							fprintf(log_file, "%x", tbuf[qw]);
//							if (((a >= '0') && (a <= '9') || (a >= 'a') && (a <= 'z') || (a >= 'A') && (a <= 'Z') || (a == '_'))) fprintf(log_file, "[%c]", a);
//							fprintf(log_file, " ");
//							fflush(log_file);
							switch (out(tbuf[qw], &ports[i]))
							{
							case 1:
								buff_add(ports + i, tbuf[qw]);
								break;
							case 2:
								buff_add(ports + i, '\n');
								break;
							default:
								break;
							}
					}
					if (v) printf("\n");
//					fprintf(log_file, "\n");
//					fflush(log_file);
					for (qw = ports[i].tbuf; qw != ports[i].hbuf; qw++)
					{
						if (qw == buf_size) qw = 0;
						if (qw == ports[i].hbuf) break;
					}
					for (qw = ports[i].tbuf; qw != ports[i].hbuf; qw++)
					{
						if (qw == buf_size) qw = 0;
						if (qw == ports[i].hbuf) break;
					}
				}
			}
		}
	}
}

int init_socket()
{
	struct sockaddr saddr_in;

	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK);
	saddr_in.sa_family = AF_LOCAL;
	strcpy(saddr_in.sa_data, SOCKET_NAME);
	bind(sock, &saddr_in, sizeof(saddr_in));
	listen(sock, 10);
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
	for (i = 0; i< MAX_PORTS; i++)
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
//	fclose(log_file);
	close_all_ports ();
	close_all_sock ();
	close(lock_file);
	unlink("/var/run/tbuffd.pid");
	exit(0);
}

int main (int argc, char **argv)
{
	pid_t pid, i;
	struct sigaction sa;
	
	lock_file = open("/var/run/tbuffd.pid", O_CREAT | O_EXCL | O_RDWR);
	if (lock_file == -1)
	{
		FILE * lf = fopen("/var/run/tbuffd.pid", "r");
		fscanf(lf, "%i", &pid);
		if (v) printf("file locked by process with pid=%i\n", pid);
		fclose(lf);
		if (!kill(pid, 0))
		{
			return 0;
		}
		if (v) printf("this process does not exist!!\n");
		unlink("/var/run/tbuffd.pid");
		lock_file = open("/var/run/tbuffd.pid", O_CREAT | O_EXCL | O_RDWR);
	}
	
//	log_file = fopen("/root/log", "w+");
//	fprintf(log_file, "demon started-----------------\n");
//	fflush(log_file);
//	if (log_file <= 0) printf("log error\n");
	if (argc > 1)
	{
		v = 1;
		sa.sa_handler = handler;
		sigaction(SIGTERM, &sa, 0);
		sigaction(SIGINT, &sa, 0);
		for (i = 0; i < MAX_PORTS; i++)
		{
			ports[i].fd = -1;
		}
		load_params();
		init_socket();
		loop();
		
	} else {
		pid = fork();
		if (pid == 0)
		{
			setsid();
			chdir("/");
			close(0);
			close(1);
			close(2);
			sa.sa_handler = handler;
			sigaction(SIGTERM, &sa, 0);
			sigaction(SIGINT, &sa, 0);
			for (i = 0; i < MAX_PORTS; i++)
			{
				ports[i].fd = -1;
			}
			load_params();
			init_socket();
			loop();
		} else {
			FILE * lf = fopen("/var/run/tbuffd.pid", "w+");
			fprintf(lf, "%i\n", pid);
			fclose(lf);
		}
	}
	return 0;
}
