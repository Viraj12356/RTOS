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
#include <mcrypt.h>

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

void * reception(void * sockID){
		pa_simple *s2;
		pa_sample_spec ss;
		ss.format = PA_SAMPLE_S16LE;
		ss.channels = 1;
		ss.rate = 44100;
		int clientSocket = *((int *) sockID);

	        srand(time(0)); 
		char name[]="talking-";
		char randNum[4];
		sprintf(randNum,"%d",rand());
		strcat(name,randNum);
		s2 = pa_simple_new(NULL,name,PA_STREAM_PLAYBACK,NULL,name,&ss,NULL,NULL,NULL);
                uint8_t data1[BUFSIZE];

 // receive decrypted message from server and playback 
		while(1)
		{
		        
    			int error;
			int read = recv(clientSocket,data1,BUFSIZE,NULL);
			if(read==-1){printf("FAILED TO RECIEVE FROM SERVER\n");}               			
			pa_simple_write(s2,data1,BUFSIZE,NULL);
		    	pa_simple_flush(s2,NULL);
			fflush(stdin);
		}




}


void catch(int dummy)
	{
     	char  c;
     	signal(dummy, SIG_IGN);
     	printf("Do you want to exit? [y/n] ");
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

	printf("Connected to server successfully \n");
	printf("My PORT NO:  %d\n",clientSocket);

//	Sending username
	send(clientSocket,username,1024,0);

	pthread_t thread;
	pthread_create(&thread, NULL, reception, (void *) &clientSocket );

//	SIGINT signal

	signal(SIGINT, catch);

//	DECLARING AND SETTING ATTRIBUTES OF AUDIO - SAMPLING AND NO OF CHANNELS

	pa_simple *s1;
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16LE;
	ss.channels = 1;
	ss.rate = 44100;

	uint8_t buf[BUFSIZE];

        srand(time(0)); 
	char name[]="listening-";
	char randNum[4];
	sprintf(randNum,"%d",rand());
        
	strcat(name,randNum);
	char ch[2];

	s1 = pa_simple_new(NULL,name,PA_STREAM_RECORD,NULL,name,&ss,NULL,NULL,NULL);
	while(1)
	{
               

//	Record data	
		if( pa_simple_read(s1,buf,BUFSIZE,NULL)<0){printf("error recording\n");}
		//printf("plaintext");
		//display(buf,15);
		encrypt(buf);
		//printf("cipher");
		//display(buf,15);
		pa_simple_flush(s1,NULL);

//	Send the recorded audio
		if(send(clientSocket,buf,BUFSIZE,NULL)==-1){printf("FAILED TO SEND TO SERVER\n");};
		fflush(stdin);
                
                
	}
  
}
