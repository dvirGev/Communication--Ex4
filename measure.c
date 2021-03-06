/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#define PORT "3490"      // the port users will be connecting to
#define MAXDATASIZE 1024 // max number of bytes we can get at once
#define BACKLOG 10       // how many pending connections queue will hold
#define BILLION 1000000000.0
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}
double getFile(int new_fd)
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    printf("new client connect %d\n", new_fd);

    // FILE *fptr = fopen("output.txt", "w");
    // if (fptr == NULL)
    // {
    //     perror("fptr == NULL");
    // }
    char *buffer = (char *)malloc(MAXDATASIZE);
    if (buffer == NULL)
    {
        perror("buffer == NULL");
    }
    size_t bytes = -1;
    int numbytes;
    while ((numbytes = recv(new_fd, buffer, MAXDATASIZE - 1, 0)) > 0)
    {
        buffer[numbytes] = '\0';
        // printf("%s", buffer);
        // fwrite(buffer, 1, numbytes, fptr);
    }
    // fclose(fptr);
    close(new_fd);
    printf("finish %d\n", new_fd);
    clock_gettime(CLOCK_REALTIME, &end);
    // time_spent = end - start
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char CC_type[20] = "cubic";
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    for (size_t i = 0; i < 2; i++)
    {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        double averageTime = 0;
        for (size_t j = 0; j < 5; j++)
        { // main accept() loop
            averageTime += getFile(new_fd);
        }
        printf("\033[32;1m average Time mode: \033[34m%s \033[0m is %f\n", CC_type, averageTime);
        strcpy(CC_type, "reno");
    }

    return 0;
}