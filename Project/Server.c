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
#include <mcrypt.h>

int clientCount = 0;
int groupCount = 0;

#define BUFSIZE 44100


char* IV = "43212367835abcd";
char *key = "0123456789abcdef";
int key_len = 16; /* 128 bits */
char* buffer;
int buffer_len = 128;

int encrypt(
    void* buffer)
	{
  MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);
  if( buffer_len % blocksize != 0 ){return 1;}

  mcrypt_generic_init(td, key, key_len, IV);
  mcrypt_generic(td, buffer, buffer_len);
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);
  
  return 0;
}

int decrypt(
    void* buffer
){
  MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);
  if( buffer_len % blocksize != 0 ){return 1;}
  
  mcrypt_generic_init(td, key, key_len, IV);
  mdecrypt_generic(td, buffer, buffer_len);
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);
  
  return 0;
}

void display(char* ciphertext, int len){
  int v;
  for (v=0; v<len; v++){
    printf("%x ", ciphertext[v]);
  }
  printf("\n");
}


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


unsigned char universalBuff[BUFSIZE];
void * reception(void * attrClient){

//	Client attributes
	struct client* attrClient1 = (struct client*) attrClient;
	int index = attrClient1 -> index;
	int clientSocket = attrClient1 -> sockID;
	char username[1024];
		
//	Receive client username
	recv(clientSocket,username,1024,0);
	strcpy(Client[index].username,username);
	printf("Client %s has joined and ID : %d is given.\n",username,index + 1);



	int mine=0;
	uint8_t data[BUFSIZE];
	while(1)
	{

        //Decrypt and send to everyone
		if(memcmp(data,universalBuff,sizeof(data))){
			memcpy(data,universalBuff,sizeof(data));
			printf("Contents of the buffer changed.\n");
			 if(send(clientSocket,data,BUFSIZE,NULL)==-1){printf("Error sending\n");}
			 else{printf("SENT NEW ONE");}
			}

		int read = recv(clientSocket,universalBuff,BUFSIZE,0);
                decrypt(universalBuff);
		//printf("decrypt");
		//display(universalBuff,15);
		if(read==-1){printf("ERROR RECEIVEING ");}
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
