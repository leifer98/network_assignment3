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

#define BUFFER_SIZE 1024

int main() {
    // READING FILE SETUP STAFF
    printf("check0\n");

    char *filename = "mobydick3times.txt";

    FILE *fp = fopen(filename, "r");

    printf("check1\n");
    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
        return 1;
    }
    printf("check2\n");
    // reading line by line, max 256 bytes
    const unsigned MAX_LENGTH = 256;
    char buffer1[MAX_LENGTH];
    char ch[1000][256];

    while (fgets(buffer1, MAX_LENGTH, fp)) {
        printf("%s", buffer1);
        for (int i = 0; i < 256; i++)
        {
            ch[0][i] = buffer1[i];
        }        
    }

    for (int i = 0; i < 256; i++)
    {
        printf("%c", ch[0][i]);
    }
    
    printf("\n");


    // // close the file
    fclose(fp);


    // NETWORK STAFF
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == -1) {
        printf("Could not create socket : %d", errno);
        return -1;
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);                                              // (5001 = 0x89 0x13) little endian => (0x13 0x89) network endian (big endian)
    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);  // convert IPv4 and IPv6 addresses from text to binary form
    // e.g. 127.0.0.1 => 0x7f000001 => 01111111.00000000.00000000.00000001 => 2130706433
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }

    // Make a connection to the server with socket SendingSocket.
    int connectResult = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1) {
        printf("connect() failed with error code : %d\n", errno);
        // cleanup the socket;
        close(sock);
        return -1;
    }

    printf("connected to server\n");

    // Sends some data to server
    char buffer[BUFFER_SIZE] = {'\0'};
    char message[] = "Hello, from the Client\n";
    int messageLen = strlen(message) + 1;
    
    int bytesSent = send(sock, message, messageLen, 0);

    if (bytesSent == -1) {
        printf("send() failed with error code : %d\n", errno);
    } else if (bytesSent == 0) {
        printf("peer has closed the TCP connection prior to send().\n");
    } else if (bytesSent < messageLen) {
        printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
    } else {
        printf("message was successfully sent.\n");
    }
    
    // Receive data from server
    char bufferReply[BUFFER_SIZE] = {'\0'};
    int bytesReceived = recv(sock, bufferReply, BUFFER_SIZE, 0);
    if (bytesReceived == -1) {
        printf("recv() failed with error code : %d\n", errno);
    } else if (bytesReceived == 0) {
        printf("peer has closed the TCP connection prior to recv().\n");
    } else {
        printf("received %d bytes from server: %s\n", bytesReceived, bufferReply);
    }
    
    close(sock);
    return 0;
}
