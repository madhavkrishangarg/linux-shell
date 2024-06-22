# Shell Program

This shell program demonstrates the usage of `fork()`, `wait()`, and `execl()` family of system calls. It allows the user to enter commands, which are then executed by creating child processes. Additionally, the shell supports running commands in the background using threads.

## Working

1. **Forking a Process**: When the user enters a command, the `fork()` method is called to create a child process.
2. **Executing the Command**: In the child process, the `execvp()` command is used to execute the process specified by the user.
3. **Waiting for Completion**: The parent process uses the `wait()` call to wait for the child process to complete. This ensures the output of the program is deterministic.
4. **Running in Background**: Users can prefix their command with `&t` to run the command in the background by creating a thread.

## Running the Shell

To compile and run the shell program, use the following commands:

```bash
gcc shell.c -o shell
./shell
```
