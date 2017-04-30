/* This page contains the client program. The following one contains the
* server program. Once the server has been compiled and started, clients
* anywhere on the Internet can send commands (file names) to the server.
* The server responds by opening and returning the entire file requested.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
//for FILE and fwrite
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#define SERVER_UDP_PORT 2559
/* arbitrary, but client and server must agree */
#define MAXLEN 4096
#define DEFLEN 64

/* block transfer size */
int main(int argc, char **argv)
{
	int data_size = DEFLEN, port = SERVER_UDP_PORT;
        int     i, j, sd, server_len;
        char    *pname, *host, rbuf[MAXLEN], sbuf[MAXLEN];
        struct  hostent *hp;
        struct  sockaddr_in server;
        struct  timeval start, end;
        unsigned long address;
	
	pname = argv[0];
	argc--;
	argv++;

	if (argc > 0 && (strcmp(*argv, "-s") == 0)) {
		if (--argc > 0 && (data_size = atoi(*++argv))) 
		{                       
			argc--;                      
			argv++;                
		}                
		else 
		{                       
			fprintf(stderr, "Usage: %s [-s data_size] host [port]\n", pname);                      
			exit(1);   
		}         
	}
	if (argc > 0) 
	{                 
		host = *argv;                 
		if (--argc > 0)                         
			port = atoi(*++argv);         
	} 
 	else 
	{   
		fprintf(stderr, "Usage: %s [-s data_size] host [port]\n", pname);                exit(1);  
	}
	
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{                 
		fprintf(stderr, "Can't create a socket\n");                 
		exit(1);         
	} 
	bzero((char *)&server, sizeof(server));         
	server.sin_family = AF_INET;         
	server.sin_port = htons(port);

	if ((hp = gethostbyname(host)) == NULL) 
	{                 
		fprintf(stderr, "Can't get server's IP address\n");
		exit(1);         
	}
	bcopy(hp->h_addr, (char *) &server.sin_addr, hp->h_length);

	if (data_size > MAXLEN) 
	{                 
		fprintf(stderr, "Data is too big\n");                 
		exit(1);         
	}

	for (i = 0; i < data_size; i++) 
	{                 
		j = (i < 26) ? i : i % 26;                 
		sbuf[i] = 'a' + j;         
	} // construct data to send to the server

	gettimeofday(&start, NULL); /* start delay measurement */  
	server_len = sizeof(server);

	if (sendto(sd, sbuf, data_size, 0, (struct sockaddr *)                 
		&server, server_len) == -1) {                 
		fprintf(stderr, "sendto error\n");                 
		exit(1);         
	}

	if (recvfrom(sd, rbuf, MAXLEN, 0, (struct sockaddr *)                 
		&server, &server_len) < 0) {                 
		fprintf(stderr, "recvfrom error\n");                 
		exit(1);         
	}
	
	 gettimeofday(&end, NULL); /* end delay measurement */         
	if (strncmp(sbuf, rbuf, data_size) != 0)                 
		printf("Data is corrupted\n");         
	close(sd);         
	return(0); 
}
long delay(struct timeval t1, struct timeval t2)
{
	long d;
	d = (t2.tv_sec - t1.tv_sec) * 1000;
	d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);
	return(d);
	
}
