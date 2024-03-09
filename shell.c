#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
typedef enum
{
    shell_builtin,
    executable_or_error,
} input_type;

/*
 * Release all resources from any zombie process.
 */
void reap_child_zombie()
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

/*
 * Track child processes terminations and record them in the file.
 */
void write_to_log_file()
{
    FILE *file;
    file = fopen("/home/vboxuser/Documents/log.txt", "a");
    fprintf(file, "Child terminated\n");
    fclose(file);
}

/*
 * Handle termination of any child process to avoid having zombies by using signals.
 */
void on_child_exit()
{
    reap_child_zombie();
    write_to_log_file();
}

/*
 *Setup the directory of the shell to the home directory.
 */
void setup_environment()
{
    chdir(getenv("HOME"));
}

/*
 * Execute builtin commands : "cd", "echo" and "export".
 *
 * Parameters:
 * - args: Array of strings representing the command and its arguments.
 */
void execute_shell_bultin(char *args[])
{
    static char curdir[200];
    char tmpdir[200];

    if (!strcmp(args[0], "cd"))
    {
        if (!args[1] || !strcmp(args[1], "~"))
        {
            char *dir = getenv("HOME");
            strcpy(curdir, dir);
        }
        else if (!strcmp(args[1], "."))
        {
            getcwd(tmpdir, sizeof(tmpdir));
            strcpy(curdir, tmpdir);
        }
        else
        {
            strcpy(curdir, args[1]);
        }
        chdir(curdir);
        getcwd(tmpdir, sizeof(tmpdir));
    }
    else if (!strcmp(args[0], "echo"))
    {
        for (int i = 1; args[i] != NULL; i++)
        {
            printf("%s ", args[i]);
        }
        printf("\n");
    }
    // review that place
    else if (!strcmp(args[0], "export"))
    {
        char var[200];
        char *value;
        char finalval[200];
        int found = 0;
        for (int i = 1; args[i] != NULL; i++)
        {
            int size = strlen(args[i]);
            int indx = strcspn(args[i], "=");
            if (indx != size)
            {
                value = strchr(args[i], '=');
                value++;
                strncpy(var, args[i], indx);
                var[indx] = '\0';
                strcpy(finalval, value);
                found = i;
            }
            if (found > 0 && found != i)
            {
                strcat(finalval, " ");
                strcat(finalval, args[i]);
            }
        }
        setenv(var, finalval, 1);
    }
}

/*
 * Execute other commands by fork a subprocess and execute it using "execvp" system call.
 * if the command has "&" argument the parent process doesn't wait for its child to terminate.
 * if "&" argument doesn't exist the parent process waits for the termination of its child
 *
 * Parameters:
 * - args: Array of strings representing the command and its arguments.
 */
void execute_command(char *args[])
{
    int found_flag = 0;
    for (int i = 1; args[i] != NULL; i++)
    {
        if (!strcmp(args[i], "&"))
        {
            found_flag = 1;
            args[i] = NULL;
        }
    }
    int child_id = fork();
    if (child_id < 0)
    {
        perror("fork");
        exit(0);
    }
    else if (!child_id)
    {
        // child part execute the process here
        // print("Error)
        execvp(args[0], args);
        perror("execvp");
        exit(0);
    }
    else
    {
        if (!found_flag)
            waitpid(child_id, NULL, 0);
    }
}

/*
 * Read the input line from user and remove the new line character .
 */
char *read_input()
{
    static char line[200] = {};
    fgets(line, sizeof(line), stdin);
    int indx = strcspn(line, "\n");
    line[indx] = '\0';
    return line;
}

/*
 * Split the input line into array of strings.
 *
 * Parameters:
 * - line: Input string from user after remove newline character.
 * - args: Array of strings representing the command and its arguments.
 */
void parse_input(char *line, char *args[])
{
    args[0] = strtok(line, " ");
    for (int i = 1; args[i - 1] != NULL; i++)
    {
        args[i] = strtok(NULL, " ");
    }
    for (int i = 1; args[i] != NULL; i++)
    {
        // remove first " and last "
        int indx = strcspn(args[i], "\"");
        int tempsize = strlen(args[i]);
        if (indx == 0 && tempsize != 0)
        {
            memmove(args[i], args[i] + 1, strlen(args[i] + 1) + 1);
        }
        indx = strcspn(args[i], "\"");
        tempsize = strlen(args[i]) - 1;
        if (indx == tempsize)
        {
            args[i][tempsize] = '\0';
        }
    }
}

/*
 * Get the value of any variable in the environment and update the expression/command line.
 *
 * Parameters:
 * - line:Input string from user after remove newline character.
 */
char *evaluate_expression(char *line)
{
    char right_half[200];
    char left_half[200];
    char mid_half[200];
    int indx = strcspn(line, "$");
    while (indx != strlen(line))
    {
        if (indx != 0)
            strncpy(right_half, line, indx);
        else
            strcpy(right_half, "\0");

        right_half[indx] = '\0';
        strcpy(left_half, line + indx);
        int num_of_copy = strcspn(left_half, " ") - 1;
        strncpy(mid_half, left_half + 1, num_of_copy);
        mid_half[num_of_copy] = '\0';

        if (strchr(left_half, ' ') == NULL)
            strcpy(left_half, "\0");
        else
            strcpy(left_half, strchr(left_half, ' '));

        strcpy(mid_half, getenv(mid_half));
        strcat(right_half, mid_half);
        strcat(right_half, left_half);
        strcpy(line, right_half);

        indx = strcspn(line, "$");
        sleep(2);
    }

    return line;
}

/*
 * check the type of the command.
 */
input_type find_type(char *arg)
{
    if (strcmp(arg, "cd") == 0 || strcmp(arg, "echo") == 0 || strcmp(arg, "export") == 0)
        return shell_builtin;
    return executable_or_error;
}

/*
 * Provides a simple shell interface for command execution.
 * The shell runs in a loop, allowing users to input commands and executes them.
 * The loop continues until the user enters the "exit" command.
 */
void shell()
{
    int exit_now = 0;
    while (exit_now == 0)
    {
        char *args[100] = {};
        char *username = getlogin();
        // char dir[200];
        // getcwd(dir,sizeof(dir));
        // char *home = getenv("HOME");
        // if (!strcmp(home ,dir))
        //     strcpy(dir,"~");
        printf("%s@Ubuntu: ", username);
        parse_input(evaluate_expression(read_input()), args);
        if (!args[0])
            continue;

        if (args[0] && strcmp(args[0], "exit") == 0)
        {
            exit_now = 1;
            break;
        }

        input_type type = find_type(args[0]);

        switch (type)
        {
        case shell_builtin:
            execute_shell_bultin(args);
            break;
        case executable_or_error:
            execute_command(args);
            break;
        }
    }
}

int main()
{
    signal(SIGCHLD, on_child_exit);
    setup_environment();
    shell();
    return 0;
}
