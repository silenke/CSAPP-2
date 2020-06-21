#include "csapp.h"

#define DEBUG

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";

void doit(int fd);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
void parse_url(char *url, char *hostname, char *port, char *path);
void forward_request(rio_t *rio, int fd, char *hostname, char *port, char *path);

void sigpipe_handler(int sig);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    // Check command line args
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    Signal(SIGPIPE, sigpipe_handler);

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int clientfd)
{
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], port[MAXLINE], path[MAXLINE];
    rio_t clientrio, serverrio;
    int serverfd;

    Rio_readinitb(&clientrio, clientfd);
    if (!Rio_readlineb(&clientrio, buf, MAXLINE))
        return;
#ifdef DEBUG
    printf("%s", buf);
#endif
    sscanf(buf, "%s %s %s", method, url, version);
    if (strcasecmp(method, "GET")) {
        clienterror(clientfd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }

    parse_url(url, hostname, port, path);
    serverfd = Open_clientfd(hostname, port);
    Rio_readinitb(&serverrio, serverfd);
    forward_request(&clientrio, serverfd, hostname, port, path);

    while (Rio_readlineb(&serverrio, buf, MAXLINE)) {
#ifdef DEBUG
        printf("%s", buf);
#endif
        Rio_writen(clientfd, buf, strlen(buf));
    }
    Close(serverfd);
}

void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];

    // Print the HTTP response headers
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    // Print the HTTP response body
    sprintf(buf, "<html><title>Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Proxy Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

void parse_url(char *url, char *hostname, char *port, char *path)
{
#ifdef DEBUG
    printf("<funtion: %s>\n", __FUNCTION__);
#endif
    char *p1, *p2, *p3;

    // ignore "http://"
    if ((p1 = strstr(url, "//")))
        p1 += 2;
    else
        p1 = url;

    // get path (default "/")
    if ((p3 = strstr(p1, "/"))) {
        strcpy(path, p3);
        *p3 = '\0';
    }
    else
        strcpy(path, "/");

    // get port (default "80")
    if ((p2 = strstr(p1, ":"))) {
        strcpy(port, p2 + 1);
        *p2 = '\0';
    }
    else
        strcpy(port, "80");

    // get hostname
    strcpy(hostname, p1);
}

void forward_request(rio_t *rio, int fd, char *hostname, char *port, char *path)
{
#ifdef DEBUG
    printf("<funtion: %s>\n", __FUNCTION__);
#endif
    char buf[MAXLINE];

    sprintf(buf, "GET %s HTTP/1.0\r\n", path);
    Rio_writen(fd, buf, sizeof(buf));
#ifdef DEBUG
    printf("%s", buf);
#endif

    while (Rio_readlineb(rio, buf, MAXLINE) && strcmp(buf, "\r\n")) {
        if (!strncasecmp(buf, "Host", 4) ||
                !strncasecmp(buf, "User-Agent", 10) ||
                !strncasecmp(buf, "Connection", 10) ||
                !strncasecmp(buf, "Proxy-Connection", 16))
            continue;
        Rio_writen(fd, buf, strlen(buf));
#ifdef DEBUG
        printf("%s", buf);
#endif
    }

    sprintf(buf, "Host: %s:%s\r\n", hostname, port);
    sprintf(buf, "%s%s%s%s\r\n", buf, user_agent_hdr, connection_hdr, proxy_connection_hdr);
#ifdef DEBUG
    printf("%s", buf);
#endif
    Rio_writen(fd, buf, strlen(buf));
}

void sigpipe_handler(int sig)
{
    printf("%s\n", __FUNCTION__);
    return;
}