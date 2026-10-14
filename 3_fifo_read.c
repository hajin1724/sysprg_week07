/*
 * 3_fifo_read.c — FIFO(이름 있는 파이프)로 메시지를 받는다  [먼저 실행할 쪽]
 *
 * [핵심 개념]
 *   파이프는 부모-자식처럼 관계있는 프로세스끼리만 쓸 수 있다.
 *   FIFO 는 파일 시스템에 이름을 가지므로 아무 관계 없는 두 프로그램도 통신할 수 있다.
 *   open 은 상대가 반대쪽을 열어 줄 때까지 기다린다(블록).
 *   출처: man 3 mkfifo, man 7 fifo
 *
 * [컴파일·실행]  터미널 두 개를 쓴다
 *   gcc -Wall -Wextra -o 3_fifo_read 3_fifo_read.c
 *   ./3_fifo_read          # 먼저 실행 (기다린다)
 *   ./4_fifo_write 안녕     # 다른 터미널에서 실행
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_PATH "/tmp/sysprog_fifo"

int main(void)
{
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    printf("보내는 쪽을 기다립니다... (%s)\n", FIFO_PATH);

    int fd = open(FIFO_PATH, O_RDONLY);   /* 상대가 열 때까지 여기서 기다린다 */
    if (fd == -1) { perror("open"); return 1; }

    printf("--- 받은 메시지 ---\n");

    char buf[128];
    ssize_t n;
    /* [변경] read 를 한 번이 아니라 반복한다. 보내는 쪽이 FIFO 를 닫으면 read 가 0 을 돌려줘 반복이 끝난다 */
    while ((n = read(fd, buf, sizeof buf - 1)) > 0) {
        buf[n] = '\0';
        fputs(buf, stdout);   /* 보내는 쪽이 단어마다 줄바꿈을 붙여 보내므로 그대로 출력 */
    }

    printf("--- 수신 종료 ---\n");   /* [변경] */

    close(fd);
    unlink(FIFO_PATH);   /* 다 썼으면 FIFO 파일도 지운다 */
    return 0;
}