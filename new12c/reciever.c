/*
    TCP/IP-server
*/

#include <stdio.h>

// Linux and other UNIXes
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 5060 // The port that the server listens
#define FILE_SIZE 1029120
#define BUFFER_SIZE 8192

void addLongToString(char *str, long num)
{
    char temp[20];
    sprintf(temp, "%ld", num);
    strcat(str, temp);
}

int main()
{

    int listeningSocket = -1;
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == -1)
    {
        printf("Could not create listening socket : %d", errno);
        return 1;
    }
    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0)
    {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1)
    {
        printf("Bind failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }

    int listenResult = listen(listeningSocket, 3);
    if (listenResult == -1)
    {
        printf("listen() failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }

    // Accept and incoming connection
    printf("Waiting for incoming TCP-connections...\n");
    struct sockaddr_in clientAddress; //
    socklen_t clientAddressLen = sizeof(clientAddress);

    while (1)
    {
        printf("waiting..\n");
        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
        {
            printf("listen failed with error code : %d", errno);
            // close the sockets
            close(listeningSocket);
            return -1;
        }

        printf("A new client connection accepted\n");

        // Receive first half of the file from client
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived, amountRec = 0, chunkIndex = 1;
        while (((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) && (amountRec < (FILE_SIZE / 2)))
        {
            amountRec += bytesReceived;
            bzero(buffer, BUFFER_SIZE);

            // Reply to client
            char approval[BUFFER_SIZE] = "chunk approved, chunk number ";
            addLongToString(approval, chunkIndex);
            int messageLen = strlen(approval) + 1;

            int bytesSent = send(clientSocket, approval, messageLen, 0);
            if (bytesSent == -1)
            {
                printf("send() failed with error code : %d", errno);
                close(listeningSocket);
                close(clientSocket);
                return -1;
            }
            else if (bytesSent == 0)
            {
                printf("peer has closed the TCP connection prior to send().\n");
            }
            else if (bytesSent < messageLen)
            {
                printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
            }
            printf("recieved chunk number %d, with %d bytes, and recieved approval for chunk number %d\n", chunkIndex, bytesReceived, chunkIndex);
            chunkIndex++;
        }

        printf("finished recieving first half amount recieved is: %d \n", amountRec);
    }

    close(listeningSocket);

    return 0;
}
