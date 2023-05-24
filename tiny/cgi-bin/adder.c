/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
  char *buf, *p;
  char arg1[MAXLINE],arg2[MAXLINE],content[MAXLINE];
  int n1 = 0, n2 = 0;
  /*Extractthetwoarguments*/
  if ((buf = getenv("QUERY_STRING")) != NULL){ // 버퍼에다가 인자들을 가져옴
    p = strchr(buf, '&'); // 인자는 &로 나눠져다~~
    *p = '\0'; 
    strcpy(arg1, buf); // arg1에 인자1 저장
    strcpy(arg2, p+1); // arg2에 인자2 저장(이래서 포인터 변수를 썻다~~)
    n1 = atoi(arg1); // int형으로 바꿔주는 함수
    n2 = atoi(arg2);
  }

  /* Make the response body */
  // server 실행 후 진행
  // sprintf (body,"%s",a); -> body에 a값을 넣겠다
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>",content);
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>",content, n1, n2, n1+n2);
  sprintf(content, "%sThanks for visiting!\r\n",content);
  //응답 본체만들기

  /* Generate the HTTP response */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n",(int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  //응답 헤더만들기

  printf("%s",content);
  fflush(stdout); // buffer에 넣어둔 값 초기화

  exit(0);
}
/* $end adder */