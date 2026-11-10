#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int write_all(int fd, const char *buffer, size_t length) {
    size_t total = 0;

    while (total < length) {
        ssize_t n = write(fd, buffer + total, length - total);

        if (n == -1 && errno == EINTR)
            continue;

        if (n <= 0) {
            if (n == -1)
                perror("write");
            else
                fprintf(stderr, "write returned 0\n");
            return -1;
        }

        total += n;
    }

    return 0;
}

int main(void) {
    char line[1024];
    char prefix[64];
    size_t length = 0;

    int fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    printf("Enter log message: ");
    fflush(stdout);

    while (1) {
        char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);

        if (n == -1 && errno == EINTR)
            continue;

        if (n == -1) {
            perror("read");
            close(fd);
            return 1;
        }

        if (n == 0 || ch == '\n')
            break;

        if (length >= sizeof(line)) {
            fprintf(stderr, "Input line too long\n");
            close(fd);
            return 1;
        }

        line[length++] = ch;
    }

    pid_t pid = getpid();

    int prefixLength = snprintf(prefix, sizeof(prefix),
                                "PID=%ld: ", (long)pid);

    if (prefixLength < 0 ||
        (size_t)prefixLength >= sizeof(prefix)) {
        fprintf(stderr, "Failed to format PID\n");
        close(fd);
        return 1;
    }

    if (write_all(fd, prefix, (size_t)prefixLength) == -1 ||
        write_all(fd, line, length) == -1 ||
        write_all(fd, "\n", 1) == -1) {
        close(fd);
        return 1;
    }

    off_t offset = lseek(fd, 0, SEEK_CUR);

    if (offset == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("Final file offset: %lld bytes\n", (long long)offset);

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
