// Adia Foster, Isabelle Cochran, and Lindsey Rafalsky

#include <sys/types.h>  /* basic system data types */
#include <sys/socket.h> /* basic socket defintions */
#include <netinet/in.h> /* sockaddr_in{} and other Internet defns */

#include <sys/time.h> /* timeval{} for select() */
#include <time/h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

void datagramProcessing(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen) {
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];
    socklen_t len;
    struct sockaddr *preply_addr;

    preply_addr = Malloc(servlen);

    while (fgets(sendline, MAXLINE, fp)!= NULL) {
        sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

        len = servlen;
        n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);
        if (len != servlen || memcmp(pservaddr, preply_addr, len) != 0) {
            printf("reply from %s (ignored)\n", sock_ntop(preply_addr,len));
            continue;
        }
        recvline[n] = 0; /*null terminate*/
        fputs(recvline, stdout);
    }
}

int main(int argc, char **argv) {
    int sd;
    struct sockaddr_in server;
    struct hostent *hp;
    
    sd = socket(AF_INET,SOCK_DGRAM,0)

    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    hp = gethostbyname(argv[1]);
    bcopy(hp->h_addr, &(server.sin_addr), hp->h_length);

    for (;;) {
        sendto(sd, "HI", 2, 0, (struct sockaddr *) &server, sizeof(server));
        sleep(2);
    }
    close(sd);
}