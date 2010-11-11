#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include "md5.h"
#include "kdb.h"
#include "tbuffd.h"


int sock, port, size;
struct timeval tv1, tv2;

int read_form_port (void)
{
	int ret, buf_size, i;
	char *opt, *buf, r[10];

	// read buf_size from kdb
	kdbinit();
	ret = kdb_appget("sys_demon_buf_size", &opt);
	if (ret != 1)
	{
		printf("Error: cannot read buf size from kdb\n");
		exit(-1);
	}
	buf_size = atoi(opt);
//	printf(">>Debug: buf_size = %i\n", buf_size);
	db_close();
	if ((size == 0)||(size > buf_size)) size = buf_size;

//	printf(">>Debug: opt: [%s]\n", opt);

	sprintf(r, "r;%i;%i", port, size);
	ret = write(sock, r, strlen(r)+1);
//	printf(">>Debug: we write to socket %i byte\n", ret);
	if (ret < strlen(r))
	{
		printf("Error: cannot write data in socket");
		exit(-1);
	}

	buf = malloc(buf_size);
	if (!buf)
	{
		printf("Error: cannot allocate memory\n");
		exit(-1);
	}
	ret = read(sock, buf, buf_size);

	if (ret == -1)
	{
		printf("Error: cannot read from socket, %s\n", strerror(errno));
		exit(-1);
	}
	if ((ret != 1) || (buf[0] != -1))
	{
		i = 0;
	//	printf("Buffer:\n");
		while (i < ret)
		{
			printf("%c", buf[i]);
			i++;
		}
		printf("\n");
	//	printf("--------\n");
	} else {
//		printf("Debug: buff is empty\n")
	}


	return 0;
}

int write_in_port (char * d)
{
	int i = 0;
	char str[MAX_DATA];
	sprintf(str, "w;%i;%s;%i;", port, d, strlen(d));
//	printf(">>Debug: str = [%s]\n", str);

	if (write(sock, str, strlen(str)) != (strlen(str)))
	{
		printf("Error: cannot write in socket\n");
		exit(-1);
	}
	strcpy(str, "");
	i = read(sock, &str, 100);
	str[i] = 0;
//	if (i <= 0)
//	{
//		printf("Error: cannot read from socket\n");
//		exit(-1);
//	}
//	printf(">>Debug: answer %i byte = [%s]\n", i, str);
//	if (strcmp(str, "OK") == 0)
//	{
//		printf("Data successfully sent\n");
//	}
	if (strcmp(str, "NONE") != 0) printf("%s", str);
	return 0;
}


int write_in_port_t (char * d)
{
	int i = 0, j;
	char str[MAX_DATA];
	sprintf(str, "t;%i;%s;%i;", port, d, strlen(d) + 1);
//	printf(">>Debug: str = [%s]\n", str);

	if (write(sock, str, strlen(str)) != (strlen(str)))
	{
		printf("Error: cannot write in socket\n");
		exit(-1);
	}
	strcpy(str, "");
	i = read(sock, &str, 10);
	if (i <= 0)
	{
		printf("Error: cannot read from socket\n");
		exit(-1);
	}
//	printf(">>Debug: answer %i byte = [%s]\n", i, str);
	if (strcmp(str, "OK") == 0)
	{
//		printf("Data successfully sent\n");
	}
	return 0;
}

int write_in_port_ch (unsigned char ch)
{
	int i = 0;
	char str[MAX_DATA];
	sprintf(str, "c;%i;%c;%i;", port, ch, 1);

	if (write(sock, str, strlen(str)) != (strlen(str)))
	{
		printf("Error: cannot write in socket\n");
		exit(-1);
	}
	strcpy(str, "");
	i = read(sock, &str, 100);
	str[i] = 0;
	if (i <= 0)
	{
		printf("Error: cannot read from socket\n");
		exit(-1);
	}
//	printf(">>Debug: answer %i byte = [%s]\n", i, str);
//	if (strcmp(str, "OK") == 0)
//	{
//		printf("Data successfully sent\n");
//	}
	if (strcmp(str, "NONE") != 0)
	{
//		FILE *log;
//		log = fopen("/root/log", "a");
//		for (i = 0; i < strlen(str); i++)
//		{
//			fprintf(log, "%i", str[i]);
//			if (str[i] > 13) fprintf(log, "[%c] ", str[i]); else fprintf(log, " ");
//		}
//		fprintf(log, "\n");
//		fclose(log);
		for (i = 0; i < 2*65535; i++);
		printf("%s", str);// else printf("NONEEEEEEEEEE\n");
		for (i = 0; i < 4*65535; i++);
	}
	return 0;
}


int write_in_port_d ()
{
	int i = 0;
	char str[MAX_DATA];
	sprintf(str, "d;%i;\b;%i;", port, 100);

	if (write(sock, str, strlen(str)) != (strlen(str)))
	{
		printf("Error: cannot write in socket\n");
		exit(-1);
	}
	strcpy(str, "");
	i = read(sock, &str, 10);
	if (i <= 0)
	{
		printf("Error: cannot read from socket\n");
		exit(-1);
	}
//	printf(">>Debug: answer %i byte = [%s]\n", i, str);
	if (strcmp(str, "OK") == 0)
	{
//		printf("Data successfully sent\n");
	}
	return 0;
}


int read_from_all_ports()
{
	char r = 'a';
	int ret, buf_size, i;
	char *opt, *buf;
	fd_set ready;
	struct timeval tv;
//	gettimeofday(&tv1, NULL);
	// read buf_size from kdb
//	kdbinit();
//	ret = kdb_appget("sys_demon_buf_size", &opt);
//	if (ret != 1)
//	{
//		printf("Error: cannot read buf size from kdb\n");
//		exit(-1);
//	}
//	buf_size = atoi(opt);


	buf_size = 4096;

//	printf(">>Debug: buf_size = %i\n", buf_size);
//	db_close();
//	if ((size == 0)||(size > buf_size)) size = buf_size;
	write(sock, &r, 1);
	buf = malloc(buf_size*MAX_PORTS);
	gettimeofday(&tv2, NULL);
//	printf("time1 = %.6f sec.\n", (tv2.tv_sec * 1E6 + tv2.tv_usec - tv1.tv_sec * 1E6 - tv1.tv_usec) / 1E6);


	FD_ZERO(&ready);
	FD_SET(sock, &ready);
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	select(sock + 1, &ready, NULL, NULL, &tv);
	if (FD_ISSET(sock, &ready))
	{
//		printf("select %.6f\n", 0.5 - (tv.tv_sec * 1E6 + tv.tv_usec) / 1E6);
		ret = read(sock, buf, buf_size*MAX_PORTS);
	} else {
		ret = read(sock, buf, buf_size*MAX_PORTS);
	}
//	printf("ret = %i\n", ret);
	buf[ret] = 0;
	gettimeofday(&tv1, NULL);
//	printf("time2 = %.6f sec.\n", (tv1.tv_sec * 1E6 + tv1.tv_usec - tv2.tv_sec * 1E6 - tv2.tv_usec) / 1E6);
	printf("%s", buf);
//	gettimeofday(&tv2, NULL);
//	printf("time3 = %.6f sec.\n", (tv2.tv_sec * 1E6 + tv2.tv_usec - tv1.tv_sec * 1E6 - tv1.tv_usec) / 1E6);
//	for (i = 0; i < ret; i++)
//	{
//		printf("%c", buf[i]);
//	}
	return 0;
}

int main (int argc, char ** argv)
{
	int i = 0;
	const char *query = getenv("QUERY_STRING");
	char * endptr;
	char action;
	char data[1000];
	struct sockaddr saddr;

	if (argc < 2)
	{
//		printf("%s\n", getenv("REQUEST_METHOD"));
		if (getenv("REQUEST_METHOD") != NULL)
		{
//			printf("query = %s\nlen = %i\n", query, strlen(query));
			i = 0;
			while (!isdigit(query[i])) i++;
			port = strtol(&query[i], &endptr, 10);
			while (query[i]!= ';') i++;
			action = query[i+1];
			i+=3;
			strcpy(data, &query[i]);
//			printf("port = %i\naction = %c\ndata = %s\n", port, action, data);
			goto doo;
		} else {
			printf("Usage:\n     for read data form port: %s -pttyUSB0 -r 'how many bytes to read (0 - all buffer)'\n", argv[0]);
			printf("     for write data to port: %s -pttyUSB0 -w 'data to send'\n", argv[0]);
			printf("     for read data from all ports: %s -p* -a'\n", argv[0]);
			exit(-1);
		}
	}
	if (argv[1][1] == 'p')
	{
		if (argc < 3)
		{
			printf("Usage:\n     for read data form port: %s -pttyUSB0 -r 'how many bytes to read (0 - all buffer)'\n", argv[0]);
			printf("     for write data to port: %s -pttyUSB0 -w 'data to send'\n", argv[0]);
			printf("     for read data from all ports: %s -p* -a'\n", argv[0]);
			exit(-1);
		}
		if (argc == 4)
		{
			while (argv[1][i] != 0) i++;
			if ((argv[1][i-2] >= '0') && (argv[1][i-2] <= '9')) port += (argv[1][i-2]-'0')*10;
			if ((argv[1][i-1] >= '0') && (argv[1][i-1] <= '9'))
			{
				port += argv[1][i-1] - '0';
			} else {
				printf("Error: check port name\n");
				exit(-1);
			}
		}
	} else {
		printf("Usage:\n     for read data form port: %s -pttyUSB0 -r 'how many bytes to read (0 - all buffer)'\n", argv[0]);
		printf("     for write data to port: %s -pttyUSB0 -w 'data to send'\n", argv[0]);
		printf("     for read data from all ports: %s -p* -a'\n", argv[0]);
		exit(-1);
	}
	action = argv[2][1];
	if (argc == 4) strcpy(data, argv[3]);

doo:

	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (!sock)
	{
		printf("Error: cannot create socket\n");
		exit(-1);
	}
	saddr.sa_family = AF_LOCAL;
	strcpy(saddr.sa_data, SOCKET_NAME);
	if (connect(sock, &saddr, sizeof(saddr)))
	{
		printf("Error: cannot connect to socket\n");
		exit(-1);
	}
	switch (action)
	{
		case 'r':
			size = atoi(data);
			if (read_form_port())
			{
				printf("Error: cannot read data from port\n");
				exit(-1);
			}
		break;
		case 'w':
			if (write_in_port(data))
			{
				printf("Error: cannot write data in port\n");
				exit(-1);
			}

		break;
		case 'a':
			if (read_from_all_ports())
			{
				printf("Error: cannot write data in port\n");
				exit(-1);
			}
		break;
		case 't':
			if (write_in_port_t(data))
			{
				printf("Error: cannot write data in port\n");
				exit(-1);
			}
		break;
		case 'c':
			if (write_in_port_ch((unsigned char)atoi(data)))
			{
				printf("Error: cannot write data in port\n");
				exit(-1);
			}
		break;
		case 'd':
			write_in_port_d();
		break;


	}

	close(sock);

	gettimeofday(&tv2, NULL);
//	printf("time = %.6f sec.\n", (tv2.tv_sec * 1E6 + tv2.tv_usec - tv1.tv_sec * 1E6 - tv1.tv_usec) / 1E6);
	return 0;
}
