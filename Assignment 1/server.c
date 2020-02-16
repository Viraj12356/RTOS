#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <pthread.h>

struct clients
 {
    int id;
    int ip;
}cl;

int client_index[100];
int n=0;
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int i,j;
    clock_t t; 
    struct clients cl1 = *((struct clients *)socket_desc);

    int read_size;
    char *message , client_message[2000];

    //Receive a message from client
    while( (read_size =recv(cl1.id , client_message , 2000 , 0)) > 0 )
    {
	printf("%s\n",client_message);       
	//Send message to  all remaining clients in group 
    	t = clock(); 
	for(i=0;i<n;i++)
	{
	   if(client_index[i]!=cl1.id)
	   {
		//printf("Client id: %d",cl1.id);
        	send( client_index[i], client_message , strlen(client_message),0);
        	//printf("%s\n",client_message);
	   }
	}
	t = clock() - t; 
    	double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  
    	printf("fun() took %f seconds to execute \n", time_taken); 
	for(int i=0;i<2000;i++)
	{
	  client_message[i]=0;	
	}
    }
    
   if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    for(i = 0; i < n; i++) {
		if(clients[i] == cl.sockno) {
			j = i;
			while(j < n-1) {
				clients[j] = clients[j+1];
				j++;
			}
		}
	}
	n--;
    else if(read_size == -1)
    {
        perror("recv failed");
    }

}


int main(int argc, char *argv[])
{
    int socket_desc ,client_sock, c , *new_sock,i;
    pthread_t sendt,recvt;
    struct sockaddr_in serv_addr,client;
    int addrlen = sizeof(struct sockaddr_in );
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("192.168.43.249");
    serv_addr.sin_port = htons(5000); 

    bind(socket_desc, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
   
    listen(socket_desc, 150); 
    
    while((client_sock = accept(socket_desc, (struct sockaddr *)&client,  
                       (socklen_t*)&addrlen)))
    {	
	puts("Connection accepted");
	cl.id=client_sock;
	client_index[n]=client_sock;
	n++;
        if( pthread_create( &recvt , NULL ,  connection_handler , (void*) &cl) < 0)
        {
            perror("could not create thread");
            return 1;
        }
	
        puts("Handler assigned");
    

     }    
return 0; 
} 
