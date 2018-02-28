Running:
Use the "make all" command to compile the kernel module "pt2.ko" and make the "test" program which allows automatic testing and has the ability to test the system call interceptor on any user-provided pid. The kernel module must be first be inserted by using the command "sudo insmod pt2.ko". To determine that the module has been inserted you can use the command "lsmod", which should list the active modules. To remove the active module, use the command "sudo rmmod pt2". This is a requirement if re-compiling the module to then re-insert.

Testing:
Must compile and insert the kernel module first. To test the program, use the command "./test" to run an automatic test. To run a test using a specific pid (i.e. pid 1), pass the pid as the first and only argument (i.e. "./test 1").

Test Output:
An example of the test output has been provided in example_output.txt.

System Logs:
An example of the system log output has been provided in syslog.txt.

Notes:
If the shell prints "killed" after inserting the module, restart your virtual machine and try again after waiting a few minutes post-boot.
