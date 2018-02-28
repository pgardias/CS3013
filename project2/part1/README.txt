Running:
Use the "make all" command to compile the kernel module "pt1.ko". The kernel module must first be inserted by using the command "sudo insmod pt1.ko". To determine that the module has been inserted you can use the command "lsmod", which should list the active modules. To remove the active module, use the command "sudo rmmod pt1". This is a requirement for re-compiling the module to then re-insert.

Testing:
Must compile and insert the kernel module first. To test the program, run any program that uses open/read/close operations on a file (basically any program). Alternatively, simply use the command "cat VIRUS.txt" which will print the content of the bundled file VIRUS.txt to the shell.

System Logs:
An example of the system log output has been provided in syslog.txt. The system logs should contain output such as the following:
Jan 28 20:39:54 CS-3013-VM kernel: [  216.749521] User 1001 is opening file: /usr/share/icons/default/cursors/top_side
