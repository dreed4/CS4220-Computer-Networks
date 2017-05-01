#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define SERVER_UDP_PORT 2559 /* arbitrary, but client and server
must agree */
#define MAXLEN 4096 /* block transfer size */

int shoulddrop(int prob);

int main(int argc, char *argv[])
{
	
	srand(time(NULL));
	int  sd, client_len, port, n, m, bytes, fd, arqprot, ackbuf;  
	char  buf[MAXLEN], sendbuf[MAXLEN], strackbuf[4];
	struct  sockaddr_in  server, client;
	struct timeval start, end;
	double elapsedTime;
	int  prob = 0.1;
	arqprot = 3;
	printf("beginning of fn\n");
 
 	switch(argc) {  
	case 3:   
		port = SERVER_UDP_PORT;
		arqprot = atoi(argv[1]);
		prob = atoi(argv[2]);	
		break;  
	case 4:   
		port = atoi(argv[1]);
		arqprot = atoi(argv[2]);
		prob = atoi(argv[3]);
		break;  
	default:   
		fprintf(stderr, "Usage: %s [port] [protocol] [int percentage packet drop]\n", argv[0]);   exit(1);  
	}

	/* Create a datagram socket */
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { 
  		fprintf(stderr, "Can't create a socket\n");   
		exit(1);  
	}
	
	/* Bind an address to the socket */  
	bzero((char *)&server, sizeof(server));  
	server.sin_family = AF_INET;  
	server.sin_port = htons(port);  
	server.sin_addr.s_addr = htonl(INADDR_ANY);  
	if (bind(sd, (struct sockaddr *)&server,   sizeof(server)) == -1) 
	{   
		fprintf(stderr, "Can't bind name to socket\n");   
		exit(1);  
	}
	printf("before while loop\n");	
	int sendfile = 0;
	
	while (arqprot == 1) {   
		
		printf("beginning of while loop\n");
		client_len = sizeof(client);   
		printf("got past client_len = ..\n");
		sendfile = 0;
		
		if ((n = recvfrom(sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0) {         
		fprintf(stderr, "Can't receive datagram\n");         
		exit(1);
		}
		else
		{
			printf("buf: %s\n", buf);
			printf("n: %i\n", n);    
		}
		
		if (n>0)
			sendfile = 1;
			fd = open(buf, O_RDONLY);
			if (fd<0) 
			{
				fprintf(stderr,"open failed");
				exit(1);
			}
		printf("made it this far...\n");	
  		//for testing, right now we're just sending the filename
  		//back to the client
  		/*
		if (sendto(sd, buf, strlen(buf), 0,    
		(struct sockaddr *)&client, client_len) != n) {         
			fprintf(stderr, "Can't send datagram\n");         
			exit(1);   
		}
		*/

		//loop to start sending blocks/packets
		while(sendfile == 1) {
			bytes = read(fd, sendbuf, MAXLEN);
			printf("bytes: %i\n", bytes);
			if (bytes == 0) break; //end of file
			if (bytes == -1)
			{
				printf("bytes returned: %i\n", bytes);
				printf("exiting with error..\n");
				exit(1);
			}
			/*
			if (sendto(sd, sendbuf, bytes, 0,
			(struct sockaddr *)&client, client_len) != strlen(sendbuf)) {
				fprintf(stderr, "can't send datagram\n");
				exit(1);
			}
			*/
			
			gettimeofday(&start, NULL);
			
			int drop = shoulddrop(prob);
			if(drop != 1)
			{
				m = sendto(sd, sendbuf, bytes, 0, (struct sockaddr *)&client, client_len);
				
			}
			else
			{
				printf("Packet was dropped\n");
			}
			//printf("sendbuf sent: %s\n", sendbuf);
			while(1)
			{
				n = 0;
				//printf("in waiting loop\n");
				if((n = recvfrom(sd, strackbuf, MAXLEN, MSG_DONTWAIT, (struct sockaddr *)&client, &client_len)) < 0) {         
					//printf("n: %i\n", n);
					gettimeofday(&end, NULL);
					elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0;
					if(elapsedTime >= 300)
					{
						drop = shoulddrop(prob);
						if(drop != 1)
						{
							m = sendto(sd, sendbuf, bytes, 0, (struct sockaddr *)&client, client_len);
						}
						else
						{
							printf("Packet was dropped\n");
						}
						gettimeofday(&start, NULL);
						continue;
					}
					continue;
				}	
				else
				{
					ackbuf = atoi(strackbuf);
					printf("ackbuf: %i\n", ackbuf);
					if(ackbuf == m)
					{
						printf("acknowledgement Recieved..\n");
						break;
					}
				}
			}
		}
		printf("finished sending file..\n");
		close(fd);   
	}
	
	while (arqprot == 2) {   
		printf("beginning of while loop\n");
		client_len = sizeof(client);   
		printf("got past client_len = ..\n");
		sendfile = 0;
		if ((n = recvfrom(sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0) {         
		fprintf(stderr, "Can't receive datagram\n");         
		exit(1);
		}
		else
		{
			printf("buf: %s\n", buf);
			printf("n: %i\n", n);    
		}
		if (n>0)
			sendfile = 1;
			fd = open(buf, O_RDONLY);
			if (fd<0) 
			{
				fprintf(stderr,"open failed");
				exit(1);
			}
		printf("made it this far...\n");	
  		//for testing, right now we're just sending the filename
  		//back to the client
  		/*
		if (sendto(sd, buf, strlen(buf), 0,    
		(struct sockaddr *)&client, client_len) != n) {         
			fprintf(stderr, "Can't send datagram\n");         
			exit(1);   
		}
		*/

		//loop to start sending blocks/packets
		while(sendfile == 1) {
			bytes = read(fd, sendbuf, MAXLEN);
			printf("bytes: %i\n", bytes);
			if (bytes == 0) break; //end of file
			if (bytes == -1)
			{
				printf("bytes returned: %i\n", bytes);
				printf("exiting with error..\n");
				exit(1);
			}
			/*
			if (sendto(sd, sendbuf, bytes, 0,
			(struct sockaddr *)&client, client_len) != strlen(sendbuf)) {
				fprintf(stderr, "can't send datagram\n");
				exit(1);
			}
			*/
			
			sendto(sd, sendbuf, bytes, 0,
                        (struct sockaddr *)&client, client_len);
			//printf("sendbuf sent: %s\n", sendbuf);
			usleep(300);
		}
		printf("finished sending file..\n");
		close(fd);   
	}
	
	while (arqprot == 3) {   
		printf("beginning of while loop\n");
		client_len = sizeof(client);   
		printf("got past client_len = ..\n");
		sendfile = 0;
		if ((n = recvfrom(sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0) {         
		fprintf(stderr, "Can't receive datagram\n");         
		exit(1);
		}
		else
		{
			printf("buf: %s\n", buf);
			printf("n: %i\n", n);    
		}
		if (n>0)
			sendfile = 1;
			fd = open(buf, O_RDONLY);
			if (fd<0) 
			{
				fprintf(stderr,"open failed");
				exit(1);
			}
		printf("made it this far...\n");	
  		//for testing, right now we're just sending the filename
  		//back to the client
  		/*
		if (sendto(sd, buf, strlen(buf), 0,    
		(struct sockaddr *)&client, client_len) != n) {         
			fprintf(stderr, "Can't send datagram\n");         
			exit(1);   
		}
		*/

		//loop to start sending blocks/packets
		while(sendfile == 1) {
			bytes = read(fd, sendbuf, MAXLEN);
			printf("bytes: %i\n", bytes);
			if (bytes == 0) break; //end of file
			if (bytes == -1)
			{
				printf("bytes returned: %i\n", bytes);
				printf("exiting with error..\n");
				exit(1);
			}
			/*
			if (sendto(sd, sendbuf, bytes, 0,
			(struct sockaddr *)&client, client_len) != strlen(sendbuf)) {
				fprintf(stderr, "can't send datagram\n");
				exit(1);
			}
			*/
			
			sendto(sd, sendbuf, bytes, 0,
                        (struct sockaddr *)&client, client_len);
			//printf("sendbuf sent: %s\n", sendbuf);
			usleep(300);
		}
		printf("finished sending file..\n");
		close(fd);   
	}
	close(sd);
	return(0);
}

int shoulddrop(int prob)
{
	int random_number = rand();
	int n = 99;
	int rand_capped = random_number % n;//between 0 and 99
	
	int amount = prob;
	int actualhigh = amount - 1;
	int actuallow = 0;
	int dodrop = 0;
	if (rand_capped >= actuallow && rand_capped <= actualhigh)
	{
		dodrop = 1;
	}

	return dodrop;
}
long delay(struct timeval t1, struct timeval t2)
{
	long d;
	d = (t2.tv_sec - t1.tv_sec) * 1000;
	d += ((t2.tv_usec - t1.tv_usec + 500)/1000);
	return(d);
	

}
