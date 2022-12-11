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

    printf("connected to server\n");

    // Sends some data to server
    char message[] = "Hello, from the Client\n";
    int messageLen = strlen(message) + 1;

    int bytesSent = send(sock, message, messageLen, 0); // 4

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
        printf("message was successfully sent.\n");
    }

    // Sends some data to server
    char message0[] = "second message\n";
    messageLen = strlen(message0) + 1;

    bytesSent = send(sock, message0, messageLen, 0); // 4

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
        printf("message was successfully sent.\n");
    }

    // Receive data from server
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

    char message1[] = "another message sended\n";
    messageLen = strlen(message1) + 1;
    bytesSent = send(sock, message1, messageLen, 0); // 4

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
        printf("message was successfully sent.\n");
    }

    close(sock);

    // // reads text until newline is encountered
    // FILE *file;
    // file = fopen("mobydick2times.txt", "r");
    // char half[16000][300];
    // // char second_half[15000][300];
    // int i = 0, len = 0;
    // while (!feof(file))
    // {
    //     fgets(half[0], 300, file);
    //     len++;
    // }
    // fclose(file);

    // FILE *f;
    // f = fopen("mobydick2times.txt", "r");
    // i = 0;
    // if (len / 2 < 16000)
    // {
    //     while (i < len / 2)
    //     {
    //         fgets(half[i], 300, f);
    //         puts(half[i]);
    //         i++;
    //     }
    //     printf("count is: %d\n", len);

    //     while (!feof(file))
    //     {
    //         fgets(half[i], 300, f);
    //         puts(half[i]);
    //         i++;
    //     }
    //     printf("count is: %d\n", len);
    // }
    // else
    // {
    //     printf("this file is 2 big for this program...");
    // }

    // fclose(f);

    // printf("count is: %d\n", len);

    return 0;
}
