/*
 * 2_pipe_dup2.c — 셸의 "ls | wc -l" 을 직접 만들어 본다
 *
 * [핵심 개념]
 *   dup2(a, b) 는 디스크립터 b 가 a 와 같은 곳을 가리키게 만든다.
 *   자식의 표준출력(1)을 파이프의 쓰기 끝으로 바꿔치기하면
 *   그 프로그램은 아무것도 몰라도 출력이 파이프로 흘러간다. 셸이 하는 일이 바로 이것이다.
 *   출처: man 2 dup2, man 2 pipe
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 2_pipe_dup2 2_pipe_dup2.c
 *   ./2_pipe_dup2      # ls | wc -l 과 같은 결과가 나온다
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int colon = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ":") == 0) {
            colon = i;
            break;
        }
    }

    if (colon == -1 || colon == 1 || colon == argc - 1) {
        printf("사용법: ./2_pipeline 명령1 [인자] : 명령2 [인자]\n");
        return 1;
    }

    argv[colon] = NULL;               /* ":" 자리를 NULL로 바꿔 앞 명령의 끝으로 쓴다 */
    char **cmd1 = &argv[1];           /* 앞 명령 */
    char **cmd2 = &argv[colon + 1];   /* 뒤 명령 */

    int fd[2];   /* fd[0] = 읽기 끝, fd[1] = 쓰기 끝 */
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    /* 첫 번째 자식: ls 를 실행하고 출력을 파이프로 보낸다 */
    if (fork() == 0) {   /* 반환값 0 이면 자식 쪽 코드 경로 */
        close(fd[0]);                 /* 읽기 끝은 쓰지 않는다 */
        dup2(fd[1], STDOUT_FILENO);   /* 표준출력을 파이프 쓰기 끝으로 교체 */
        close(fd[1]);                 /* 원본 디스크립터는 이제 필요 없다 */
        execvp(cmd1[0], cmd1);
        perror("execvp 앞 명령");
        _exit(127);                 /* _exit: 부모의 버퍼·자원을 다시 정리하지 않고 즉시 종료 */
    }

    /* 두 번째 자식: wc -l 을 실행하고 입력을 파이프에서 받는다 */
    if (fork() == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);    /* 표준입력을 파이프 읽기 끝으로 교체 */
        close(fd[0]);
        execvp(cmd2[0], cmd2);
        perror("execvp 뒤 명령");
        _exit(127);
    }

    /* 부모는 양쪽 다 닫아야 한다. 안 닫으면 wc 가 EOF 를 못 봐서 끝나지 않는다. */
    close(fd[0]);
    close(fd[1]);

    wait(NULL);   /* 자식 두 개를 각각 기다려 좀비로 남지 않게 한다 */
    wait(NULL);
    return 0;
}
