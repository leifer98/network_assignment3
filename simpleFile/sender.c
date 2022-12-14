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
#include <netinet/tcp.h>
#include <time.h>

#define SERVER_PORT 5062
#define SERVER_IP_ADDRESS "127.0.0.1"
#define FILE_SIZE 1261568
#define BUFFER_SIZE 8192

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 1

    if (sock == -1)
    {
        printf("Could not create socket : %d", errno);
        return -1;
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);                                             // (5001 = 0x89 0x13) little endian => (0x13 0x89) network endian (big endian)
    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr); // convert IPv4 and IPv6 addresses from text to binary form
    // e.g. 127.0.0.1 => 0x7f000001 => 01111111.00000000.00000000.00000001 => 2130706433
    if (rval <= 0)
    {
        printf("inet_pton() failed");
        return -1;
    } // 2

    // Make a connection to the server with socket SendingSocket.
    int connectResult = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1)
    {
        printf("connect() failed with error code : %d", errno);
        // cleanup the socket;
        close(sock);
        return -1;
    } // 3
restart:

    printf("connected to server\n");
    // change to cubic CC algorithm
    char ccBuffer[256];
    printf("Changed Congestion Control to Cubic\n");
    strcpy(ccBuffer, "cubic");
    socklen_t socklen = strlen(ccBuffer);
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
    {
        perror("ERROR! socket setting failed!");
        return -1;
    }
    socklen = sizeof(ccBuffer);
    if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
    {
        perror("ERROR! socket getting failed!");
        return -1;
    }

    // reads text until newline is encountered
    FILE *file;
    file = fopen("mobydick2times.txt", "r");
    int i = 0, len = 0;
    int amountSent = 0, b;
    char data[BUFFER_SIZE];
    while (((b = fread(data, 1, sizeof data, file)) > 0) && (amountSent <= FILE_SIZE / 2))
    {
        if (send(sock, data, sizeof(data), 0) == -1)
        {
            perror("ERROR! Sending has failed!\n");
            exit(1);
        }
        printf("sent data: %d \n", b);
        amountSent += b;
        bzero(data, BUFFER_SIZE);
    }
    printf("amount sent in half1 is %d. \n", amountSent);
    // Sends message to server thats the first half finished

    // Receive data from server
    puts("waiting for server to send authuntication...");
    char bufferReply[BUFFER_SIZE] = {'\0'};
    int bytesReceived = recv(sock, bufferReply, BUFFER_SIZE, 0);
    if (bytesReceived == -1)
    {
        printf("recv() failed with error code.");
    }
    else if (bytesReceived == 0)
    {
        printf("peer has closed the TCP connection prior to recv().\n");
    }
    else
    {
        printf("received %d bytes from server: %s\n", bytesReceived, bufferReply);
    }
    int flag = 1;
    char xor [] = "1740887";
    for (int i = 0; i < strlen(xor); i++)
    {
        if (xor[i] != bufferReply[i])
            flag = 0;
    }
    if (flag == 0)
    {
        printf("wrong authntication!");
        close(sock);
        return 1;
    }
    // code got changing CC algorithm
    // Changing to reno algorithm
    printf("Changed Congestion Control to Reno\n");
    strcpy(ccBuffer, "reno");
    socklen = strlen(ccBuffer);
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
    {
        perror("ERROR! socket setting failed!");
        return -1;
    }
    socklen = sizeof(ccBuffer);
    if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
    {
        perror("ERROR! socket getting failed!");
        return -1;
    }

    // second half
    int oldamountSent =amountSent, g;
    bzero(data, BUFFER_SIZE);
    while (((b = fread(data, 1, BUFFER_SIZE, file)) > 0) && (amountSent <= FILE_SIZE))
    {
        if ((g = send(sock, data, BUFFER_SIZE, 0)) == -1)
        {
            perror("ERROR! Sending has failed!\n");
            exit(1);
        }
        printf("b: %d, g: %d \n", b, g);
        amountSent += b;
        bzero(data, BUFFER_SIZE);
    }
    printf("amount sent in half2 is %d. \n", amountSent - oldamountSent);
    fclose(file);

    // USER DECISION
    char buffer[BUFFER_SIZE];
    printf("Enter \"byebye\" to end session, enter anything else to restart process: \n");
    fgets(buffer, BUFFER_SIZE, stdin);
    int messageLen = strlen(buffer);
    int bytesSent = send(sock, buffer, BUFFER_SIZE, 0); // 4

    if (bytesSent == -1)
    {
        printf("send() failed with error code : %d", errno);
    }
    else if (bytesSent == 0)
    {
        printf("peer has closed the TCP connection prior to send().\n");
    }
    else if (bytesSent < messageLen)
    {
        printf("sent only %d bytes from the required %d.\n", BUFFER_SIZE, bytesSent);
    }
    else
    {
        printf("sent msg succesfuly!");
    }
    if (strncmp(buffer, "byebye", 4) != 0)
    {
        goto restart;
    }
    close(sock);
    return 0;
}