<div align="center">
  <img 
    src="https://raw.githubusercontent.com/LearnixOS/learnixos.github.io/refs/heads/main/assets/images/logo.png" 
    alt="LXSH Logo" 
    width="150" 
    style="display: block; margin: 0 auto; border: 2px solid #555; border-radius: 12px; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);"
  >
</div>

<div align="center">
  
  <h1 style="font-size: 48px; margin-top: 20px;">
    <a href="https://learnixos.github.io/" style="text-decoration: none; color: inherit;">
      𝗟𝗫𝗦𝗛
    </a>
  </h1>
  <p style="font-size: 18px; margin-top: 10px;">
    𝗔 𝗺𝗶𝗻𝗶𝗺𝗮𝗹 𝘀𝗵𝗲𝗹𝗹 𝗳𝗼𝗿 𝘁𝗵𝗲 𝗟𝗫𝗢𝗦 𝗗𝗶𝘀𝘁𝗿𝗶𝗯𝘂𝘁𝗶𝗼𝗻
  </p>
</div>

---

<div align="center">

<h1>
  <img src="assets/lxsh.png" align="center" alt="Preview" width="650" style="display: inline-block; margin: 0; border: 2px solid #555; border-radius: 12px; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);">
</h1>

lxsh: A Bash-like Shell in C
lxsh is a Unix shell written in C, designed as a lightweight bash replacement with support for scripting, job control, and interactive features.
Features

Configuration: Loads ~/.lxshrc, auto-created with default PS1, aliases (ll, la), and HOME.
PS1 Prompt: Customizable with \u (user), \h (host), \w (dir, with ~ substitution). Default: \u@\h:\w\$ .
Built-in Commands: cd, echo, exit, help, set, shopt, fg, bg, jobs.
Command Execution: Runs external programs and built-ins.
Scripting: Executes scripts from files.
Environment Variables: Supports setting and expanding variables, including $?.
Redirection: Handles >, <.
Pipes: Supports cmd1 | cmd2.
Job Control: Manages background (&) and foreground processes with fg, bg, jobs.
Command History: Uses readline for history, up/down arrow navigation, Ctrl+left/right word jumping.
Command Substitution: Supports $(command).
Globbing: Expands wildcards (*, ?, [abc]).
Aliases: Defines shortcuts (e.g., alias ll='ls -l').
Signal Handling: Handles SIGINT (Ctrl+C).
Exit Status: Tracks $?.
Error Handling: Supports set -e; trap is a placeholder.
Shell Options: set -e, shopt with nocaseglob.

Building

Install dependencies: gcc, make, libreadline-dev.
On Ubuntu: sudo apt-get install libreadline-dev.


Run make to build lxsh.
Run ./lxsh to start the shell.

Installation
To set lxsh as your default shell:

Copy lxsh to /usr/local/bin.
Run chsh -s /usr/local/bin/lxsh.

Usage

Interactive: ./lxsh
Script: ./lxsh script.sh
Help: ./lxsh --help

Example ~/.lxshrc
PS1='\u@\h:\w\$ '
alias ll='ls -l'
alias la='ls -a'
HOME=$HOME

Interactive Features

Up/Down Arrows: Navigate command history.
Ctrl+Left/Right: Jump between words (requires .inputrc configuration).
Create ~/.inputrc:"\e[1;5C": forward-word
"\e[1;5D": backward-word



Example Usage
$ ./lxsh
user@machine:~$ echo $HOME
/home/user
user@machine:~$ ls | grep txt
file.txt
user@machine:~$ sleep 10 &
[1] 1234
user@machine:~$ jobs
[1] Running sleep 10
user@machine:~$ fg
sleep 10
user@machine:~$ echo $(pwd)
/home/user
user@machine:~$ ls *.txt > output.txt

Limitations

Advanced scripting (conditionals, loops) is not fully implemented.
trap is a placeholder.
Some shopt options are not supported.

Contributing
Contributions are welcome! Please submit pull requests or open issues on the project repository.

