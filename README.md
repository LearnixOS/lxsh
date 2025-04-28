
  



  
    
      𝗟𝗫𝗦𝗛
    
  
  
    A Minimal Shell for the LXOS Distribution
  

      




  


Overview
lxsh is a lightweight Unix shell written in C, designed as a minimal replacement for Bash, tailored for the LXOS Distribution. It supports scripting, job control, and interactive features, making it a versatile tool for both developers and system administrators.
Features

Configuration: Automatically creates and loads ~/.lxshrc with default PS1, aliases (ll, la), and HOME.
PS1 Prompt: Customizable with \u (user), \h (host), \w (directory with ~ substitution). Default: \u@\h:\w\$ .
Built-in Commands: cd, echo, exit, help, set, shopt, fg, bg, jobs.
Command Execution: Runs external programs and built-ins seamlessly.
Scripting: Executes scripts from files.
Environment Variables: Supports setting and expanding variables, including $? for exit status.
Redirection: Handles > and < for input/output redirection.
Pipes: Supports command pipelines (cmd1 | cmd2).
Job Control: Manages background (&) and foreground processes with fg, bg, and jobs.
Command History: Uses readline for command history, up/down arrow navigation, and Ctrl+left/right word jumping.
Command Substitution: Supports $(command) for inline command output.
Globbing: Expands wildcards (*, ?, [abc]).
Aliases: Defines shortcuts (e.g., alias ll='ls -l').
Signal Handling: Gracefully handles SIGINT (Ctrl+C).
Exit Status: Tracks last command status with $?.
Error Handling: Supports set -e for exiting on errors; trap is a placeholder.
Shell Options: Implements set -e and shopt with nocaseglob support.

Building
Prerequisites

gcc
make
libreadline-dev

On Ubuntu/Debian:
sudo apt-get install libreadline-dev

Build Instructions

Clone the repository:git clone https://github.com/LearnixOS/lxsh.git
cd lxsh


Build the shell:make


Run the shell:./lxsh



Installation
To set lxsh as your default shell:

Copy the binary to /usr/local/bin:sudo cp lxsh /usr/local/bin


Update your shell:chsh -s /usr/local/bin/lxsh



Usage

Interactive Mode:./lxsh


Run a Script:./lxsh script.sh


Display Help:./lxsh --help



Example ~/.lxshrc
PS1='\u@\h:\w\$ '
alias ll='ls -l'
alias la='ls -a'
HOME=$HOME

Interactive Features

Up/Down Arrows: Navigate command history.
Ctrl+Left/Right: Jump between words (requires ~/.inputrc configuration).

Create ~/.inputrc:
"\e[1;5C": forward-word
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

Advanced scripting features (e.g., conditionals, loops) are not fully implemented.
The trap command is a placeholder and lacks full functionality.
Some shopt options (e.g., globstar) are not supported.

Contributing
Contributions are welcome! To contribute:

Fork the repository.
Create a feature branch (git checkout -b feature/your-feature).
Commit your changes (git commit -m 'Add your feature').
Push to the branch (git push origin feature/your-feature).
Open a pull request.

Please report bugs or suggest features via GitHub Issues.
License
This project is licensed under the MIT License.
Acknowledgments

Inspired by Bash and other Unix shells.
Built with the readline library for interactive features.
Part of the LXOS Distribution.

