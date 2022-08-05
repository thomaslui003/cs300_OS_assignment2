# CS300 simple shell - cshell

## Assignment Goals
To understand the relationship between OS command interpreters (shells), system calls, and the kernel.
To design and implement an extremely simple shell and system call (Bonus).

For this assignment, you should understand the concepts of environment variables, system calls, standard input and output, I/O redirection, parent and child processes, current directory, pipes, jobs, foreground and background, signals, and end-of-file.


## We can summarize the structure of the shell as follows:

1.print out a prompt

2.read a line of input from the user

3.parse the line into the program name, and an array of parameters
4.use the fork() system call to spawn a new child process

  a.the child process then uses the exec() system call to launch the specified program
  
  b.the parent process (the shell) uses the wait() system call to wait for the child to terminate
  
5.when the child (i.e. the launched program) finishes, the shell repeats the loop by jumping to 1.
