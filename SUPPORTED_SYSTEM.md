
## Supported OS Table

* The column **OS** is the list of different MS Windows generations.
* The column **last version can be run** is for the status of the latest release to be run under the OS. If the current (latest) version can be run under the specific OS, then the `current version`  will be indicated instead of version number.
* The column **supported** contains the status of Notepad++ team supporting Notepad++ under the specific OS. It means if a bug is present under an OS no more supported but not under supported OS, then this issue won't be treated by the team.

|           OS            | last version can be run  |      supported            |
|-------------------------|--------------------------|---------------------------|
| **Windows 95**          | v3.9                     |          No               |
| **Windows 98**          | v6.0                     |          No               |
| **Windows ME**          | v6.0                     |          No               |
| **Windows NT 4.0**      | v4.7.3                   |          No               |
| **Windows 2000**        | v6.6.9                   |          No               |
| **Windows XP**          | v7.9.2                   |          No               |
| **Windows Server 2003** | v7.9.2                   |          No               |
| **Windows Vista**       | v8.4.6  \*               |          No               |
| **Windows Server 2008** | v8.4.6  \*               |          No               |
| **Windows 7**           | current version          |          No               |
| **Windows 8**           | current version          |          No               |
| **Windows 8.1**         | current version          |          Yes              |
| **Windows 10**          | current version          |          Yes              |
| **Windows 11**          | current version          |          Yes              |
| **Linux** (GTK Port)    | current version          |          Yes              |

\* The current version of Notepad++ built by GCC can be run under Vista & Server 2008

*Note that the list is meant for the last SP of each version*

## Linux Support

A native Linux port using GTK3 is available in the `linux-gtk-prototype` directory. This is a fully-featured native implementation that provides:
- Native GTK3 interface
- Multi-cursor editing
- Column/rectangular selection
- Full Scintilla editing capabilities
- Syntax highlighting for 20+ languages
- 80+ keyboard shortcuts

Supported Linux distributions:
- Ubuntu 20.04+
- Debian 10+
- Fedora 33+
- Arch Linux (current)
- Other distributions with GTK 3.0+
