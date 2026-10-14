/*
 * 4_fifo_write.c — FIFO 로 메시지를 보낸다  [나중에 실행할 쪽]
 *
 * [핵심 개념]
 *   받는 쪽(3_fifo_read)이 만들어 둔 FIFO 를 열어서 쓰기만 하면 된다.
 *   서로 부모-자식 관계가 아니어도 통신이 된다는 점이 파이프와 다르다.
 *   출처: man 7 fifo
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 4_fifo_write 4_fifo_write.c
 *   ./4_fifo_write 안녕하세요
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_PATH "/tmp/sysprog_fifo"

int main(int argc, char *argv[])
{
    /* [변경] 보낼 단어가 하나도 없으면 사용법을 알려 준다 */
    if (argc < 2) {
        printf("사용법: ./4_fifo_write 단어1 [단어2 ...]\n");
        return 1;
    }

    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open (받는 쪽을 먼저 실행했는지 확인)");
        return 1;
    }

    /* [변경] 인자로 받은 단어를 하나씩 보낸다. 구분을 위해 뒤에 줄바꿈도 같이 보낸다 */
    for (int i = 1; i < argc; i++) {
        if (write(fd, argv[i], strlen(argv[i])) == -1 || write(fd, "\n", 1) == -1) {
            perror("write");
            close(fd);
            return 1;
        }
        printf("보냈습니다: %s\n", argv[i]);
    }

    close(fd);   /* 닫아야 받는 쪽의 read 가 0 을 돌려주고 반복이 끝난다 */
    return 0;
}