/**
 * CS2106 AY22/23 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

struct PCBTable {
    pid_t pid;
    int status;  
    int exitCode;
    int stat_loc; 
};

int add_info(pid_t pid);
void update_info(pid_t pid, int status, int exitCode);
int read_command(char *command, char **tokens);
int execute_command(size_t num_tokens, char **tokens, size_t start_index);
int check_existence(char *abs_path);
void start(size_t num_tokens, char **tokens);
int concurrent(size_t num_tokens, char **tokens);
int re_stdin(size_t num_tokens, char **tokens, int *fd);
int re_stdout(size_t num_tokens, char **tokens, int *fd);
int re_stderr(size_t num_tokens, char **tokens, int *fd);
void my_run(size_t num_tokens, char **tokens);
void my_info(int op);
void my_quit();
void my_wait(pid_t pid);
void my_terminate(pid_t pid);
void SIG_STP(int sig);
void SIG_INT(int sig);
void my_fg(pid_t process_id);

struct PCBTable PCBT[50];
int process_started = 0;

pid_t work_pid = -1; 

void my_init(void) {
    // Initialize what you need here
}

int read_command(char *command, char **tokens) {
    return strcmp(tokens[0], command) == 0;
}

void update_info(pid_t pid, int status, int exitCode) {
    for (int i=0; i<process_started; i++) {
        if (PCBT[i].pid == pid) {
            PCBT[i].status = status;

            if (status == 1 && PCBT[i].exitCode == -1) 
            {
                PCBT[i].exitCode = exitCode;
            }
            break;
        }
    }
}

int add_info(pid_t pid) {
    PCBT[process_started].pid = pid;
    PCBT[process_started].status = 2; 
    PCBT[process_started].exitCode = -1;
    setpgid(pid, 0);

    int process_index = process_started;
    process_started++;
    return process_index;
}

void my_process_command(size_t num_tokens, char **tokens) {
    // Your code here, refer to the lab document for a description of the arguments

    if (read_command("info", tokens)) {
        if (tokens[1] != NULL) 
        {
            my_info(atoi(tokens[1]));
        } 
        else 
        {
            printf("Wrong command\n");
        }
    }
    else if (read_command("wait", tokens)) 
    {
        my_wait(atoi(tokens[1]));
    } 
    else if (read_command("terminate", tokens)) 
    {
        my_terminate(atoi(tokens[1]));
    } 
    else if (read_command("fg", tokens)) 
    {
        my_fg(atoi(tokens[1]));
    } else {
        my_run(num_tokens, tokens);
    }
}

void my_run(size_t num_tokens, char **tokens) {
    int curr_index = 0;

    while (curr_index < (int)num_tokens) 
    {
        curr_index = execute_command(num_tokens, tokens, curr_index);
    }    
}

int execute_command(size_t num_tokens, char **tokens, size_t begin) {
    char *buffer[100];

    for (size_t i=begin; i<num_tokens-1; i++) 
    {
        if (strcmp(tokens[i], ";") == 0) 
        {
            buffer[i - begin] = NULL;
            if (check_existence(buffer[0]) == -1) 
            {
                printf("%s not found\n", buffer[0]);
                return i + 1;
            }

            size_t exe_tokens = i - begin + 1;
            start(exe_tokens, buffer);
            return i + 1;
        }
        buffer[i - begin] = tokens[i];
    }

    buffer[num_tokens - begin - 1] = NULL;
    if (check_existence(buffer[0]) == -1) 
    {
        printf("%s not found\n", buffer[0]);
        return num_tokens;
    }
    size_t exe_tokens = num_tokens - begin;
    start(exe_tokens, buffer);
    return num_tokens;
}

int check_existence(char *abs_path) {
    return access(abs_path, F_OK);
}

void start(size_t num_tokens, char **tokens) {
    int fd_0 = -1, fd_1 = -1, fd_2 = -1;
    int std_in = re_stdin(num_tokens, tokens, &fd_0);    
    int std_out = re_stdout(num_tokens, tokens, &fd_1);
    int std_err = re_stderr(num_tokens, tokens, &fd_2);

    if (concurrent(num_tokens, tokens) == 1) 
    {
        pid_t child_pid = fork();

        if (child_pid == 0) 
        {
            if (std_out == 1) 
            {
                dup2(fd_1, STDOUT_FILENO);
            }

            if (std_err == 1)
            {
                dup2(fd_2, STDERR_FILENO);                
            }

            if (std_in == 1) 
            {                
                dup2(fd_0, STDIN_FILENO);                
                execl(tokens[0], tokens[0], NULL);
                _exit(EXIT_FAILURE);
            }
            execv(tokens[0], tokens);
            _exit(EXIT_FAILURE); 
        } 
        else 
        {
            signal(SIGTSTP, SIG_STP);
            signal(SIGINT, SIG_INT);

            printf("Child[%d] in background\n", child_pid);
            if (std_in == -1) 
            {
                kill(child_pid, SIGTERM);
                add_info(child_pid);
                update_info(child_pid, 1, 1);
            }
            else
            {
                add_info(child_pid);
            }
        }
    }
    else 
    {
        pid_t child_pid = fork();

        if (child_pid == 0) 
        {
            if (std_out == 1) 
            {                
                dup2(fd_1, STDOUT_FILENO);                
            }

            if (std_err == 1)
            {                
                dup2(fd_2, STDERR_FILENO);                
            }

            if (std_in == 1) 
            {
                dup2(fd_0, STDIN_FILENO);                
                execl(tokens[0], tokens[0], NULL);
                _exit(EXIT_FAILURE);
            }
            execv(tokens[0], tokens);
            _exit(EXIT_FAILURE);
        } 
        else 
        {
            if (std_in == -1) 
            {
                kill(child_pid, SIGTERM);
                add_info(child_pid);
                update_info(child_pid, 1, 1);
            }
            else
            {
                signal(SIGTSTP, SIG_STP);
                signal(SIGINT, SIG_INT);
                int index = add_info(child_pid);
                work_pid = child_pid;

                waitpid(child_pid, &PCBT[index].stat_loc, WUNTRACED);

                if (WIFEXITED(PCBT[index].stat_loc) == 1 && WEXITSTATUS(PCBT[index].stat_loc) != 0)
                {
                    update_info(child_pid, 1, WEXITSTATUS(PCBT[index].stat_loc));
                    return;
                } 

                if (WIFSIGNALED(PCBT[index].stat_loc) == 1 && WTERMSIG(PCBT[index].stat_loc) == SIGINT)
                {
                    printf("[%d] interrupted\n", child_pid);
                    return;
                }

                if (WIFSTOPPED(PCBT[index].stat_loc) == 1 && WSTOPSIG(PCBT[index].stat_loc) == SIGTSTP)
                {                
                    printf("[%d] stopped\n", child_pid);
                    update_info(child_pid, 4, -1);
                    return;
                }

                update_info(child_pid, 1, WEXITSTATUS(PCBT[index].stat_loc));
                work_pid = -1;
                return;
            }
        }
    }
}

int concurrent(size_t num_tokens, char **tokens) {
    for (size_t i=0; i<num_tokens-1; i++) 
    {
        if (tokens[i] != NULL && strcmp(tokens[i], "&") == 0) 
        {
            tokens[i] = NULL;
            return 1; 
        }
    }
    return 0;
}

int re_stdin(size_t num_tokens, char **tokens, int *fd) {
    for (size_t i=0; i<num_tokens-1; i++) 
    {
        if (tokens[i] != NULL && strcmp(tokens[i], "<") == 0) 
        {
            if (access(tokens[i+1], R_OK) == -1) 
            {
                printf("%s does not exist\n", tokens[i+1]);
                return -1;
            } 
            else 
            {
                *fd = open(tokens[i+1], O_RDONLY);
                tokens[i] = NULL;
                tokens[i+1] = NULL;
                return 1;
            }
        }
    }
    return 0;
}

int re_stdout(size_t num_tokens, char **tokens, int *fd) {
    for (size_t i=0; i<num_tokens-1; i++) 
    {
        if (tokens[i] != NULL && strcmp(tokens[i], ">") == 0)
        {
            *fd = open(tokens[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH);
            tokens[i] = NULL;
            tokens[i+1] = NULL;
            return 1;
        }
    }
    return 0;
}

int re_stderr(size_t num_tokens, char **tokens, int *fd) {
    for (size_t i=0; i<num_tokens-1; i++) 
    {
        if (tokens[i] != NULL && strcmp(tokens[i], "2>") == 0) 
        {
            *fd = open(tokens[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH);
            tokens[i] = NULL;
            tokens[i+1] = NULL;
            return 1;
        }
    }
    return 0;
}

void my_wait(pid_t pid) {
    signal(SIGTSTP, SIG_STP);
    signal(SIGINT, SIG_INT);

    for (int i=0;i<process_started;i++)
    {
        if (PCBT[i].pid == pid && PCBT[i].status == 2)
        {
            int exitCode;
            work_pid = pid;
            waitpid(pid, &exitCode, 0);

            if (WIFSIGNALED(exitCode) && WTERMSIG(exitCode) == SIGINT) 
            {
                printf("[%d] interrupted\n", pid);
                return;
            }
            if (WIFSTOPPED(exitCode) && WSTOPSIG(exitCode) == SIGTSTP) 
            {                
                printf("[%d] stopped\n", pid);
                update_info(pid, 4, -1);
                return;
            }
            update_info(pid, 1, WEXITSTATUS(exitCode));
            work_pid = -1;
            return;
        }
    }
}

void my_terminate(pid_t pid) {
    for (int i=0;i<process_started;i++) 
    {
        if (PCBT[i].pid == pid) 
        {
            if (PCBT[i].status == 2)
            {
                kill(pid, SIGTERM);
                PCBT[i].status = 3;
                break;
            }
        }
    }
}

void my_quit(void) {
    for (int i=0; i<process_started; i++)
    {          
        int status;
        int change = waitpid(PCBT[i].pid, &status, WNOHANG);
        
        if (change == PCBT[i].pid) 
        {
            continue;
        } 

        if (PCBT[i].status == 2) 
        {
            printf("Killing [%d]\n",PCBT[i].pid);
            killpg(PCBT[i].pid, SIGTERM);
            waitpid(PCBT[i].pid, NULL, 0);
        }
        else if (PCBT[i].status == 4) 
        {
            killpg(PCBT[i].pid, SIGCONT);
            printf("Killing [%d]\n",PCBT[i].pid);
            killpg(PCBT[i].pid, SIGTERM);
            waitpid(PCBT[i].pid, NULL, 0);
        }
        else if (PCBT[i].status == 3) 
        {
            waitpid(PCBT[i].pid, NULL, 0);
        }
    }
    printf("\nGoodbye\n");
}

void my_info(int op) {
    if (op == 0) 
    {
        for (int i=0; i<process_started; i++) {

            if (PCBT[i].status == 3) 
            {
                int status;
                int change = waitpid(PCBT[i].pid, &status, WNOHANG);

                if (change == 0) 
                {
                    printf("[%d] Terminating\n", PCBT[i].pid);
                } 
                else
                {
                    PCBT[i].status = 1;
                    if (PCBT[i].exitCode < 0)
                    {
                        PCBT[i].exitCode = WEXITSTATUS(status);
                        if (WIFSIGNALED(status))
                        {
                            PCBT[i].exitCode = 15;
                        }
                    }
                    printf("[%d] Exited %d\n", PCBT[i].pid, PCBT[i].exitCode);
                }  
            } 
            else if (PCBT[i].status == 2) 
            {
                int status;
                int change = waitpid(PCBT[i].pid, &status, WNOHANG);
                if (change == 0) 
                {
                    printf("[%d] Running\n", PCBT[i].pid);
                }
                else
                {
                    PCBT[i].status = 1; 
                    PCBT[i].exitCode = WEXITSTATUS(status); 
                    printf("[%d] Exited %d\n", PCBT[i].pid, PCBT[i].exitCode);
                }
            }
            else if (PCBT[i].status == 4)
            {
                printf("[%d] Stopped\n", PCBT[i].pid);
            }
            else if (PCBT[i].status == 1) 
            {
                printf("[%d] Exited %d\n", PCBT[i].pid, PCBT[i].exitCode);
            }
        }
    } 
    else if (op == 1) 
    {
        int total = 0;
        for (int i=0; i<process_started; i++) 
        {
            if (PCBT[i].status == 3) 
            {
                int status;
                if (waitpid(PCBT[i].pid, &status, WNOHANG) == PCBT[i].pid)
                {
                    PCBT[i].status = 1; 
                    PCBT[i].exitCode = WEXITSTATUS(status);
                    total += 1;
                }
            } 
            else if (PCBT[i].status == 2) 
            {
                int status;
                if (waitpid(PCBT[i].pid, &status, WNOHANG) == PCBT[i].pid) 
                {
                    PCBT[i].status = 1; 
                    PCBT[i].exitCode = WEXITSTATUS(status); 
                    total += 1;
                }
            } 
            else if (PCBT[i].status == 1)             total += 1;  
        }
        printf("Total exited process: %d\n",total);
    } 
    else if (op == 2) 
    {
        int total = 0;
        for (int i=0; i<process_started; i++) 
        {
            if (PCBT[i].status == 2) 
            {
                if (kill(PCBT[i].pid,0) == 0)
                {
                    total += 1;       
                }        
            }
        }
        printf("Total running process: %d\n",total);
    } 
    else if (op == 3)
    {
        int total = 0;
        for (int i=0; i<process_started; i++)
        {
            if (PCBT[i].status == 3 && kill(PCBT[i].pid,0) == 0) 
            {
                total += 1;
            }
        }
        printf("Total terminating process: %d\n",total);
    }
    else if (op == 4)
    {
        int total = 0;
        for (int i=0; i<process_started; i++)
        {
            if (PCBT[i].status == 4 && kill(PCBT[i].pid,0) == 0) 
            {
                total += 1;
            }
        }
        printf("Total stopped process: %d\n",total);
    }
    else 
    {
        printf("Wrong command\n");
    }
}

void SIG_STP(int sig) {
    signal(SIGTSTP, SIG_STP);
    if (work_pid != -1)
    {
        killpg(getpgid(work_pid), SIGTSTP);
    }
    work_pid = -1;
}

void SIG_INT(int sig) {
    signal(SIGINT, SIG_INT);
    if (work_pid != -1) 
    {
        killpg(getpgid(work_pid), SIGINT);       
        update_info(work_pid, 1, 2);
    }    
    work_pid = -1;
}

void my_fg(pid_t process_id) {
    signal(SIGINT, SIG_INT);
    signal(SIGTSTP, SIG_STP);

    for (int i=0; i<process_started; i++) {
        if (PCBT[i].pid == process_id) {
            if (PCBT[i].status == 4) {
                printf("[%d] resumed\n", process_id);
                killpg(getpgid(process_id), SIGCONT);
                work_pid = process_id;
    
                int status;
                waitpid(process_id, &status, WUNTRACED);

                if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT) 
                {
                    printf("[%d] interrupted\n", process_id);
                    return;
                }

                if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTSTP) 
                {                
                    printf("[%d] stopped\n", process_id);
                    update_info(process_id, 4, -1);
                    work_pid = -1;
                    return;
                }

                update_info(process_id, 1, WEXITSTATUS(status));
                return;
            }
        }
    }
}





