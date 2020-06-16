#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include <sys/time.h>
#include <errno.h>

#define BUFSIZE 48000
#define RAND_MAX 1000

void * reception(void * sockID){


		pa_simple *s2;
		pa_sample_spec ss;
		ss.format = PA_SAMPLE_S16NE;
		ss.channels = 2;
		ss.rate = 48000;
		int clientSocket = *((int *) sockID);
	
	    srand(time(0)); 
		char name[]="talking-";
		char randNum[4];
		printf(randNum,"%d",rand());
		strcat(name,randNum);
		s2 = pa_simple_new(NULL,name,PA_STREAM_PLAYBACK,NULL,name,&ss,NULL,NULL,NULL);

// receive from server and playback 

		while(1)
		{
			int data[BUFSIZE];
    			int error;
			int read = recv(clientSocket,data,BUFSIZE,NULL);
			if(read==-1){printf("FAILED TO RECIEVE FROM SERVER\n");}
			pa_simple_write(s2,data,BUFSIZE,NULL);
	    		pa_simple_flush(s2,NULL);
			fflush(stdin);
		}



}


void catch(int dummy)
	{
     	char  c;
     	signal(dummy, SIG_IGN);
     	printf("EXIT? [y/n] ");
     	c = getchar();
     	if (c == 'y' || c == 'Y')
        	exit(0);
     	else
        	signal(SIGINT, catch);
     	getchar();
	}

int main(int argc,char *argv[]){
	int port;
	char username[100];
	
	port = atoi(argv[2]);
	strcpy(username,argv[1]);

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);

//	Server attributes
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	printf("Connected to the server successfuly\n");
	printf("My PORT NO:  %d\n",clientSocket);

//	Sending Username
	send(clientSocket,username,1024,0);


	pthread_t thread;
	pthread_create(&thread, NULL, reception, (void *) &clientSocket );

//	SIGINT Signal
	signal(SIGINT, catch);

//	DECLARING AND SETTING ATTRIBUTES OF AUDIO - SAMPLING AND NO OF CHANNELS

	pa_simple *s1;
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16NE;
	ss.channels = 2;
	ss.rate = 48000;
	int buf[BUFSIZE];



	srand(time(0)); 
	char name[]="listening-";
	char randNum[4];
	sprintf(randNum,"%d",rand());
	strcat(name,randNum);
	char ch[2];

	s1 = pa_simple_new(NULL,name,PA_STREAM_RECORD,NULL,name,&ss,NULL,NULL,NULL);
	while(1)
	{
//	Record audio	
		if( pa_simple_read(s1,buf,BUFSIZE,NULL)<0){printf("error recording\n");}

		pa_simple_flush(s1,NULL);

//	Send the recorded audiio
		if(send(clientSocket,buf,BUFSIZE,NULL)==-1){printf("FAILED TO SEND TO SERVER\n");};
		fflush(stdin);
	}
}
