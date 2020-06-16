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
#include <sys/time.h>

int clientCount = 0;
int groupCount = 0;

#define BUFSIZE 48000
#define RAND_MAX 1000


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct client{
	int index;
	char username[1024];
	int sockID;
	struct sockaddr_in clientAddr;
	int len;

};

struct group{

	int index;
	int no_people;
	char username_g[1024];
	int people[1024];

};


struct client Client[1024];
struct group Group[1024];


pthread_t thread[1024];


int universalBuff[BUFSIZE];
void * reception(void * attrClient){

//	Client attributes

	struct client* attrClient1 = (struct client*) attrClient;
	int index = attrClient1 -> index;
	int clientSocket = attrClient1 -> sockID;
	char username[1024];
		
//	client username

	recv(clientSocket,username,1024,0);
	strcpy(Client[index].username,username);


	int mine=0;
	int data[BUFSIZE];
	while(1)
	{



		int read = recv(clientSocket,data,BUFSIZE,0);
		if(read==-1){printf("Error receiving ");}
		send(clientSocket,data,BUFSIZE,NULL);
	
	}
	return NULL;



}
	

int main(int argc,char *argv[]){

	int port;
	port = atoi(argv[1]);
	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

//	Server attributes

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

//	Bind to port

	if(bind(serverSocket,(struct sockaddr *) &serverAddr , sizeof(serverAddr)) == -1) return 0;

//	Listen

	if(listen(serverSocket,1024) == -1) return 0;
	printf("PORT NO %d\n",port);


	while(1){ 

		Client[clientCount].sockID = accept(serverSocket, (struct sockaddr*) &Client[clientCount].clientAddr, &Client[clientCount].len);
		Client[clientCount].index = clientCount;
		pthread_create(&thread[clientCount+groupCount], NULL, reception, (void *) &Client[clientCount]);
		clientCount ++;
	}

	for(int i = 0 ; i < clientCount ; i ++)
		pthread_join(thread[i],NULL);

}
