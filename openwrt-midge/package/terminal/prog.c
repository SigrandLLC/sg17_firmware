#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "md5.h"
#include "kdb.h"

#define MAX_DATA 100
#define SOCKET_NAME "/tmp/socket"

int sock, port, size;

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
	
	sprintf(r, "1;%i;%i", port, size);
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
	sprintf(str, "2;%i;%s;%i;", port, d, strlen(d));
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

int main (int argc, char ** argv)
{
	int i = 0;
	if (argc < 4)
	{
		printf("Usage:\n     for read data form port: %s -pttyUSB0 -r 'how many bytes to read (0 - all buffer)'\n", argv[0]);
		printf("     for write data to port: %s -pttyUSB0 -w 'data to send'\n", argv[0]);
		exit(-1);
	}
	while (argv[1][i] != 0) i++;
	if ((argv[1][i-2] >= '0') && (argv[1][i-2] <= '9')) port += (argv[1][i-2]-'0')*10;
	if ((argv[1][i-1] >= '0') && (argv[1][i-1] <= '9'))
	{
		port += argv[1][i-1] - '0';
	} else {
		printf("Error: check port name\n");
		exit(-1);
	}
	
	struct sockaddr saddr;
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

	switch (argv[2][1])
	{
		case 'r':
			size = atoi(argv[3]);
			if (read_form_port())
			{
				printf("Error: cannot read data from port\n");
				exit(-1);
			}
		break;
		case 'w':
			if (write_in_port(argv[3]))
			{
				printf("Error: cannot write data in port\n");
				exit(-1);
			}
			
		break;
	}
	
	close(sock);

	return 0;
}
