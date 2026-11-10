#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 4096

ssize_t read_chunk(int fd, char *buffer, size_t count) {
    ssize_t n;

    do {
        n = read(fd, buffer, count);
    } while (n == -1 && errno == EINTR);

    return n;
}

int main(void) {
    char path1[256], path2[256];
    unsigned char buffer1[BUFFER_SIZE];
    unsigned char buffer2[BUFFER_SIZE];

    printf("Enter first file path: ");
    if (scanf("%255s", path1) != 1) {
        fprintf(stderr, "Invalid first path\n");
        return 1;
    }

    printf("Enter second file path: ");
    if (scanf("%255s", path2) != 1) {
        fprintf(stderr, "Invalid second path\n");
        return 1;
    }

    int fd1 = open(path1, O_RDONLY);
    if (fd1 == -1) {
        perror("open first file");
        return 1;
    }

    int fd2 = open(path2, O_RDONLY);
    if (fd2 == -1) {
        perror("open second file");
        close(fd1);
        return 1;
    }

    unsigned long long offset = 0;
    size_t pos1 = 0, pos2 = 0;
    size_t len1 = 0, len2 = 0;
    int eof1 = 0, eof2 = 0;

    while (1) {
        if (pos1 == len1 && !eof1) {
            ssize_t n = read_chunk(fd1, (char *)buffer1, BUFFER_SIZE);

            if (n == -1) {
                perror("read first file");
                goto error;
            }

            len1 = (size_t)n;
            pos1 = 0;
            if (n == 0) eof1 = 1;
        }

        if (pos2 == len2 && !eof2) {
            ssize_t n = read_chunk(fd2, (char *)buffer2, BUFFER_SIZE);

            if (n == -1) {
                perror("read second file");
                goto error;
            }

            len2 = (size_t)n;
            pos2 = 0;
            if (n == 0) eof2 = 1;
        }

        int end1 = eof1 && pos1 == len1;
        int end2 = eof2 && pos2 == len2;

        if (end1 && end2) {
            if (close(fd1) == -1 || close(fd2) == -1) {
                perror("close");
                return 1;
            }

            printf("Files are identical\n");
            return 0;
        }

        if (end1 || end2) {
            printf("Files differ at byte %llu\n", offset);
            goto different;
        }

        if (buffer1[pos1] != buffer2[pos2]) {
            printf("Files differ at byte %llu\n", offset);
            goto different;
        }

        pos1++;
        pos2++;
        offset++;
    }

different:
    close(fd1);
    close(fd2);
    return 1;

error:
    close(fd1);
    close(fd2);
    return 1;
}
