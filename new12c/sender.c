/*
        TCP/IP client
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "127.0.0.1"
#define FILE_SIZE 1029120
#define BUFFER_SIZE 8192
#define FILE_NAME "1.txt"

void addLongToString(char *str, long num)
{
    char temp[20];
    sprintf(temp, "%ld", num);
    strcat(str, temp);
}

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
    {
        printf("Could not create socket : %d", errno);
        return -1;
    }
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
    if (rval <= 0)
    {
        printf("inet_pton() failed");
        return -1;
    }
    // Make a connection to the server with socket SendingSocket.
    int connectResult = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1)
    {
        printf("connect() failed with error code : %d", errno);
        // cleanup the socket;
        close(sock);
        return -1;
    }
    printf("connected to server\n");

    // Sending first half of the file data to server
    FILE *file;

    file = fopen(FILE_NAME, "r");
    int amountSent = 0, b, chunkIndex = 1;
    char data[BUFFER_SIZE] = {'\0'};
    while (((b = fread(data, 1, sizeof(data), file)) > 0) && (amountSent < (FILE_SIZE / 2)))
    {
    anotherTry:
        if (send(sock, data, sizeof(data), 0) == -1)
        {
            perror("ERROR! Sending has failed!\n");
            exit(1);
        }
        amountSent += b;
        bzero(data, BUFFER_SIZE);
        // Receive data from server
        char approval[BUFFER_SIZE] = "chunk approved, chunk number ";
        addLongToString(approval, chunkIndex);
        char bufferReply[BUFFER_SIZE] = {'\0'};
        int bytesReceived = recv(sock, bufferReply, BUFFER_SIZE, 0);
        if (bytesReceived == -1)
        {
            printf("recv() failed with error code : %d", errno);
        }
        else if (bytesReceived == 0)
        {
            printf("peer has closed the TCP connection prior to recv().\n");
        }
        else
        {
            if (strcmp(bufferReply, approval) != 0)
            {
                goto anotherTry;
            }
        }
        printf("sended chunk number %d, with %d bytes, and recieved approval for chunk number %d\n", chunkIndex, b, chunkIndex);
        chunkIndex++;
    }
    printf("finished sending first half amount sent is: %d \n", amountSent);
    // sending second half of the file

    close(sock);
    return 0;
}
