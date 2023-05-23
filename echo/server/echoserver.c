#include "../csapp.h"

void echo (int connfd);

int main(int argc, char **argv){ // 인자의 갯수와 인자의 값
    int listenfd, connfd; // 듣기식별자라고 해가지고 값이 들어오는 것을 기다리는 파일디스크럽터 변수
    socklen_t clientlen; // 소켓의 크기를 나타낸다
    struct sockaddr_storage clientaddr; // 클라이언트 소켓의 주소
    char client_hostname[MAXLINE], client_port[MAXLINE]; // 클라이언트 호스트와 포트

    if(argc != 2){ // 인자가 2가 아니라면 
        fprintf(stderr, "usage: %s <port>\n", argv[0]); // 서버는 이미 본인의 ip주소를 갖고 있기 때문에 port만 정해주면 된다
        exit(0);
    }
    
    listenfd = Open_listenfd(argv[1]); // 기다리는 상태입니다.

    while(1) { // 무한루프를 돌아버리겠다.
        clientlen = sizeof(struct sockaddr_storage); //소켓주소의 사이즈를 저장
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //서버가 연결을 허가해준다
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0); // 클라이언트 ip주소를 알아내는 함수
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        echo(connfd);

        Close(connfd);
    }
    exit(0);
}