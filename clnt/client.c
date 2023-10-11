/*** 여기서부터 이 책의 모든 예제에서 공통으로 포함하여 사용하는 코드이다. ***/

#include <sys/types.h>	// basic type definitions
#include <sys/socket.h> // socket(), AF_INET, ...
#include <arpa/inet.h>	// htons(), htonl(), ...
#include <netdb.h>		// gethostbyname(), ...
#include <unistd.h>		// close(), ...
#include <fcntl.h>		// fcntl(), ...
#include <pthread.h>	// pthread_create(), ...
#include <poll.h>		// poll()
#include <sys/epoll.h>	// epoll()

#include <stdio.h>	// printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strerror(), ...
#include <errno.h>	// errno

// Windows 소켓 코드와 호환성을 위한 정의
typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

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

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 4001

int main(int argc, char *argv[])
{
	system("clear");
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1)
		SERVERIP = argv[1];

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");

	// 데이터 통신에 사용할 변수
	////////////////

	int i, choice;
	char mark;

	i = checkwin();

	// 서버와 데이터 통신
	while (i != 1)
	{
		board_display();
		printf("클라이언트 턴입니다 입력하세요! (만약 항복하길 원한다면 99를 입력하세요)\n");
		scanf("%d", &choice);

		if (choice == 99) // 만약99를 입력한다면 즉시 항복하여 게임을 끝낼 수 있다.
		{
			int flag = 99;
			printf("당신은 항복하였습니다 :(");
			retval = send(sock, &flag, sizeof(flag), 0);
			close(sock);
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
				printf("당신은 항복하였습니다 :(");
				retval = send(sock, &flag, sizeof(flag), 0);
				close(sock);
				break;
			}
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

		retval = send(sock, &choice, sizeof(choice), 0); // 클라이언트의 입력을 호스트로 send한다.

		board_display();

		retval = recv(sock, &choice, sizeof(choice), 0); // 호스트의 입력을 recv한다

		if (choice == 99) // 호스트의 항복
		{

			printf("상대가 항복하였습니다. 접속을 종료합니다.");
			close(sock);
			break;
		}
		if (choice == 110) // 클라이언트의 승리
		{

			printf("당신(클라이언트)이 승리하였습니다. 접속을 종료합니다.");
			close(sock);
			break;
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

		i = checkwin();
		printf("\n%d\n", i);

		if (i == 1) // recv받은후 checkwin()실행 i=1이면 호스트에게 클라이언트가 패배 하였다는 flag=110을 전송
		{
			board_display();
			int flag = 110;
			printf("호스트가 승리하였습니다.");
			retval = send(sock, &flag, sizeof(flag), 0);
			close(sock);
			break;
		}
	}

	// 소켓 닫기
	close(sock);
	return 0;
}