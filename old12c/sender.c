#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int main()
{
    // create a socket
    int SenderSocket;
    FILE *file; 
    char *SERVER_IP_ADDRESS = "127.0.0.1";
    int BUFFER_SIZE = 65536;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5093);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    float totalTime = 0;
    char answer = 'y';
    while (answer == 'y')
    {
        printf("Current Congestion Control -> Cubic\n");
        printf("---------------------------------------------------------------------------------------\n");
        // 5 times in Cubic
        for (int i = 0; i < 5; i++)
        {
            SenderSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(SenderSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
            {
                perror("ERROR! connection has failed!");
            }
            else
            {
                printf("You have successfully connected to the server\n");
            }

            file = fopen("mobydick3times.txt", "r");
            if (!file)
            {
                printf("ERROR! file opening has failed!\n");
            }

            // fseek(file, 0, SEEK_END); // seek to end of file
            // int *size = ftell(file); // get current file pointer
            // fseek(file, 0, SEEK_SET); // seek back to beginning of file
            
            clock_t start = clock();
            char data[BUFFER_SIZE];
            while ((fread(data, 1, sizeof data, file)) > 0)
            {
                if (send(SenderSocket, data, sizeof(data), 0) == -1)
                {
                    perror("ERROR! Sending has failed!\n");
                    exit(1);
                }
            }
            if (ferror(file))
            {
                perror("ERROR! file opening has failed!");
            }
            clock_t end = clock();
            totalTime += (float)(end - start);
            close(SenderSocket);
            fclose(file);
        }
        printf("Total sending time with Cubic: %f seconds\n", totalTime / 1000000);
        printf("Average sending time with Cubic: %f seconds\n", totalTime / 5000000);

        // add section to recieve msg
        int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &server_addr.sin_addr);  // convert IPv4 and IPv6 addresses from text to binary form
        if (rval <= 0) {
            printf("inet_pton() failed");
            return -1;
        }
        // Make a connection to the server with socket SendingSocket.
        int connectResult = connect(SenderSocket, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (connectResult == -1) {
            printf("connect() failed with error code : %d", errno);
            // cleanup the socket;
            close(SenderSocket);
            return -1;
        }

        // Receive data from server
        char bufferReply[65536] = {'\0'};
        int bytesReceived = recv(SenderSocket, bufferReply, BUFFER_SIZE, 0);
        if (bytesReceived == -1) {
            printf("recv() failed with error code : %d", errno);
        } else if (bytesReceived == 0) {
            printf("peer has closed the TCP connection prior to recv().\n");
        } else {
            printf("received %d bytes from server: %s\n", bytesReceived, bufferReply);
            // if (bufferReply == 1740887) {
            //     printf("\nsuccecfull!\n")
            // }
        }
    
        close(SenderSocket);
        // end section to recieve msg

        printf("---------------------------------------------------------------------------------------\n");
        printf("Current Congestion Control -> Reno\n");
        printf("---------------------------------------------------------------------------------------\n");
        char CC[256];
        socklen_t socklen;
        // 5 times in reno
        for (int i = 0; i < 5; i++)
        {
            SenderSocket = socket(AF_INET, SOCK_STREAM, 0);
            strcpy(CC, "reno");
            socklen = strlen(CC);
            if (setsockopt(SenderSocket, IPPROTO_TCP, TCP_CONGESTION, CC, socklen) != 0)
            {
                perror("ERROR! socket setting failed!");
                return -1;
            }
            if (connect(SenderSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
            {
                perror("ERROR! connection has failed!");
            }
            else
            {
                printf("You have successfully connected to the server\n");
            }

            file = fopen("mobydick3times.txt", "r");
            if (!file)
            {

                perror("ERROR! file opening has failed!");
            }
            clock_t start = clock();
            char data[BUFFER_SIZE];
            while ((fread(data, 1, sizeof data, file)) > 0)
            {
                if (send(SenderSocket, data, sizeof(data), 0) == -1)
                {
                    perror("ERROR! Sending has failed!\n");
                    exit(1);
                }
            }
            if (ferror(file))
            {
                perror("ERROR! file opening has failed!");
            }
            clock_t end = clock();
            totalTime += (float)(end - start);
            if ( i!= 4) {
                close(SenderSocket);
            }
            fclose(file);
        }
        // USER DECISION
        printf("Would you like to send the file again? y/n\n");
        printf("\n");
        scanf("%s", &answer);
        while (answer != 'y' && answer != 'n')
        {
            printf("Wrong input, Would you like to send the file again? y/n\n");
            printf("\n");
            scanf("%s", &answer);
            printf("\n");
        }
        char message[1];
        if (answer == 'y') {
            message[0] = 'y';
        } else {
            message[0] = 'n';
        }

        // Sends some data to server
        int messageLen = strlen(message) + 1;
        
        int bytesSent = send(SenderSocket, message, messageLen, 0);

        if (bytesSent == -1) {
            printf("send() failed with error code : %d", errno);
        } else if (bytesSent == 0) {
            printf("peer has closed the TCP connection prior to send().\n");
        } else if (bytesSent < messageLen) {
            printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
        } else {
            printf("message was successfully sent.\n");
        }
        close(SenderSocket);
    }




    
    printf("Total sending time with Reno: %f seconds\n", totalTime / 1000000);
    printf("Average sending time with Reno: %f seconds\n", totalTime / 5000000);
}