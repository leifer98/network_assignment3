#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


int main(){

    FILE *file;
    char *filename = "bigTXT.txt";

    //create a socket
    int SenderSocket;

    struct sockaddr_in measure_addr;
    measure_addr.sin_family = AF_INET;
    measure_addr.sin_port = htons(6789);
    measure_addr.sin_addr.s_addr = INADDR_ANY;

//    int con;
    float totalTime = 0;

    printf("Current Congestion Control -> Cubic\n");
    printf("---------------------------------------------------------------------------------------\n");
    //5 times in Cubic
    for(int i = 0 ; i <5 ; i++){
        SenderSocket = socket(AF_INET, SOCK_STREAM, 0);
        if(connect(SenderSocket, (struct  sockaddr*) &measure_addr, sizeof(measure_addr)) == -1){
            perror("ERROR! connection has failed!");
        }
        else{
            printf("You have successfully connected to the server\n");
        }

        file = fopen(filename, "r");
        if(!file){
            printf("ERROR! file opening has failed!\n");
        }
        clock_t start = clock();
        char data[1024];
        while ((fread(data, 1, sizeof data, file)) > 0){
            if(send(SenderSocket, data, sizeof(data), 0) == -1){
                perror("ERROR! Sending has failed!\n");
                exit(1);
            }
        }
        if (ferror(file)) {
            perror("ERROR! file opening has failed!");
        }
        clock_t end = clock();
        totalTime += (float)(end - start);
        close(SenderSocket);
        fclose(file);

    }
    printf("Total sending time with Cubic: %f seconds\n", totalTime / 1000000);
    printf("Average sending time with Cubic: %f seconds\n", totalTime / 5000000);


    printf("---------------------------------------------------------------------------------------\n");
    printf("Current Congestion Control -> Reno\n");
    printf("---------------------------------------------------------------------------------------\n");
    char CC[256];
    socklen_t socklen;
    for(int i = 0 ; i < 5 ; i++){
        SenderSocket = socket(AF_INET, SOCK_STREAM, 0);
        strcpy(CC, "reno");
        socklen = strlen(CC);
        if (setsockopt(SenderSocket, IPPROTO_TCP, TCP_CONGESTION, CC, socklen) != 0) {
            perror("ERROR! socket setting failed!");
            return -1;
        }
        if(connect(SenderSocket, (struct  sockaddr*) &measure_addr, sizeof(measure_addr)) == -1){
            perror("ERROR! connection has failed!");
        }
        else{
            printf("You have successfully connected to the server\n");
        }

        file = fopen(filename, "r");
        if(!file){

            perror("ERROR! file opening has failed!");
        }
        clock_t start = clock();
        char data[1024];
        while ((fread(data, 1, sizeof data, file)) > 0){
            if(send(SenderSocket, data, sizeof(data), 0) == -1){
                perror("ERROR! Sending has failed!\n");
                exit(1);
            }
        }
        if (ferror(file)) {
            perror("ERROR! file opening has failed!");
        }
        clock_t end = clock();
        totalTime += (float)(end - start);
        close(SenderSocket);
        fclose(file);

    }
    printf("Total sending time with Reno: %f seconds\n", totalTime / 1000000);
    printf("Average sending time with Reno: %f seconds\n", totalTime / 5000000);

}