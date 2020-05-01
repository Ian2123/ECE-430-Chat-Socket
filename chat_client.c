/*
	Author:      Ian Isely
	Date:        4/3/20
	Description: This is the client-side code for the chat mechanism with the chat_server.c program.
					 This program utilizes a TCP socket to send and recieve messages from the specified
				    server. Once the connection is established, a child process is created to constantly
					 read from the socket and print the message, while the parent takes input from the 
					 client user and writes to the socket.

	Execution: ./client <ip address> 
		- use 127.0.0.1 address for local host.
*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Signalhandler
void quit();

#define BUFSIZE 1024
#define SERVER_PORT 60001	//The port of the server

int main(int argc, char const **argv)
{
	int sd;							//socket descriptor
	struct sockaddr_in server;		//server information
	struct hostent *server_host;
	char recvbuf[BUFSIZE], sendbuf[BUFSIZE];
	int i, nbytes;

//Create a socket
	if ((sd=socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		fprintf(stderr, "%s: cannot create socket: ", argv[0] );
		perror(0);
		exit(1);
	}


//Find server by IP
	if ((server_host=gethostbyname(argv[1]))==NULL)	
	{
		fprintf(stderr, "%s: unknwon host %s\n",argv[0], argv[1] );
		exit(1);
	}

//Save necessary information about to the strcut sockaddr_in 
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	bcopy((char*)server_host->h_addr, (char*)&server.sin_addr, server_host->h_length);



//Connect to the server
	if (connect(sd, (struct sockaddr *) &server, sizeof(server))<0)
	{
		fprintf(stderr, "%s: cannot connect to server: ", argv[0]);
		perror(0);
		exit(1);
	}

//Signalhandler for ending of program
	signal(SIGCHLD, quit);

// Receive data from server
	printf("Type something to the server! (ctrl+c to quit)\n");
	
	if(fork() == 0) //child reads constantly and prints to screen
	{
		while(1)
		{
			if ((nbytes=read(sd, recvbuf, BUFSIZE-1))<=0)
			{
				printf("Server has exited!\n");
				exit(1);
			}

			for(i=0;i<BUFSIZE-1;i++)
				if(recvbuf[i] == '\n')
				{
					recvbuf[i] = '\0';
					break;
				}
			printf("Server: %s\n", recvbuf);
			usleep(10000); //safe delay
		}
	}
	else //parent constantly scans for input and writes
	{
		while(1)
		{
			fgets(sendbuf, sizeof(sendbuf), stdin);
			write(sd, sendbuf, strlen(sendbuf));
			usleep(10000); //safe delay
		}
	}

	return 0;
}


void quit()
{
	exit(1);
}

