# CS300 simple shell - cshell

## Assignment Goals
To understand the relationship between OS command interpreters (shells), system calls, and the kernel.
To design and implement an extremely simple shell and system call.

For this assignment, you should understand the concepts of environment variables, system calls, standard input and output, I/O redirection, parent and child processes, current directory, pipes, jobs, foreground and background, signals, and end-of-file.


## Shell structure summarize:

1. Print out a prompt

2. Read a line of input from the user

3. Parse the line into the program name, and an array of parameters
4. Use the fork() system call to spawn a new child process

    a. The child process then uses the exec() system call to launch the specified program
  
    b. The parent process (the shell) uses the wait() system call to wait for the child to terminate
  
5. When the child process(i.e. the launched program) finishes, the shell repeats the loop by jumping to 1.
