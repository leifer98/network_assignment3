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
#include <sys/time.h>

#define SERVER_PORT 5062 // The port that the server listens
#define BUFFER_SIZE 8192
#define FILE_SIZE 1048576
void addLongToString(char *str, long num)
{
    char temp[20];
    sprintf(temp, "%ld", num);
    strcat(str, temp);
}
int main()
{
    // Time setup:
    struct timeval start, end;
    long tot = 0;
    // Open the listening (server) socket
    int listeningSocket = -1;
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 0 means default protocol for stream sockets (Equivalently, IPPROTO_TCP)
    if (listeningSocket == -1)
    {
        printf("Could not create listening socket : %d", errno);
        return 1;
    }

    // Reuse the address if the server socket on was closed
    // and remains for 45 seconds in TIME-WAIT state till the final removal.
    //
    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0)
    {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;  // any IP at this port (Address to accept any incoming messages)
    serverAddress.sin_port = htons(SERVER_PORT); // network order (makes byte order consistent)

    // Bind the socket to the port with any IP at this port
    int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1)
    {
        printf("Bind failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }

    printf("Bind() success\n");

    // Make the socket listening; actually mother of all client sockets.
    // 500 is a Maximum size of queue connection requests
    // number of concurrent connections
    int listenResult = listen(listeningSocket, 3);
    if (listenResult == -1)
    {
        printf("listen() failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }

    // Accept and incoming connection
    struct sockaddr_in clientAddress; //
    socklen_t clientAddressLen = sizeof(clientAddress);

    while (1)
    {
        printf("Waiting for incoming TCP-connections...\n");

        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
        {
            printf("listen failed with error code : %d\n", errno);
            // close the sockets
            close(listeningSocket);
            return -1;
        }
        printf("A new client connection accepted\n");
        // time calculations preperation:
        char time_text[100000] = "";
        long totClientTime_1 = 0;
        long totClientTime_2 = 0;
        int countFileSent = 1;

    restart:
        printf("This is round number %d to send file: ", countFileSent);

        // code got changing CC algorithm
        char ccBuffer[256];
        printf("Changed Congestion Control to Cubic\n");
        strcpy(ccBuffer, "cubic");
        socklen_t socklen = strlen(ccBuffer);

        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
        {
            perror("ERROR! socket setting failed!");
            return -1;
        }
        if (getsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
        {
            perror("ERROR! socket getting failed!");
            return -1;
        }

        // time capturing handling
        gettimeofday(&start, NULL);
        // Receive a message from client
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        int bytesReceived, amountRec = 0;
        while ((amountRec <= (FILE_SIZE / 2)) && (bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0)
        {
            printf("%d \n", amountRec);
            amountRec += bytesReceived;
            bzero(buffer, BUFFER_SIZE);
        }
        printf("recieves in total for first half: %d bytes \n", amountRec);
        // time capturing handling
        gettimeofday(&end, NULL);
        tot = ((end.tv_sec * 1000000 + end.tv_usec) -
               (start.tv_sec * 1000000 + start.tv_usec));
        totClientTime_1 += tot;
        // printf("time taken in micro seconds: %ld \n", tot);
        char temp_str[50] = "time taken in micro seconds for first half: ";
        addLongToString(temp_str, tot);
        strcat(temp_str, "\n");
        strcat(time_text, temp_str);
        // Reply to client
        puts("sending authuntication to client...\n");

        char *message = "1740887"; // = 207083353 XOR 206391054
        int messageLen = strlen(message) + 1;

        int bytesSent = send(clientSocket, message, messageLen, 0);
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

        // Changing to reno algorithm
        printf("Changed Congestion Control to Reno\n");
        strcpy(ccBuffer, "reno");
        socklen = strlen(ccBuffer);
        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
        {
            perror("ERROR! socket setting failed!");
            return -1;
        }
        socklen = sizeof(ccBuffer);
        if (getsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
        {
            perror("ERROR! socket getting failed!");
            return -1;
        }
        // time capturing handling
        gettimeofday(&start, NULL);

        // waiting to recieve second part.
        /// problem here: !!!!!!!!!!!!!!!!!!!!!!!
        memset(buffer, 0, BUFFER_SIZE);
        while ((amountRec <= (FILE_SIZE)) && (bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0)
        {
            printf("%d \n", amountRec);
            amountRec += bytesReceived;
            bzero(buffer, BUFFER_SIZE);
        }

        // time capturing handling
        gettimeofday(&end, NULL);
        tot = ((end.tv_sec * 1000000 + end.tv_usec) -
               (start.tv_sec * 1000000 + start.tv_usec));
        totClientTime_2 += tot;

        // printf("time taken in micro seconds for second half: %ld \n", tot);
        char temp_str1[50] = "time taken in micro seconds for second half: ";
        addLongToString(temp_str1, tot);
        strcat(temp_str1, "\n");
        strcat(time_text, temp_str1);

        // USER DECISION
        puts("waiting for user decision...");
        memset(buffer, 0, BUFFER_SIZE);
        if ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0)
        {
            printf("Received %d bytes from client. decision is %s\n", bytesReceived, buffer);

            // check if got the "byebye" command from client, if yes, then exit and close the client socket
            if (strncmp(buffer, "byebye", 4) == 0)
            {
                printf("Client has decided to end the session. Stats:\n");
                puts(time_text);
                printf("Total of time for first half is: %ld for %d times for an avarage of %ld. \n", totClientTime_1, countFileSent, totClientTime_1 / countFileSent);
                printf("Total of time for second half is: %ld for %d times for an avarage of %ld. \n", totClientTime_2, countFileSent, totClientTime_2 / countFileSent);
            }
            else
            {
                countFileSent++;
                goto restart;
            }
        }
    }
    close(listeningSocket);
    return 0;
}