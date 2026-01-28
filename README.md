# GumDrop
Linux rootkit research project (kinda started out as vibe-coded and as a C learning opportunity). 


## How it works.
1. After loading into the kernel it'll hook into the system calls for `__x64_sys_kill` or the system call for the "kill" command
2. After it registers and someone calls it with the signal of `42` the process where that kill command was ran will be granted root access to the system by preparing creds and setting the process where that kill command was ran creds to those prepared creds.
3. The kill system call is modified and returns 0 as if it ran successfully.

## Building:
1. `make`
2. `sudo insmod gumdrop.ko`

## Removing:
`sudo rmmod gumdrop`

## Use:
* `kill -42 12345` or any PID will grant the process that ran kill root access.
* More coming soon

## other:
* Blog post coming soon on research.


## Disclaimer
This project is intended for educational and research purposes, including the study of operating system internals and security mechanisms.
Users are responsible for complying with all applicable laws and regulations.
The software is provided "AS IS", without warranty of any kind.
