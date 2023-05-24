/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}

void doit(int fd) // http 트랜잭션을 하나씩 처리하는 함수
{
  int is_static; // 정적인지 동적인지 알려주는 변수
  struct stat sbuf; 
  char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE]; // http명령어를 저장하는 변수
  char filename[MAXLINE],cgiargs[MAXLINE]; // 파일이름과, 동적컨텐츠에 사용되는 인자를 저장하는 변수
  rio_t rio; 

  /*Read request line and headers*/ 
  Rio_readinitb(&rio,fd); // 초기화
  Rio_readlineb(&rio,buf,MAXLINE); // 한줄읽어오기
  printf("Requestheaders:\n");
  printf("%s",buf); // 요청헤더 출력해보기
  sscanf(buf,"%s %s %s",method,uri,version); // 메쏘드, uri, 버젼을 버퍼에 저장 
  if(strcasecmp(method,"GET")){ // get메소드만 지원하게 만들예정이기 때문에, get메소드가 아니면
    clienterror(fd,method,"501","Not implemented", "Tiny does not implement this method"); 
    return; // 오류 출력
    } 
    read_requesthdrs(&rio); //헤더를 읽어오는 함수 
    /*Parse URI from GET request*/ 
    is_static=parse_uri(uri,filename,cgiargs); // 정/동적 컨텐츠를 확인하는 함수
    if(stat(filename,&sbuf)<0){ // 서버 버퍼에 버퍼가 없으면, 즉 해당 함수가 -1을 반환하면
      clienterror(fd,filename,"404","Notfound", "Tiny couldn’t find this file"); 
      return; // 404에러 출력
      }  
      if(is_static){/*Serve static content*/ // 정적이면
        if(!(S_ISREG(sbuf.st_mode))||!(S_IRUSR & sbuf.st_mode)){ // 해당 콘텐츠를 읽어올 권한이 없으면 
          clienterror(fd,filename,"403","Forbidden","Tiny couldn’t read the file"); 
          return; // 403에러 출력
        } 
        serve_static(fd,filename,sbuf.st_size); // 정적 콘텐츠를 처리하는 함수
        }else{/*Serve dynamic content*/ // 동적 콘텐츠이면
        if(!(S_ISREG(sbuf.st_mode))||!(S_IXUSR & sbuf.st_mode)){ //해당 콘텐츠를 실행할 권한이 없으면(왜냐하면 동적컨텐츠는 해당 콘텐츠로 새로운 컨텐츠를 만들어내는거기 떄문)
          clienterror(fd,filename,"403","Forbidden", "Tiny couldn’t run the CGI program"); 
          return; // 에러 메시지 출력
          } 
  serve_dynamic(fd,filename,cgiargs); // 동적 컨텐츠 처리하는 함수 
  }
}
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,char *longmsg) // 에러 메시지를 출력해주는 함수
{
  char buf[MAXLINE], body[MAXBUF]; 
  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n",body);
  sprintf(body, "%s%s: %s\r\n", body, errnum,shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));// 서버로 발생한 에러를 클라이언트로 보내는 함수
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n",(int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(buf));
}
void read_requesthdrs(rio_t *rp) { // 요청 헤더를 읽어오는 함수
  char buf[MAXLINE];
  // 첫 번째 요청 헤더를 읽어들입니다.
  Rio_readlineb(rp, buf, MAXLINE);
  // 요청 헤더를 모두 읽을 때까지 반복합니다.
  while(strcmp(buf, "\r\n")) {
    // 헤더를 한 줄씩 읽어들이고 출력합니다.
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}
int parse_uri(char *uri, char *filename, char *cgiargs) //동적인지 정적인지 알아내는 함수
{
  char *ptr; // 포인터 변수 선언
  if(!strstr(uri, "cgi-bin")){ // 정적 콘텐츠이면
    strcpy(cgiargs, "");
    strcpy(filename,".");
    strcat(filename, uri);
    if (uri[strlen(uri)-1]=='/') strcat(filename,"home.html");
    return 1;
  }// home.html을 띄워준다
  else{ // 동적 콘텐츠이면
    ptr = index(uri, '?');
    if(ptr){
      strcpy(cgiargs, ptr+1);
      *ptr = '\0';
    }else strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  } // chi-bin/adder(파일명)?123&456(인자)의 형식으로 보내서 adder실행시킴
}
void serve_static(int fd, char *filename, int filesize) // 정적콘텐츠를 처리하는 함수
{
  int srcfd;
  char *srcp,filetype[MAXLINE], buf[MAXBUF];
  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n",buf);
  sprintf(buf, "%sContent-length: %d\r\n",buf,filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n",buf,filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0); // 읽기모드로 열어서 srcfd에 저장
  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // 복사함수
  srcp = (char*) malloc(filesize);
  rio_readn(srcfd, srcp , filesize);
  Close(srcfd); // 파일을 닫는 함수
  Rio_writen(fd, srcp, filesize); // 복사한 파일의 정보를 클라이언트로 보냄
  // Munmap(srcp, filesize); // 매핑한 메모리를 해제
  free(srcp);
}
/* get_filetype-Derive filetype from filename */
void get_filetype(char *filename, char *filetype) // 파일의 타입을 찾는 함수
{
  if (strstr(filename, ".html"))
    strcpy(filetype,"text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jepg");
  else if (strstr(filename, ".mp4"))
    strcpy(filetype, "video/mp4");
  else  strcpy(filetype, "text/plain");
}
void serve_dynamic(int fd, char *filename, char *cgiargs) // 동적콘텐츠를 처리하는 함수
{
  char buf[MAXLINE],*emptylist[] = {NULL};
  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if(Fork() == 0){ //자식프로세스가 제대로 생성됐으면
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING",cgiargs, 1); // 자식프로세스에게 저장된 인자(나누지 않은 인자들)를 넘겨줌
    Dup2(fd, STDOUT_FILENO);              //클라이언트에게 출력가능하도록 방향을 바꾸는 함수
    Execve(filename, emptylist, environ); // 자식프로세스를 실행
  }
  Wait(NULL); /* Parent waits forand reaps child */
}