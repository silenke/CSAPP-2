#include "csapp.h"
#include "cache.h"

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
void build_request(rio_t *rio, char *request, char *hostname, char *port, char *path);

void sigpipe_handler(int sig);

void *thread(void *vargp);

int main(int argc, char **argv)
{
    int listenfd, *connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    // Check command line args
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    Signal(SIGPIPE, sigpipe_handler);
    // cache
    init_cache();

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Malloc(sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        Pthread_create(&tid, NULL, thread, connfd);
    }
}

void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}

void doit(int clientfd)
{
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], port[MAXLINE], path[MAXLINE], request[MAXLINE];
    rio_t clientrio, serverrio;
    int serverfd, n;

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

    // cache
    if (read_cache(url, clientfd))
        return;

    serverfd = Open_clientfd(hostname, port);
    build_request(&clientrio, request, hostname, port, path);
    Rio_writen(serverfd, request, strlen(request));
#ifdef DEBUG
    printf("%s", request);
#endif

    // cache
    char data[MAX_OBJECT_SIZE];
    int len = 0;

    Rio_readinitb(&serverrio, serverfd);
    while ((n = Rio_readnb(&serverrio, buf, MAXLINE))) {
#ifdef DEBUG
        printf("Received %d bytes\n", n);
#endif
        Rio_writen(clientfd, buf, n);

        // cache
        if (len + n <= MAX_OBJECT_SIZE) {
            memcpy(data + len, buf, n);
            len += n;
        }
    }
    // cache
    if (len <= MAX_OBJECT_SIZE)
        write_cache(url, data, len);

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
    char *p1, *p2, *p3;
    char urlcpy[MAXLINE];

    strcpy(urlcpy, url);
    // ignore "http://"
    if ((p1 = strstr(urlcpy, "//")))
        p1 += 2;
    else
        p1 = urlcpy;

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

void build_request(rio_t *rio, char *request, char *hostname, char *port, char *path)
{
    char buf[MAXLINE];

    sprintf(request, "GET %s HTTP/1.0\r\n", path);
    
    while (Rio_readlineb(rio, buf, MAXLINE) && strcmp(buf, "\r\n")) {
        if (!strncasecmp(buf, "Host", 4) ||
                !strncasecmp(buf, "User-Agent", 10) ||
                !strncasecmp(buf, "Connection", 10) ||
                !strncasecmp(buf, "Proxy-Connection", 16))
            continue;
        sprintf(request, "%s%s", request, buf);
    }

    sprintf(request, "%sHost: %s:%s\r\n", request, hostname, port);
    sprintf(request, "%s%s%s%s\r\n", request, user_agent_hdr, connection_hdr, proxy_connection_hdr);
}

void sigpipe_handler(int sig)
{
    printf("%s\n", __FUNCTION__);
    return;
}