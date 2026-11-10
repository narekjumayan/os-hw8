#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    char filename[256];
    char ch;

    printf("Enter file path: ");
    if (scanf("%255s", filename) != 1) {
        fprintf(stderr, "Invalid file path\n");
        return 1;
    }

    int fd = open(filename, O_RDONLY);
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

    for (off_t i = size; i > 0; i--) {
        if (lseek(fd, i - 1, SEEK_SET) == -1) {
            perror("lseek");
            close(fd);
            return 1;
        }

        ssize_t bytesRead = read(fd, &ch, 1);
        if (bytesRead != 1) {
            if (bytesRead == -1)
                perror("read");
            else
                fprintf(stderr, "Unexpected EOF\n");

            close(fd);
            return 1;
        }

        if (write(STDOUT_FILENO, &ch, 1) != 1) {
            perror("write");
            close(fd);
            return 1;
        }
    }

    if (write(STDOUT_FILENO, "\n", 1) != 1) {
        perror("write");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
