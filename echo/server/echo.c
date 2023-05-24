#include "../csapp.h"

void echo (int connfd) // 클라이언트에 연결된 파일 디스크럽터
{
    size_t n; //사이즈
    char buf[MAXLINE]; // 버퍼
    rio_t rio; // 견고한 입출력

    Rio_readinitb(&rio, connfd); // 초기화
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){ //readline이 존재하면
        printf("server received %d bytes\n", (int)n); // 서버가 n바이트만큼 받았습니다.
        printf("%s", buf);
        Rio_writen(connfd, buf, n); //버퍼에 있는 걸 클라이언트에 전송
    }
}