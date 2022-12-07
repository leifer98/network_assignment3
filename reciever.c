#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>

typedef struct timeval time;

// calculate the amount of time it takes to get the packets.
double getAmountOfTime(time starting_time, time ending_time)
{
    double total_time = ((ending_time.tv_sec * 1000000 + ending_time.tv_usec) -
                         (starting_time.tv_sec * 1000000 + starting_time.tv_usec));
    return total_time;
}
#define PORT 5093
#define SIZE 65536

int main()
{

    time starting_time, ending_time;
    int MeasureSocket, SenderSocket, errCheck;
    double totalTime = 0;

    // create a socket for measurement.
    MeasureSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (MeasureSocket < 0)
    {
        printf("ERROR! Socket creation failed\n");
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY; // anyone can connect.

    errCheck = bind(MeasureSocket, (struct sockaddr *)&address, sizeof(address));
    if (errCheck < 0)
    {
        printf("ERROR! Binding address to socket failed.\n");
        return -1;
    }
    while (1)
    {
        char ccBuffer[256];
        // CUBIC
        printf("Current Congestion Control -> Cubic\n");
        printf("---------------------------------------------------------------------------------------\n");
        for (int i = 0; i < 5; i++)
        {
            if (listen(MeasureSocket, 1) == -1)
            {
                printf("ERROR! set listening failed.\n");
            }
            SenderSocket = accept(MeasureSocket, NULL, NULL);
            if (SenderSocket == -1)
            {
                perror("ERROR! accepting client failed.\n");
            }

            int amountReceived = 0;
            gettimeofday(&starting_time, NULL); // start counting
            while ((errCheck = recv(SenderSocket, &ccBuffer, sizeof(ccBuffer), 0) > 0))
            {
                if (errCheck < 0)
                {
                    perror("Error: ");
                }
                amountReceived += errCheck;
            }
            gettimeofday(&ending_time, NULL); // stop counting
            // printf("%d\n", KbytesRec);
            double current_time = getAmountOfTime(starting_time, ending_time);
            // printf("Message recieved: %s \n", buffer);
            totalTime += current_time;
            memset(&ccBuffer, 0, sizeof(ccBuffer));
            if (i != 4)
            {
                close(SenderSocket);
            }
        }

        // printf("Total receiving time with Cubic: %f seconds\n", totalTime / 1000000);
        // printf("Average receiving time with Cubic: %f seconds\n", totalTime / 5000000);
        printf("---------------------------------------------------------------------------------------\n");
        // int a = 207083353;
        // int b = 206391054;
        // int c = a ^ b;
        //10 == msg length
        char *message = "abcd";
        int messageLen = strlen(message) + 1;
        int bytesSent = send(SenderSocket, message, messageLen, 0);
        close(SenderSocket);

        if (bytesSent == -1)
        {
            printf("send() failed with error code : %d", -1);
            close(SenderSocket);
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

        // RENO
        strcpy(ccBuffer, "reno");
        socklen_t socklen = strlen(ccBuffer);

        if (setsockopt(MeasureSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
        {
            perror("ERROR! socket setting failed!");
            return -1;
        }

        socklen = sizeof(ccBuffer);
        if (getsockopt(MeasureSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
        {
            perror("ERROR! socket getting failed!");
            return -1;
        }

        printf("Current Congestion Control -> Reno\n");
        printf("---------------------------------------------------------------------------------------\n");
        for (int i = 0; i < 5; i++)
        {
            errCheck = listen(MeasureSocket, 10);
            if (errCheck < 0)
            {
                printf("ERROR! set listening failed.\n");
            }

            SenderSocket = accept(MeasureSocket, NULL, NULL);
            if (SenderSocket < 0)
            {
                perror("ERROR! accepting client failed.\n");
            }

            char receiverByReno[1024];
            int amountReceived = 0;
            gettimeofday(&starting_time, NULL);
            while ((errCheck = recv(SenderSocket, &receiverByReno, sizeof(receiverByReno), 0) > 0))
            {

                if (errCheck < 0)
                {
                    perror("Error: ");
                }
                amountReceived += errCheck;
            }
            gettimeofday(&ending_time, NULL);

            // printf("%d\n", KbytesRec);

            double current_time = getAmountOfTime(starting_time, ending_time);
            // printf("Message recieved: %s \n", buffer);
            totalTime += current_time;
            bzero(receiverByReno, 1024);
            if (i != 4)
            {
                close(SenderSocket);
            }
        }

        // Receive data from server
        char bufferReply[SIZE] = {'\0'};
        int bytesReceived = recv(SenderSocket, bufferReply, SIZE, 0);
        if (bytesReceived == -1)
        {
            printf("recv() failed with error code : %d", -1);
        }
        else if (bytesReceived == 0)
        {
            printf("peer has closed the TCP connection prior to recv().\n");
        }
        else
        {
            if (bufferReply == 'y')
            {
                printf("received %d bytes from server: %s\n", bytesReceived, bufferReply);
            }
            else
            {
                close(SenderSocket);
                goto exit_loop;
            }
        }
        close(SenderSocket);
    }
exit_loop:;

    // printf("Total receiving time with Reno: %f seconds\n", totalTime / 1000000);
    // printf("Average receiving time with Reno: %f seconds\n", totalTime / 5000000);

    close(MeasureSocket);
    return 0;
}