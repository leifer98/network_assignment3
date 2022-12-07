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
    while (answer == 'Y') {
        printf("hello world");
        //sending first 5 messages
        for(int i = 0; i < 5; i ++){
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
            close(clientSocket);
        }
        // ended sending 5 duplicates

        // recieve authontication message to server with ID's

        // CODE

        // end section for authontication
        printf("Changing algorithm!\n");
        //starting sending 5 duplicates
        for(int i = 0; i < 5; i ++){
            //printf("open socket\n");	
            clientSocket = 0;
            char buffer[SIZE] = {0}; 

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
                printf("\ninet_pton() failed \n"); 
                return -1; 
            }
            
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
            
            printf("connecting...\n");
            if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
            { 
                printf("\nConnection Failed \n"); 
                return -1; 
            } 
                
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
            close(clientSocket);
        }
        //ending sending 5 duplicates
        // USER DECISION
        printf("Would you like to send the file again? Y/N");
        scanf("%c", &answer);
        while (answer != 'Y' && answer != 'N')
        {
            printf("Wrong input, Would you like to send the file again? Y/N\n");
            scanf("%c", &answer);
            printf("\n");
        }
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == -1) {
        printf("Could not create socket : %d", errno);
        return -1;
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_IP_ADDRESS);                                              // (5001 = 0x89 0x13) little endian => (0x13 0x89) network endian (big endian)
    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);  // convert IPv4 and IPv6 addresses from text to binary form
    // e.g. 127.0.0.1 => 0x7f000001 => 01111111.00000000.00000000.00000001 => 2130706433
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }

    // Make a connection to the server with socket SendingSocket.
    int connectResult = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1) {
        printf("connect() failed with error code : %d", errno);
        // cleanup the socket;
        close(clientSocket);
        return -1;
    }

    printf("connected to server\n");

    // Sends some data to server
    char buffer[SIZE] = {'\0'};
    char message[] = "Hello, from the Client\n";
    int messageLen = strlen(message) + 1;
    
    int bytesSent = send(clientSocket, message, messageLen, 0);

    if (bytesSent == -1) {
        printf("send() failed with error code : %d", errno);
    } else if (bytesSent == 0) {
        printf("peer has closed the TCP connection prior to send().\n");
    } else if (bytesSent < messageLen) {
        printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
    } else {
        printf("message was successfully sent.\n");
    }



	return 0; 
} 

