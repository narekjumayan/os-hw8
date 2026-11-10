#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    int fd = open("numbers.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    for (int i = 1; i <= 10; i++) {
        char line[16];
        int len = snprintf(line, sizeof(line), "%d\n", i);

        if (write(fd, line, len) != len) {
            perror("write");
            close(fd);
            return 1;
        }
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    fd = open("numbers.txt", O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    char ch;
    int newlines = 0;

    while (newlines < 3) {
        ssize_t n = read(fd, &ch, 1);

        if (n != 1) {
            if (n == -1) perror("read");
            else fprintf(stderr, "Line 4 not found\n");
            close(fd);
            return 1;
        }

        if (ch == '\n')
            newlines++;
    }

    off_t line4Start = lseek(fd, 0, SEEK_CUR);

    if (line4Start == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    while (1) {
        ssize_t n = read(fd, &ch, 1);

        if (n != 1) {
            if (n == -1) perror("read");
            else fprintf(stderr, "Unexpected EOF\n");
            close(fd);
            return 1;
        }

        if (ch == '\n')
            break;
    }

    off_t remainderStart = lseek(fd, 0, SEEK_CUR);
    off_t fileEnd = lseek(fd, 0, SEEK_END);

    if (remainderStart == -1 || fileEnd == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    size_t remainderSize = (size_t)(fileEnd - remainderStart);

    char *remainder = malloc(remainderSize ? remainderSize : 1);

    if (remainder == NULL) {
        perror("malloc");
        close(fd);
        return 1;
    }

    if (lseek(fd, remainderStart, SEEK_SET) == -1) {
        perror("lseek");
        free(remainder);
        close(fd);
        return 1;
    }

    size_t totalRead = 0;

    while (totalRead < remainderSize) {
        ssize_t n = read(fd, remainder + totalRead,
                         remainderSize - totalRead);

        if (n <= 0) {
            if (n == -1) perror("read");
            else fprintf(stderr, "Unexpected EOF\n");
            free(remainder);
            close(fd);
            return 1;
        }

        totalRead += n;
    }

    if (lseek(fd, line4Start, SEEK_SET) == -1) {
        perror("lseek");
        free(remainder);
        close(fd);
        return 1;
    }

    const char *replacement = "100\n";
    size_t replacementLen = strlen(replacement);

    size_t totalWritten = 0;

    while (totalWritten < replacementLen) {
        ssize_t n = write(fd, replacement + totalWritten,
                          replacementLen - totalWritten);

        if (n <= 0) {
            perror("write");
            free(remainder);
            close(fd);
            return 1;
        }

        totalWritten += n;
    }

    totalWritten = 0;

    while (totalWritten < remainderSize) {
        ssize_t n = write(fd, remainder + totalWritten,
                          remainderSize - totalWritten);

        if (n <= 0) {
            perror("write");
            free(remainder);
            close(fd);
            return 1;
        }

        totalWritten += n;
    }

    off_t newSize = line4Start + replacementLen + remainderSize;

    if (ftruncate(fd, newSize) == -1) {
        perror("ftruncate");
        free(remainder);
        close(fd);
        return 1;
    }

    free(remainder);

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    printf("Final file content:\n");
    fflush(stdout);

    char buffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        size_t written = 0;

        while (written < (size_t)bytesRead) {
            ssize_t n = write(STDOUT_FILENO, buffer + written,
                              (size_t)bytesRead - written);

            if (n <= 0) {
                perror("write");
                close(fd);
                return 1;
            }

            written += n;
        }
    }

    if (bytesRead == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
