#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int write_all(int fd, const char *buffer, size_t length) {
    size_t total = 0;

    while (total < length) {
        ssize_t n = write(fd, buffer + total, length - total);

        if (n == -1 && errno == EINTR)
            continue;

        if (n <= 0) {
            if (n < 0)
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
    const char *filename = "sparse.bin";

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (write_all(fd, "START", 5) == -1) {
        close(fd);
        return 1;
    }

    off_t offset = lseek(fd, 1024 * 1024, SEEK_CUR);

    if (offset == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    if (write_all(fd, "END", 3) == -1) {
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);

    if (size == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("Apparent file size: %lld bytes\n", (long long)size);

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
