#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

int main()
{
    char *path = "file.txt";
    int fd;
    char *output_buf = "hoge uga oro\n";
    char input_buf[100];

    fd = open(path, O_WRONLY | O_APPEND);
    if(fd == -1) {
        perror("open");
        exit(1);
    }

    printf("lock start\n");
    if((flock(fd, LOCK_EX) != 0)) {
        perror("lock");
        exit(1);
    }
    printf("lock OK\n");

    write(fd, output_buf, strlen(output_buf));

    getchar();

    if((flock(fd, LOCK_UN) != 0)) {
        perror("unlock");
        exit(1);
    }

    close(fd);

    return 0;
}
