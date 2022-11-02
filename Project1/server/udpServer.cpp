// Adia Foster, Isabelle Cochran, and Lindsey Rafalsky

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#define PACKET_SIZE 512
#define PORT 8080 //change from 8080, find ours

using namespace std;


int sockfd; /**/
struct sockaddr_in clientaddr, serveraddr; 


int init() {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    memset(&clientaddr, 0, sizeof(clientaddr));

    //inputting server information
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);

    //bind socket to the server address
    if (::bind(sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        perror("Socket binding failed");
        exit(EXIT_FAILURE);
    }

    return 0;

}

int checksumCalculate(char buffer[]) {
    int checksum = 0;
    for(int i = 7; i < PACKET_SIZE; i++) {
        checksum += buffer[i];
    }
    return checksum
}

bool checksumValidate(char buffer[]) {
    try {
        int checksumCalculation = checksumCalculate(buffer);

        string checksumString;
        for(int i = 2; i < 7; i++) {
            checksumString += buffer[i];
        }

        int checksumPassed = stoi(checksumString);

        return checksumCalculation == checksumPassed;
    }

    catch (std::invalid_argument) {
        return false;
    }
}

int fileWriter(ofstream &file, char buffer[]) {
    for (int i = 7; i < PACKET_SIZE; i++) {
        if (buffer[i] != '\0')
        file << buffer[i];
    }

    return 0;
}

int sendResponse(char response[], socklen_t socketLen) {
    sendto(sockfd, (const char *)response, strlen(response), 0, (const struct sockaddr *)&clientaddr)
}

int main() {
    int n, sd; /* sd is an integer socket descriptor */
    struct sockaddr_in server;
    char buf[512]; /* contains the memory address to store the data */

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(12345);

    sd = sock(AF_INET,SOCK_DGRAM,0);
    bind(sd, (struct sockaddr *)&server, sizeof(server));

    for (;;) {
        n = recv(sd, buf, sizeof(buf), 0);
        buf[n] = '\0';
        printf("Received: %s\n", buf);
    }

    close(sd);
    return 0;
}