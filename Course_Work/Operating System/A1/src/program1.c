#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>





void checkSignal(int);

int main(int argc, char *argv[]) {
    /* fork a child process */
    int status = -1;
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork Error!\n");
        exit(1);
    } else {
        //In the child process, pid = 0
        if (pid == 0) {
	    //Child process
            int i;
            char *arg[argc];
            arg[argc - 1] = NULL;
            /* execute test program */
            printf("I'm the child process. my pid = %d\n", getpid());
            for (i = 0; i < argc; ++i) {
                arg[i] = argv[i + 1];
            }
	    /*execve(const char*, char* const, char * const) 
	     *argv[0]: Path of the executed program
	     * arg: Array of pointers to strings passed to the new program as command-line arguments
             * envp[]: Environment variable of the executed program
             */
	    printf("Child process start to execute the program.\n");
            execve(arg[0], arg, NULL);
       
        } else {
        //In the Parent Process, pid will return the pid of child.
	    char *signal[] = {"?", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS",
                  "SIGFPE",
                  "SIGKILL", "SIGUSR1", "SIGEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM",
                  "SIGSTKFLT",
                  "SIGCHLD", "SIGCONT", "SIGSTOP", "..."};
	    //Suspends the execution of the current process until one of its children terminates, then it
            //will free the zombie process.
	    /* wait for child process terminates */
            printf("Process start to fork\n");
            printf("I'm the parent process. my pid = %d\n", getpid());
            waitpid(pid, &status, WUNTRACED);
            printf("Parent process receiving the SIGCHILD signal\n");


            /* check child process'termination status */
            if (WIFEXITED(status)) {
                printf("Normal termination with EXIT STATUS = %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child process get %s signal\n", signal[WTERMSIG(status)]);
                checkSignal(WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("Child process get a stop signal %s", signal[WSTOPSIG(status)]
                       );
                checkSignal(WSTOPSIG(status));
            } else {
                printf("Child process continued \n");
            }

        }
    }


}

/*
 *This function will print out the relational information 
 * according to the signal get by waitpid's return status and macros.
 */

void checkSignal(int status) {
    switch (status) {
        case 1:
            printf("Hangup detected on controlling terminal\n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 2:
            printf("Interrupt from keyboard and recieved the core dump signal\n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 3:
            printf("Quit from keyboard\n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 4:
            printf("Illegal Instruction detected \n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 5:
            printf("Process is trapped. \n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 6:
	    printf("Child process is aborted by abort signal\n");
 	    printf("CHILD EXECUTION FAILED!!\n");
            break;
        case 7:
            printf("Bad memory access detected \n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 8:
            printf("Floating point exception detected \n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 9:
            printf("Child process recived a terminating signal \n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 11:
            printf("Segmentation fault detected \n");
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 13:
            printf("Broken pipe: write to pipe with no readers\n"
            );
 	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 14:
            printf("Timer signal detected \n");
	    printf("CHILD EXECUTION OVERTIME!!\n");
            break;
        case 15:
            printf("Termination signal detected \n");
	    printf("CHILD EXECUTION TERMINATED!!\n");
            break;
        case 19:
            printf("Child process is stopped\n");
	    printf("CHILD EXECUTION STOPPED!!\n");
            break;
    }
}
