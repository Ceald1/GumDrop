# GumDrop
Linux rootkit research project (kinda started out as vibe-coded and as a C learning opportunity). 


## How it works.
### Woot!
1. After loading into the kernel it'll hook into the system calls for `__x64_sys_kill` or the system call for the "kill" command
2. After it registers and someone calls it with the signal of `42` and if the target process is the same as the process the command originates from like bash, will be granted root access to the system by preparing creds and setting the process where that kill command was ran creds to those prepared creds.
3. The kill system call is modified and returns 0 as if it ran successfully.

### Sneaky beaky mode
* By default the module will not be hidden.
* Just like popping a root process but instead use `41` as the signal.
* Send `64` to a process to hide/unhide it from everything.

## Building:
1. `make`
2. `sudo insmod gumdrop.ko`

## Removing:
1. `kill -41 $$ # unhide the module if hidden`
2. `sudo rmmod gumdrop --force # needs to be force because it will leave an empty directory after being unhidden`

## Use:
* `kill -42 $$` will grant the process that ran kill root access, this tries to send the signal code 42 to the current process.
* `kill -64 target_pid` will hide or unhide the process from the entire system.
* `kill -41 $$` will hide/unhide the rootkit
* More coming soon


## other:
* Blog post coming soon on research.


## Disclaimer
This project is intended for educational and research purposes, including the study of operating system internals and security mechanisms.
Users are responsible for complying with all applicable laws and regulations.
The software is provided "AS IS", without warranty of any kind.
