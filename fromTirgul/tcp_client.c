#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <netinet/in.h> 
#include <netinet/tcp.h>
#include <time.h>
#include <errno.h> 
#include <string.h> 

#define PORT 5060 
#define SERVER_IP_ADDRESS "127.0.0.1"
#define SIZE 65536
int main() 
{   

    char answer = 'Y';
	int clientSocket = 0; 
    struct sockaddr_in serv_addr;
    printf("open socket\n");
    char buffer[SIZE] = {0};
    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &serv_addr.sin_addr);
    if(rval<=0)  
    { 
        printf("\ninet_pton() failed.\n"); 
        return -1; 
    } 
        
    // Make a connection to the server with socket SendingSocket.
    printf("connecting...\n");
    int connectResult = connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if ( connectResult < 0) 
    { 
        printf("connect() failed with error code : %d \n", errno); 
        return -1; 
    } 
    printf("connected to server\n");
    while (answer == 'Y') {
        printf("hello world");
        //sending first 5 messages
        for(int i = 0; i < 5; i ++){         
            // Sends some data to server
            printf("sending...\n");
            FILE *fp1; 
            fp1 = fopen("mobydick3times.txt", "r");
            if(fp1 == NULL){
                perror("Error in reading file.");
                return 2;// exit(1);
            }
            printf("Sending file %d...\n",i+1);
            int b = 0;
            int bytesSent = 0;
            int count = 0;
            int amountSeg = 0;
            while((b = fread(buffer, 1, sizeof(buffer), fp1))>0 ){ //while(fgets(buffer, sizeof(buffer), fp1) != NULL) {
                int bytesSent = send(clientSocket, buffer, b, 0); //int bytesSent = send(sock, buffer, sizeof(buffer), 0);
                if (-1 == bytesSent)
                {
                    printf("Error in sending file: %d", errno);
                }
                else if (0 == bytesSent)
                {
                    printf("peer has closed the TCP connection prior to send().\n");
                }
                else if (b > bytesSent) //(sizeof(buffer) > bytesSent)
                {
                    printf("sent only %d bytes from the required %d.\n", bytesSent, b); //printf("sent only %d bytes from the required %d.\n", bytesSent, sizeof(buffer));
                }
                else 
                {
                    printf("Message was successfully sent. Send %d bytes: \n", bytesSent);
                }
                count+=bytesSent;
                amountSeg+=1;
            }
            printf("Send %d bytes of file %d by %d segments.\n", count, i+1, amountSeg);
            //bzero(buffer, SIZE);
            fclose(fp1);
        }
        // ended sending 5 duplicates

        // recieve authontication message to server with ID's

        // CODE

        // end section for authontication
        printf("Changing algorithm!\n");

        //change CC algorithm to reno
        strcpy(buffer, "reno"); 
        if (setsockopt(clientSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, strlen(buffer)) != 0) {
            perror("setsockopt"); 
            return -1;
        }
        socklen_t len = sizeof(buffer);
        if (getsockopt(clientSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, &(len)) != 0) {
            perror("getsockopt"); 
            return -1; 
        } 
        printf("***********Algorithm changed to %s ***********\n" , buffer);      

        //starting sending 5 duplicates
        for(int i = 0; i < 5; i ++){            
            FILE *fp1; 
            printf("Sending file... %d\n",i+1);
            fp1 = fopen("mobydick3times.txt", "rb");
            if(fp1 == NULL){
                perror("Error in reading file.");
                return 2;
            }
            int b = 0;
            int bytesSent = 0;
            int count = 0;
            int amountSeg = 0;
            bzero(buffer, SIZE);
            while((b = fread(buffer, 1, sizeof(buffer), fp1))>0 ){
                bytesSent = send(clientSocket, buffer, b, 0);
        
                if (-1 == bytesSent)
                {
                    printf("Error in sending file: %d", errno);
                }
                else if (0 == bytesSent)
                {
                    printf("peer has closed the TCP connection prior to send().\n");
                }
                else if (b > bytesSent)
                {
                    printf("sent only %d bytes from the required %d.\n", bytesSent, b);
                }
                else 
                {
                    //printf("Message was successfully sent. Send %d bytes: \n", bytesSent);
                }
                count+=bytesSent;
                amountSeg+=1;
            }
            printf("Send %d bytes of file %d by %d segments.\n", count, i+1, amountSeg);
            //bzero(buffer, SIZE);
            fclose(fp1);
        }
        //ending sending 5 duplicates
        // USER DECISION
        printf("Would you like to send the file again? Y/N");
        scanf("%s", &answer);
        while (answer != 'Y' && answer != 'N')
        {
            printf("Wrong input, Would you like to send the file again? Y/N\n");
            scanf("%s", &answer);
            printf("\n");
        }
        // doesnt work yet
        // if (answer == 'Y') {
        //     //change CC algorithm to cubic
        //     char buffer[SIZE] = {0}; 
        //     strcpy(buffer, "cubic"); 
        //     if (setsockopt(clientSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, strlen(buffer)) != 0) {
        //         perror("setsockopt"); 
        //         return -1;
        //     }
        //     socklen_t len = sizeof(buffer);
        //     if (getsockopt(clientSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, &(len)) != 0) {
        //         perror("getsockopt"); 
        //         return -1; 
        //     } 
        //     printf("***********Algorithm changed to %s ***********\n" , buffer);
        // }
    }


    // Sends some data to server
    char message[] = "FINISHED";
    int messageLen = strlen(message) + 1;
    
    int bytesSent = send(clientSocket, message, messageLen, 0);

    if (bytesSent == -1) {
        printf("send() failed with error code : %d", errno);
    } else if (bytesSent == 0) {
        printf("peer has closed the TCP connection prior to send().\n");
    } else if (bytesSent < messageLen) {
        printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
    } else {
        printf("message with %d bytes was successfully sent.\n", bytesSent);
    }
    close(clientSocket);

	return 0; 
} 

