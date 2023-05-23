#include "../csapp.h"

int main(int argc, char **argv) // 인자 갯수, 인자 값임
{
    int clientfd; // 파일 디스크럽터
    char *host, *port, buf[MAXLINE]; //서버의 IP주소, 포트번호, 버퍼
    rio_t rio; // 견고한 입출력

    if(argc != 3){ // 실행파일, 호스트, 포트가 들어와야만 실행가능하기 때문
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]); // 실행파일 하나만 넣었을 때 뜨는 에러, host, port번호가 필요하다.
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port); // 서버에 접근할 클라이언트의 파일 디스크럽터를 저장
    Rio_readinitb(&rio, clientfd); // rio구조체 초기화

    while(Fgets(buf, MAXLINE, stdin) != NULL) { // 계속 실행시켜놔라
        Rio_writen(clientfd, buf, strlen(buf)); // 입력하는 값을 rio에 저장
        Rio_readlineb(&rio, buf, MAXLINE); // 저장한 한 줄을 읽는다(확실치 않음) 
        Fputs(buf, stdout); //출력시켜라
    }
    Close(clientfd); // 파일 디스크럽터를 닫는다.
    exit(0); // 연결 종료
}