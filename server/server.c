/*** 여기서부터 이 책의 모든 예제에서 공통으로 포함하여 사용하는 코드이다. ***/

#include <sys/types.h>  // basic type definitions
#include <sys/socket.h> // socket(), AF_INET, ...
#include <arpa/inet.h>  // htons(), htonl(), ...
#include <netdb.h>      // gethostbyname(), ...
#include <unistd.h>     // close(), ...
#include <fcntl.h>      // fcntl(), ...
#include <pthread.h>    // pthread_create(), ...
#include <poll.h>       // poll()
#include <sys/epoll.h>  // epoll()

#include <stdio.h>  // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strerror(), ...
#include <errno.h>  // errno

// Windows 소켓 코드와 호환성을 위한 정의
typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define WIDTH 3
#define HEIGHT 3
// 소켓 함수 오류 출력 후 종료
void err_quit(const char *msg)
{
    char *msgbuf = strerror(errno);
    printf("[%s] %s\n", msg, msgbuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char *msg)
{
    char *msgbuf = strerror(errno);
    printf("[%s] %s\n", msg, msgbuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
    char *msgbuf = strerror(errcode);
    printf("[오류] %s\n", msgbuf);
}
char game_board[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

int checkwin() // 게임승리 조건
{
    if (game_board[1] == game_board[2] && game_board[2] == game_board[3])
        return 1;

    else if (game_board[4] == game_board[5] && game_board[5] == game_board[6])
        return 1;

    else if (game_board[7] == game_board[8] && game_board[8] == game_board[9])
        return 1;

    else if (game_board[1] == game_board[4] && game_board[4] == game_board[7])
        return 1;

    else if (game_board[2] == game_board[5] && game_board[5] == game_board[8])
        return 1;

    else if (game_board[3] == game_board[6] && game_board[6] == game_board[9])
        return 1;

    else if (game_board[1] == game_board[5] && game_board[5] == game_board[9])
        return 1;

    else if (game_board[3] == game_board[5] && game_board[5] == game_board[7])
        return 1;

    else if (game_board[1] != '1' && game_board[2] != '2' && game_board[3] != '3' &&
             game_board[4] != '4' && game_board[5] != '5' && game_board[6] != '6' && game_board[7] != '7' && game_board[8] != '8' && game_board[9] != '9')

        return 0;
    else
        return -1;
}

void board_display() // 틱택토 게임판을 보여준다
{
    printf("\n--------------------------------------------------------------------------");
    printf("\n\n\tTic Tac Toe\n\n");

    printf("CLIENT (X)  -  HOST (O)\n\n\n");

    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", game_board[1], game_board[2], game_board[3]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", game_board[4], game_board[5], game_board[6]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", game_board[7], game_board[8], game_board[9]);

    printf("     |     |     \n\n");
}

#define SERVERPORT 4001

int main(int argc, char *argv[])
{
    system("clear");
    int retval;

    // 소켓 생성 ssid= SOCKET listen_sock

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET)
        err_quit("socket()");

    // bind()   serveraddr -> csadd
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버ip주소
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR)
        err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR)
        err_quit("listen()");

    // 데이터 통신에 사용할 변수

    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    int i, choice;
    char mark;

    while (1)
    {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET)
        {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
               addr, ntohs(clientaddr.sin_port));

        i = checkwin();
        // 클라이언트와 데이터 통신
        while (i != 1)
        {

            retval = recv(client_sock, &choice, sizeof(choice), 0); // 클라이언트의 입력을 recv한다

            if (choice == 99) // 만약 상대가 99를 입력했다면 상대가 항복을 한것이다
            {
                printf("상대가 항복하였습니다. 접속을 종료합니다.");
                close(listen_sock);
                break;
            }
            if (choice == 110) // 클라이언트에서 wincheck()로 flag 110을 보내면 호스트의 승리
            {
                printf("당신(호스트)이 승리하였습니다. 접속을 종료합니다.");
                close(listen_sock);
                break;
            }

            mark = 'X';
            if (choice == 1 && game_board[1] == '1')
                game_board[1] = mark;

            else if (choice == 2 && game_board[2] == '2')
                game_board[2] = mark;

            else if (choice == 3 && game_board[3] == '3')
                game_board[3] = mark;

            else if (choice == 4 && game_board[4] == '4')
                game_board[4] = mark;

            else if (choice == 5 && game_board[5] == '5')
                game_board[5] = mark;

            else if (choice == 6 && game_board[6] == '6')
                game_board[6] = mark;

            else if (choice == 7 && game_board[7] == '7')
                game_board[7] = mark;

            else if (choice == 8 && game_board[8] == '8')
                game_board[8] = mark;

            else if (choice == 9 && game_board[9] == '9')
                game_board[9] = mark;

            i = checkwin();
            printf("\n%d\n", i);

            if (i == 1) // recv받은후 checkwin()실행후 i=1이면 클라이언트에게 호스트가 패배 하였다는 flag=110을 전송
            {
                board_display();
                int flag = 110;
                printf("클라이언트가 승리하였습니다.");
                retval = send(client_sock, &flag, sizeof(flag), 0);
                close(listen_sock);
                break;
            }

            board_display();
            printf("호스트 턴입니다 입력하세요! (만약 항복하길 원한다면 99를 입력하세요)\n");
            scanf("%d", &choice);

            if (choice == 99) // 만약99를 입력한다면 즉시 항복하여 게임을 끝낼 수 있다.
            {
                int flag = 99;
                printf("당신은 항복하였습니다 :( ");
                retval = send(client_sock, &flag, sizeof(flag), 0);
                close(listen_sock);
                break;
            }

            if (!(choice >= 1 && choice <= 9))
            {
                printf("잘못된 입력입니다! \n");
                printf("1~9사이의 숫자를 입력해주세요! (항복하길 원한다면 '99'를 입력하세요.) \n");
                scanf("%d", &choice);

                if (choice == 99) // 만약99를 입력한다면 즉시 항복하여 게임을 끝낼 수 있다.
                {
                    int flag = 99;
                    printf("당신은 항복하였습니다 :( ");
                    retval = send(client_sock, &flag, sizeof(flag), 0);
                    close(listen_sock);
                    break;
                }
            }

            mark = 'O';
            if (choice == 1 && game_board[1] == '1')
                game_board[1] = mark;

            else if (choice == 2 && game_board[2] == '2')
                game_board[2] = mark;

            else if (choice == 3 && game_board[3] == '3')
                game_board[3] = mark;

            else if (choice == 4 && game_board[4] == '4')
                game_board[4] = mark;

            else if (choice == 5 && game_board[5] == '5')
                game_board[5] = mark;

            else if (choice == 6 && game_board[6] == '6')
                game_board[6] = mark;

            else if (choice == 7 && game_board[7] == '7')
                game_board[7] = mark;

            else if (choice == 8 && game_board[8] == '8')
                game_board[8] = mark;

            else if (choice == 9 && game_board[9] == '9')
                game_board[9] = mark;

            board_display();
            retval = send(client_sock, &choice, sizeof(choice), 0); // 호스트의 입력을 클라이언트로 send한다.
        }

        // 소켓 닫기
        close(client_sock);
        printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
               addr, ntohs(clientaddr.sin_port));
    }

    // 소켓 닫기
    close(listen_sock);
    return 0;
}