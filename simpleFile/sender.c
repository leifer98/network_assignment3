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

#define SERVER_PORT 5065
#define SERVER_IP_ADDRESS "127.0.0.1"
#define FILE_SIZE 1048575
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

    printf("connected to server..\n");
restart:
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

    // reads first half of file
    FILE *file;
    file = fopen("0.txt", "r");
    int amountSent = 0, b;
    char data1[FILE_SIZE / 2];
    char data2[FILE_SIZE / 2];
    b = fread(data1, 1, sizeof(data1), file);
    if (b < 0)
    {
        printf("fread failed loading.");
        close(sock);
        return -1;
    }
    amountSent += b;
    fclose(file);
    int oldamountSent = amountSent;

    if (send(sock, data1, sizeof(data1), 0) == -1)
    {
        printf("ERROR! Sending has failed!\n");
        return -1;
    }
    // getting authentication and confirming it
    char bufferReply[BUFFER_SIZE] = {'\0'};
    int bytesReceived = recv(sock, bufferReply, BUFFER_SIZE, 0);
    if (bytesReceived == -1)
    {
        printf("recv() failed with error code.");
        return -1;
    }
    else if (bytesReceived == 0)
    {
        printf("peer has closed the TCP connection prior to recv().\n");
        goto end;
    }
    else
    {
        printf("received %d bytes from server. reply: %s\n", bytesReceived, bufferReply);
    }
    // check for authentication
    char * xor = "1740887";
    if (strcmp(xor, bufferReply) != 0)
    {
        goto end;
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

    // reads second half of file
    file = fopen("0.txt", "r");
    amountSent = 0;
    b = fread(data1, 1, sizeof(data1), file);
    if (b < 0)
    {
        printf("fread failed loading.");
        close(sock);
        return -1;
    }
    amountSent += b;
    fclose(file);
    printf("amount sent in half 2 is %d. \n", (amountSent));

    // USER DECISION
    char buffer[BUFFER_SIZE] = {0};
    printf("Enter \"stop\" to end session:\n");
    fgets(buffer, 2, stdin);
    // buffer[strlen(buffer) - 1] = '\0';
    int bytesSent = send(sock, buffer, 2, 0); // 4

    if (bytesSent == -1)
    {
        printf("send() failed with error code : %d", errno);
    }
    else if (bytesSent == 0)
    {
        printf("peer has closed the TCP connection prior to send().\n");
    }
    else if (bytesSent < strlen(buffer) + 1)
    {
        printf("sent only %d bytes from the required %d.\n", BUFFER_SIZE, bytesSent);
    }
    else
    {
        printf("sent msg succesfuly!: %s \n", buffer);
    }
    if (strncmp(buffer, "stop", 4) != 0)
    {
        bzero(buffer, BUFFER_SIZE);
        puts("****************************************************");
        goto restart;
    }
end:;
    close(sock);
    return 0;
}