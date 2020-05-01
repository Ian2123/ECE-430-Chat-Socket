/*
	Author:      Ian Isely
	Date:        4/3/20
	Description: This is the server-side code for the chat mechanism with the chat_client.c program.
					 This program utilizes a TCP socket to send and recieve messages from one client.
				    Once the connection is established, a child process is created to constantly
					 read from the socket and print the message, while the parent takes input from the 
					 server user and writes to the socket.

	PS: Execute this program before the client application!
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

#define PORT 60001	//Specific port
#define MAXHOSTNAME 100
#define BUFSIZE 1024

int main(int argc, char const **argv)
{
	int my_socket;
	struct sockaddr_in sin;	//Socket that will remain open for the connection
	int sin_size = sizeof(sin);
	int sd;
	char recvbuf[BUFSIZE], sendbuf[BUFSIZE];
	int i, nbytes;


//Create socket
	if ((my_socket=socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		fprintf(stderr, "%s: cannot creare listening socket: ",argv[0] );
		perror(0);
		exit(1);
	}

//Naming of socket and binding
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(my_socket, (struct sockaddr *) &sin, sin_size)<0)
	{
		fprintf(stderr, "%s: cannot bind listening socket: ",argv[0] );
		perror(0);
		exit(1);
	}


//Initialization of queue for 1 connection
	if (listen(my_socket,1)<0)
	{
		fprintf(stderr, "%s: cannot listen on socket: ",argv[0] );
		perror(0);
		exit(1);
	}


// Ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, quit);



//The server is ready for incmoning clients
	if ((sd=accept(my_socket, (struct sockaddr *) &sin, &sin_size))<0)
		exit(errno);

	printf("Type something to the client! (ctrl+c to quit)\n");
	
	if(fork() == 0) //child reads constantly and prints to screen
	{
		while(1)
		{
			if ((nbytes=read(sd, recvbuf, BUFSIZE-1))<=0)
			{
				printf("Client has exited!\n");
				exit(1);
			}

			for(i=0;i<BUFSIZE-1;i++)
				if(recvbuf[i] == '\n')
				{
					recvbuf[i] = '\0';
					break;
				}
			printf("Client: %s\n", recvbuf);
			usleep(10000);
		}
	}
	else //parent scans for input and writes
	{
		while(1)
		{
			fgets(sendbuf, sizeof(sendbuf), stdin);
			write(sd, sendbuf, strlen(sendbuf));
			usleep(10000);
		}
	}

}

void quit()
{
	exit(1);
}

