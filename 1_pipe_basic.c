/*
 * 1_pipe_basic.c — 파이프로 부모가 자식에게 문자열을 보낸다
 *
 * [핵심 개념]
 *   pipe(2) 는 파일 디스크립터 두 개를 준다: fd[0] 은 읽기, fd[1] 은 쓰기.
 *   fork 하면 자식이 이 디스크립터를 물려받으므로 부모-자식이 통신할 수 있다.
 *   각자 쓰지 않는 쪽은 반드시 닫아야 한다 — 쓰기 쪽이 모두 닫혀야 읽기 쪽이 EOF(0)를 본다.
 *   출처: man 2 pipe, man 7 pipe
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 1_pipe_basic 1_pipe_basic.c
 *   ./1_pipe_basic
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("사용법: ./1_pipe \"문자열\"\n");
        return 1;
    }
    
    int p2c[2];   /* parent to child: 부모가 쓰고 자식이 읽는 관 */
    int c2p[2];   /* child to parent: 자식이 쓰고 부모가 읽는 관 */

    if (pipe(p2c) == -1) { perror("pipe"); return 1; }
    if (pipe(c2p) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();   /* fork 이후 부모·자식이 fd[0], fd[1] 을 그대로 물려받아 같이 갖는다 */
    if (pid == -1) { perror("fork"); return 1; }

    if (pid == 0) {
        /* 자식 = 읽는 쪽. 쓰기 끝을 닫는다. */
        close(p2c[1]);
        close(c2p[0]);

        char buf[64];
        /* 파이프에서 읽는다. 마지막 한 자리는 남겨 뒤에 '\0'을 붙일 자리로 쓴다 */
        ssize_t n = read(p2c[0], buf, sizeof buf - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("  [자식] 받은 내용: %s\n", buf);
        }
        for (ssize_t i = 0; i < n; i++) {
            buf[i] = toupper((unsigned char)buf[i]);
        }
        write(c2p[1], buf, n);
        close(p2c[0]);
        close(c2p[1]);
        fflush(stdout);   /* _exit 은 버퍼를 비우지 않는다 — 직접 비워야 출력이 남는다 */
        _exit(0);
    }

    /* 부모 = 쓰는 쪽. 읽기 끝을 닫는다. */
    close(p2c[0]);
    close(c2p[1]);

    const char *msg = argv[1];
    write(p2c[1], msg, strlen(msg));   /* 널 종료 문자는 보내지 않는다 — 길이만큼만 씀 */
    close(p2c[1]);   /* 닫아야 자식의 read 가 끝난다 */


    char buf[64];
    ssize_t n = read(c2p[0], buf, sizeof buf - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("[부모] 돌려받은 내용: %s\n", buf);
    }
    close(c2p[0]);
    wait(NULL);
    return 0;
}
