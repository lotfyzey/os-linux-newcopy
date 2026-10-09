#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#define BUF_SIZE 65536

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        return 1;
    }

    const char *src = argv[1];
    char dst[PATH_MAX];
    struct stat src_st, dst_st;

    /* 1. check the source file */
    if (stat(src, &src_st) == -1) {
        fprintf(stderr, "%s: %s\n", src, strerror(errno));
        return 1;
    }
    if (!S_ISREG(src_st.st_mode)) {
        fprintf(stderr, "%s: not a regular file\n", src);
        return 1;
    }

    /* 2. if destination is a directory, copy inside it (like cp) */
    if (stat(argv[2], &dst_st) == 0 && S_ISDIR(dst_st.st_mode)) {
        const char *base = strrchr(src, '/');
        base = base ? base + 1 : src;
        snprintf(dst, sizeof(dst), "%s/%s", argv[2], base);
    } else {
        snprintf(dst, sizeof(dst), "%s", argv[2]);
    }

    /* 3. refuse to copy a file onto itself (it would be emptied) */
    if (stat(dst, &dst_st) == 0 &&
        src_st.st_dev == dst_st.st_dev &&
        src_st.st_ino == dst_st.st_ino) {
        fprintf(stderr, "'%s' and '%s' are the same file\n", src, dst);
        return 1;
    }

    /* 4. open both files */
    int in = open(src, O_RDONLY);
    if (in == -1) {
        fprintf(stderr, "%s: %s\n", src, strerror(errno));
        return 1;
    }

    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, src_st.st_mode & 0777);
    if (out == -1) {
        fprintf(stderr, "%s: %s\n", dst, strerror(errno));
        close(in);
        return 1;
    }

    /* 5. copy in chunks using a buffer */
    char *buf = malloc(BUF_SIZE);
    if (!buf) {
        perror("malloc");
        close(in);
        close(out);
        return 1;
    }

    int status = 0;
    ssize_t n;

    while ((n = read(in, buf, BUF_SIZE)) > 0) {
        ssize_t written = 0;
        while (written < n) {            /* write may write only part of the data */
            ssize_t w = write(out, buf + written, n - written);
            if (w == -1) {
                if (errno == EINTR)
                    continue;
                fprintf(stderr, "%s: write error: %s\n", dst, strerror(errno));
                status = 1;
                goto done;
            }
            written += w;
        }
    }

    if (n == -1) {
        fprintf(stderr, "%s: read error: %s\n", src, strerror(errno));
        status = 1;
    }

done:
    free(buf);
    close(in);
    if (close(out) == -1) {              /* close can fail on the final write */
        fprintf(stderr, "%s: close error: %s\n", dst, strerror(errno));
        status = 1;
    }
    return status;
}
