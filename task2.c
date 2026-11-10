#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    const char *filename = "data.txt";
    const char *text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    size_t length = strlen(text);
    size_t written = 0;

    while (written < length) {
        ssize_t n = write(fd, text + written, length - written);
        if (n <= 0) {
            if (n < 0) perror("write");
            else fprintf(stderr, "write returned 0\n");
            close(fd);
            return 1;
        }
        written += n;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    fd = open(filename, O_RDWR);
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

    printf("Original file size: %lld bytes\n", (long long)size);

    if (ftruncate(fd, 10) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("New file size: %lld bytes\n", (long long)size);

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    char buffer[11];
    size_t total = 0;

    while (total < 10) {
        ssize_t n = read(fd, buffer + total, 10 - total);
        if (n == -1) {
            perror("read");
            close(fd);
            return 1;
        }
        if (n == 0) break;
        total += n;
    }

    buffer[total] = '\0';

    printf("Remaining content: %s\n", buffer);

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
