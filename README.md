# linux-shell

Working of Shell: The programs uses fork(), wait() and execl() family of system calls.
When user enters a command, fork() method is called to make a child process. In the child process execvp() command is used to call the executable of process that has been entered by the user. Meanwhile parent process uses wait() call for the child process to complete. Using wait() call make output of the program deterministic.
