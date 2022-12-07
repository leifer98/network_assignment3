#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_PORT 5060
#define SIZE 65536

static time_t total = 0;
static double totalt = 0;
static int count = 0;

int main(int argc, char const *argv[])
{
    // --------------------------1------------------------------
    // Open the listening socket(server) + test

    // Creating socket
    printf("create socket\n");
    int serverSocket = -1;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Could not create server socket : %d", errno);
        return -1;
    }

    // Reuse the address and the port, if the server socket on was closed
    // and remains for 45 seconds in TIME-WAIT state till the final removal.
    // Prevents error such as: “address already in use”.
    int enableReuse = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enableReuse, sizeof(enableReuse)) < 0)
    {
        printf("setsockopt() failed with error code : %d", errno);
        return -1;
    }

    // "sockaddr_in" is the "derived" from sockaddr structure used for IPv4
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress)); // reseting all memory in serveradress to 0
    serverAddress.sin_family = AF_INET;               // setting for ipv4
    serverAddress.sin_addr.s_addr = INADDR_ANY;       // any IP at this port (Address to accept any incoming messages)
    serverAddress.sin_port = htons(SERVER_PORT);      // network order (makes byte order consistent)

    // Bind the socket to the port with any IP at this port
    printf("bind\n");
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        printf("Bind failed with error code : %d", errno);
        close(serverSocket);
        return -1;
    }
    printf("Bind() success\n");

    // Make the socket listening;
    // It puts the server socket in a passive mode, where it waits for the client to approach the server to make a connection.
    if (listen(serverSocket, 5) < 0) // 5 is a maximum length to which the queue of pending connections for serverSocket may grow
    {                                // number of concurrent connections
        printf("listen() failed with error code : %d", errno);
        close(serverSocket);
        return -1;
    }

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    int clientSocket = -1;
    socklen_t len;
    int bytesReceived = 0;
    int i = 0;
    char buffer[SIZE] = {0};
    char ccBuffer[256];
    len = sizeof(ccBuffer);
    if (getsockopt(serverSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &len) != 0)
    {
        perror("getsockopt");
        return -1;
    }
    printf("Current CC: %s\n", ccBuffer);
    while (1)
    {
        count = 0;
        total = 0;
        totalt = 0;
        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        while (i < 5)
        {
            printf("0Current i : %d\n", i);

            if (clientSocket == -1)
            {
                printf("accept failed with error code : %d", errno);
                close(serverSocket);
                return -1;
            }
            // printf("A new client connection accepted\n");

            clock_t begin = clock();
            time_t t = time(0);
            // printf("%f\n",(double)clock());
            int tot = 0;
            while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
            {
                tot += bytesReceived;
            }
            // bytesReceived=0;
            // printf("%ld",clock());
            time_t e = time(0);
            clock_t end = clock();

            double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            count++;
            total += e - t;
            totalt += time_spent;
            printf("1Current i : %d\n", i);

            printf("Received byte: %d in %f seconds \t", tot, time_spent);
            printf("(real time in about %ld seconds) \n", e - t);
            if (bytesReceived < 0)
                perror("Receiving");
            i++;
            printf("2Current i : %d\n", i);
        }
        printf("total avarage time =  %f\n", totalt / count);
        printf("total general time = about %ld seconds\n", total);

        // sending authentication
        int a = 207083353;
        int b = 206391054;
        // int c = ("%d", a ^ b);
        char *message = "abcd";
        int messageLen = strlen(message) + 1;
        int bytesSent = send(clientSocket, message, messageLen, 0);
        if (bytesSent == -1)
        {
            printf("send() failed with error code : %d", errno);
            close(serverSocket);
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
        else
        {
            printf("message was successfully sent.\n");
        }

        // change CC algorithm to reno
        printf("Changing CC algorithm\n");
        strcpy(buffer, "reno");
        len = sizeof(buffer);
        if (setsockopt(serverSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, len) != 0)
        {
            perror("setsockopt");
            return -1;
        }
        if (getsockopt(serverSocket, IPPROTO_TCP, TCP_CONGESTION, buffer, &len) != 0)
        {
            perror("getsockopt");
            return -1;
        }
        printf("New CC: %s\n", buffer);

        bytesReceived = 0;
        total = 0;
        totalt = 0;
        count = 0;
        i = 0;
        while (i < 5)
        {
            if (clientSocket == -1)
            {
                printf("accept failed with error code : %d", errno);
                close(clientSocket);
                return -1;
            }

            // printf("%ld",clock());
            clock_t begin = clock();
            time_t start = time(0);
            int tot = 0;
            while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
            {
                tot += bytesReceived;
            }
            // bytesReceived=0;
            // printf("%ld",clock());
            clock_t end = clock();
            double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            count++;
            time_t endLocal = time(0);
            total += endLocal - start;
            totalt += time_spent;
            printf("Received byte: %d in %f seconds \t", tot, time_spent);
            printf("(real time in about %ld seconds) \n", endLocal - start);

            if (bytesReceived < 0)
                perror("can't recive file!");

            i++;
        }
        close(clientSocket);

        printf("total new avarage time =  %f\n", totalt / count);
        printf("total new general time = about %ld seconds\n", total);
    }
    return 0;
}