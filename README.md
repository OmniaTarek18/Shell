# Shell Program
Welcome to the Shell Program in C! This project aims to provide you with hands-on experience with system calls in Unix environments, processes, multi-processing, and signal handling.

## Introduction
This Shell Program is designed to mimic a basic command-line interface, where users can interact with the system by executing various commands. The program is implemented in C language, utilizing system calls and the execvp function for command execution.

## Features
  * Command Execution: The shell supports a variety of commands, including cd, echo, export, and others.
  * System Calls: The program utilizes system calls for performing various operations, such as creating processes and managing directories.
  * Signal Handling: It handles signals such as SIGCHLD for dealing with zombie processes.
  * String Manipulation: The program includes functionalities for string manipulation to parse and process user input.
    
## Commands
  1. **cd**: Change the current directory.
  2. **echo**: Display a line of text.
  3. **export**: Set environment variables.
  4. Other commands are executed using the execvp function.
     
## Usage
To use the shell, simply compile the program and run the executable. Once the shell is running, you can enter commands and press Enter to execute them.

## Getting Started
To get started with the Shell Program:
   1. Clone the repository to your local machine.
   2. Compile the source code using a C compiler.
   3. Run the executable to start the shell.
```
$ gcc shell.c -o shell
$ ./shell
```
## Demo

[](https://example.com/your-video.mp4)

Thank you for using the Shell Program in C! Happy coding! 🚀
