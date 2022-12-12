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

#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "127.0.0.1"

#define BUFFER_SIZE 1024

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
    char ch;
    int i = 0, j = 0, len = 0;
    while (!feof(file))
    {
        ch = fgetc(file);
        len++;
    }
    int parts = len / 1024;
    fclose(file);
    // 65536
    FILE *f;
    f = fopen("mobydick2times.txt", "r");
    printf("file seperated to %d parts\n", parts);

    char half[parts / 2][1024];

    while (!feof(f) && i < parts / 2)
    {
        while (j < 1023)
        {
            half[i][j] = fgetc(f);
            j++;
        }
        half[i][j] = '\0';
        // if (i  == 300) puts(half[i]);
        int messageLen = strlen(half[i]) + 1;

        int bytesSent = send(sock, half[i], messageLen, 0); // 4

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
            printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
        }
        else
        {
            // printf("sent %d part out of %d parts \n", i, parts);
        }
        j = 0;
        i++;
    }

    // Sends message to server thats the first half finished
    int messageLen = strlen("1") + 1;

    int bytesSent = send(sock, "1", messageLen, 0); // 4

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
        printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
    }
    else
    {
        printf("finished to send first half of the file with %d parts \n", parts);
    }

    // Receive data from server
    puts("waiting for server to send authuntication...");
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
        printf("received %d bytes from server: %s\n", bytesReceived, bufferReply);
    }
    int flag = 1;
    char xor [] = "1234";
    for (int i = 0; i < strlen(xor); i++)
    {
        if (xor[i] != bufferReply[i])
            flag = 0;
    }

    if (flag == 0)
        return 1;

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
    int oldi = i;
    i = 0;
    while (!feof(f) && i < parts / 2)
    {
        while (j < 1023)
        {
            half[i][j] = fgetc(f);
            j++;
        }
        half[i][j] = '\0';
        // if (i + oldi == 900) puts(half[i]);

        int messageLen = strlen(half[i]) + 1;

        int bytesSent = send(sock, half[i], messageLen, 0); // 4

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
            printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
        }
        else
        {
            // printf("sent %d part out of %d parts \n", (i+oldi), parts);
        }
        j = 0;
        i++;
    }
    fclose(f);

    // Sends message to server thats the first half finished
    messageLen = strlen("2") + 1;

    bytesSent = send(sock, "2", messageLen, 0); // 4

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
        printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
    }
    else
    {
        printf("finished to send second half of the file with %d parts \n", parts);
    }
    // puts(half[0]);

    // USER DECISION
    char buffer[BUFFER_SIZE];
    printf("Enter \"byebye\" to end session, enter anything else to restart process, \n ");
    fgets(buffer, BUFFER_SIZE, stdin);

    bytesSent = send(sock, buffer, BUFFER_SIZE, 0); // 4

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

    if (strncmp(buffer, "byebye", 4) != 0)
    {
        goto restart;
    }
    close(sock);
    return 0;
}
