#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

int main (int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* filename = argv[1];

    char path[PATH_MAX];
    if (realpath(filename, path) != 0) {
        exit(EXIT_FAILURE);
    }

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        fprintf(stderr, "Cannot open file %s for writing\n", path);
        exit(EXIT_FAILURE);
    }

    int fds[2];
    if (pipe(fds) == -1) {
        switch (errno) {
            case EMFILE:
                fprintf(stderr, "Too many file descriptors are in use by the process. ");
                break;
            case ENFILE:
                fprintf(stderr, "The system limit on the total number of open files has been reached.");
                break;
        }

        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        switch (errno) {
            case EAGAIN:
                fprintf(stderr, "Cannot allocate sufficient memory to copy the parent's page tables and allocate a task structure for the child.");
                break;
            case ENOMEM:
                fprintf(stderr, "Failed to allocate the necessary kernel structures because memory is tight.");
                break;
            case ENOSYS:
                fprintf(stderr, "fork() is not supported on this platform.");
                break;
        }
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        dup2(fds[0], STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fds[0]);
        close(fds[1]);
        close(fd);

        if (execlp("sort", "sort", NULL) == -1) {
            exit(EXIT_FAILURE);
        }
    }

    // NOTE: fdopen errors only with EINVAL
    //       which is not applicable in this case
    FILE* stream = fdopen(fds[1], "w");

    fprintf(stream, "This is a test.\n");
    fprintf(stream, "Hello, world.\n");
    fprintf(stream, "My dog has fleas.\n");
    fprintf(stream, "This program is great.\n");
    fprintf(stream, "One fish, two fish.\n");
    fflush(stream);

    close(fds[0]);
    close(fds[1]);
    close(fd);

    /* Wait for the child process to finish. */
    waitpid(pid, NULL, 0);
}