#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

int main(void) {
    char source[256], destination[256];
    char buffer[4096];
    ssize_t bytesRead;
    long long totalBytes = 0;

    printf("Enter source file path: ");
    if (scanf("%255s", source) != 1) {
        fprintf(stderr, "Invalid source path\n");
        return 1;
    }

    printf("Enter destination file path: ");
    if (scanf("%255s", destination) != 1) {
        fprintf(stderr, "Invalid destination path\n");
        return 1;
    }

    int src = open(source, O_RDONLY);
    if (src == -1) {
        perror("open source");
        return 1;
    }

    struct stat srcStat, dstStat;

    if (fstat(src, &srcStat) == -1) {
        perror("fstat source");
        close(src);
        return 1;
    }

    if (stat(destination, &dstStat) == 0 &&
        srcStat.st_dev == dstStat.st_dev &&
        srcStat.st_ino == dstStat.st_ino) {
        fprintf(stderr, "Source and destination are the same file\n");
        close(src);
        return 1;
    }

    int dst = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst == -1) {
        perror("open destination");
        close(src);
        return 1;
    }

    while ((bytesRead = read(src, buffer, sizeof(buffer))) > 0) {
        ssize_t bytesWritten = 0;

        while (bytesWritten < bytesRead) {
            ssize_t result = write(dst, buffer + bytesWritten,
                                   bytesRead - bytesWritten);

            if (result <= 0) {
                if (result < 0) {
                    perror("write");
                } else {
                    fprintf(stderr, "write returned 0\n");
                }
                close(src);
                close(dst);
                return 1;
            }

            bytesWritten += result;
            totalBytes += result;
        }
    }

    if (bytesRead == -1) {
        perror("read");
        close(src);
        close(dst);
        return 1;
    }

    if (close(src) == -1) {
        perror("close source");
        close(dst);
        return 1;
    }

    if (close(dst) == -1) {
        perror("close destination");
        return 1;
    }

    printf("Successfully copied %lld bytes.\n", totalBytes);

    return 0;
}
