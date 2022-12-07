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
double getAmountOfTime(time starting_time, time ending_time){
    double total_time =((ending_time.tv_sec * 1000000 + ending_time.tv_usec) -
                       (starting_time.tv_sec * 1000000 + starting_time.tv_usec)) ;
    return total_time;
}


int main(){

    time starting_time, ending_time;
    int MeasureSocket, SenderSocket, checkERROR;
    double totalTime = 0;

    //create a socket for measurement.
    MeasureSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (MeasureSocket < 0){
        printf("ERROR! Socket creation failed\n");
        exit(1);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(6789);
    address.sin_addr.s_addr = INADDR_ANY; // anyone can connect.


    checkERROR = bind(MeasureSocket, (struct sockaddr*) &address, sizeof(address) );
    if (checkERROR < 0){
        printf("ERROR! Binding address to socket failed.\n");
        exit(1);
    }



    // CUBIC
    printf("Current Congestion Control -> Cubic\n");
    printf("---------------------------------------------------------------------------------------\n");
    for (int i = 0 ; i < 5 ; i++){
        if (listen(MeasureSocket, 1) == -1){
            printf("ERROR! set listening failed.\n");
        }
        SenderSocket = accept(MeasureSocket, NULL , NULL);
        if (SenderSocket == -1){
            perror("ERROR! accepting client failed.\n");
        }

        char receiverByCubic[1024];
        int amountReceived = 0 ;
        gettimeofday(&starting_time , NULL); // start counting
        while ((checkERROR = recv(SenderSocket, &receiverByCubic , sizeof(receiverByCubic) ,0) > 0)){
            if (checkERROR < 0){
                perror("Error: ");
            }
            amountReceived += checkERROR;
        }
        gettimeofday(&ending_time , NULL); // stop counting
        //printf("%d\n", KbytesRec);
        double current_time = getAmountOfTime(starting_time, ending_time);
        // printf("Message recieved: %s \n", buffer);
        totalTime += current_time;
        bzero(receiverByCubic, 1024);
        close(SenderSocket);
    }

    printf("Total receiving time with Cubic: %f seconds\n", totalTime / 1000000);
    printf("Average receiving time with Cubic: %f seconds\n", totalTime / 5000000);
    printf("---------------------------------------------------------------------------------------\n");



    // RENO
    char CC[256];
    strcpy(CC, "reno");
    socklen_t socklen = strlen(CC);

    if (setsockopt(MeasureSocket, IPPROTO_TCP, TCP_CONGESTION, CC, socklen) != 0) {
        perror("ERROR! socket setting failed!");
        return -1;
    }

    socklen = sizeof(CC);
    if (getsockopt(MeasureSocket, IPPROTO_TCP, TCP_CONGESTION, CC, &socklen) != 0) {
        perror("ERROR! socket getting failed!");
        return -1;
    }

    printf("Current Congestion Control -> Reno\n");
    printf("---------------------------------------------------------------------------------------\n");
    for(int i = 0 ; i < 5 ; i++){
        checkERROR = listen(MeasureSocket, 10);
        if (checkERROR < 0){
            printf("ERROR! set listening failed.\n");
        }

        SenderSocket = accept(MeasureSocket, NULL , NULL);
        if (SenderSocket < 0){
            perror("ERROR! accepting client failed.\n");
        }

        char receiverByReno[1024];
        int amountReceived = 0 ;
        gettimeofday(&starting_time , NULL);
        while((checkERROR = recv(SenderSocket, &receiverByReno , sizeof(receiverByReno) ,0) > 0)){

            if( checkERROR < 0){
                perror("Error: ");
            }
            amountReceived += checkERROR;
        }
        gettimeofday(&ending_time , NULL);

        //printf("%d\n", KbytesRec);

        double current_time = getAmountOfTime(starting_time, ending_time);
        // printf("Message recieved: %s \n", buffer);
        totalTime += current_time;
        bzero(receiverByReno, 1024);
        close(SenderSocket);
    }

    printf("Total receiving time with Reno: %f seconds\n", totalTime / 1000000);
    printf("Average receiving time with Reno: %f seconds\n", totalTime / 5000000);

    close(MeasureSocket);
    return 0;
}