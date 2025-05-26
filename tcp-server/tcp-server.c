#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#pragma comment(lib, "Ws2_32.lib")
#include <WS2tcpip.h>
#include <WinSock2.h>

typedef struct WSAData WSAData;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

#define SERVER_PORT 8080
#define MAX_SOCKETS 60

#define EMPTY 0
#define LISTEN 1
#define RECEIVE 2
#define IDLE 3
#define SEND 4
#define SEND_TIME 1
#define SEND_SECONDS 2

typedef struct SocketState {
  SOCKET id;
  int recv;
  int send;
  int sendSubType;
  char buffer[128];
  int len;
} SocketState;
SocketState sockets[MAX_SOCKETS] = {0};
int socketsCount = 0;

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);

typedef struct WebApplication {
  int (*run)(void);
} WebApplication;

typedef struct WebApplicationBuilder {
  WebApplication (*build)(void);
} WebApplicationBuilder;

WebApplicationBuilder CreateWebApplicationBuilder();

int main(void) { return CreateWebApplicationBuilder().build().run(); }

int _web_application_run(void) {
  WSAData wsaData;

  if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
    printf("Server: Error at WSAStartup()\n");
    return 1;
  }

  SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (INVALID_SOCKET == listenSocket) {
    printf("Server: Error at socket(): %d\n", WSAGetLastError());
    WSACleanup();
    return 1;
  }

  sockaddr_in serverService;
  serverService.sin_family = AF_INET;
  serverService.sin_addr.s_addr = INADDR_ANY;
  serverService.sin_port = htons(SERVER_PORT);

  if (SOCKET_ERROR ==
      bind(listenSocket, (SOCKADDR *)&serverService, sizeof(serverService))) {
    printf("Server: Error at bind(): %d\n", WSAGetLastError());
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  if (SOCKET_ERROR == listen(listenSocket, 5)) {
    printf("Server: Error at listen(): %d\n", WSAGetLastError());
    closesocket(listenSocket);
    WSACleanup();
    return 1;
  }

  addSocket(listenSocket, LISTEN);

  while (true) {
    fd_set waitRecv;
    fd_set waitSend;
    FD_ZERO(&waitRecv);
    FD_ZERO(&waitSend);
    for (int i = 0; i < MAX_SOCKETS; i++) {
      if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
        FD_SET(sockets[i].id, &waitRecv);
      else if (sockets[i].send == SEND)
        FD_SET(sockets[i].id, &waitSend);
    }

    int nfd;
    nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
    if (nfd == SOCKET_ERROR) {
      printf("Server: Error at select(): %d\n", WSAGetLastError());
      WSACleanup();
      return 1;
    }

    for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++) {
      if (FD_ISSET(sockets[i].id, &waitRecv)) {
        nfd--;
        switch (sockets[i].recv) {
        case LISTEN:
          acceptConnection(i);
          break;
        case RECEIVE:
          receiveMessage(i);
          break;
        }
      } else if (FD_ISSET(sockets[i].id, &waitSend)) {
        nfd--;
        switch (sockets[i].send) {
        case SEND:
          sendMessage(i);
          break;
        }
      }
    }
  }

  printf("Server: Closing Connection.\n");
  closesocket(listenSocket);
  WSACleanup();

  return 0;
}

bool addSocket(SOCKET id, int what) {
  for (int i = 0; i < MAX_SOCKETS; i++) {
    if (sockets[i].recv == EMPTY) {
      sockets[i].id = id;
      sockets[i].recv = what;
      sockets[i].send = IDLE;
      sockets[i].len = 0;
      socketsCount++;
      return (true);
    }
  }
  return (false);
}

void removeSocket(int index) {
  sockets[index].recv = EMPTY;
  sockets[index].send = EMPTY;
  socketsCount--;
}

void acceptConnection(int index) {
  SOCKET id = sockets[index].id;
  struct sockaddr_in from;
  int fromLen = sizeof(from);

  SOCKET msgSocket = accept(id, (struct sockaddr *)&from, &fromLen);
  if (INVALID_SOCKET == msgSocket) {
    printf("Server: Error at accept(): %d\n", WSAGetLastError());
    return;
  }

  char ipv4[16];
  printf("Server: Client %s:%d is connected.\n",
         inet_ntop(AF_INET, &from.sin_addr, ipv4, 16), ntohs(from.sin_port));

  unsigned long flag = 1;
  if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0) {
    printf("Server: Error at ioctlsocket(): %d\n", WSAGetLastError());
    closesocket(id);
    return;
  }

  if (addSocket(msgSocket, RECEIVE) == false) {
    printf("\t\tToo many connections, dropped!\n");
    closesocket(id);
  }
}

void receiveMessage(int index) {
  SOCKET msgSocket = sockets[index].id;

  int len = sockets[index].len;
  int bytesRecv = recv(msgSocket, &sockets[index].buffer[len],
                       sizeof(sockets[index].buffer) - len, 0);

  if (SOCKET_ERROR == bytesRecv) {
    printf("Server: Error at recv(): %d\n", WSAGetLastError());
    closesocket(msgSocket);
    removeSocket(index);
    return;
  }

  if (bytesRecv == 0) {
    closesocket(msgSocket);
    removeSocket(index);
    return;
  }

  sockets[index].buffer[len + bytesRecv] = '\0';
  printf("Server: Recieved: %d bytes of \"%s\" message.\n", bytesRecv,
         &sockets[index].buffer[len]);

  sockets[index].len += bytesRecv;

  if (sockets[index].len > 0) {
    if (strncmp(sockets[index].buffer, "TimeString", 10) == 0) {
      sockets[index].send = SEND;
      sockets[index].sendSubType = SEND_TIME;
      memcpy(sockets[index].buffer, &sockets[index].buffer[10],
             sockets[index].len - 10);
      sockets[index].len -= 10;
      return;
    } else if (strncmp(sockets[index].buffer, "SecondsSince1970", 16) == 0) {
      sockets[index].send = SEND;
      sockets[index].sendSubType = SEND_SECONDS;
      memcpy(sockets[index].buffer, &sockets[index].buffer[16],
             sockets[index].len - 16);
      sockets[index].len -= 16;
      return;
    } else if (strncmp(sockets[index].buffer, "Exit", 4) == 0) {
      closesocket(msgSocket);
      removeSocket(index);
      return;
    }
  }
}

void sendMessage(int index) {
  int bytesSent = 0;
  char sendBuff[255];

  SOCKET msgSocket = sockets[index].id;
  if (sockets[index].sendSubType == SEND_TIME) {
    time_t timer;
    time(&timer);
    char tBuff[10];
    ctime_s(tBuff, sizeof(tBuff), &timer);
    strcpy_s(sendBuff, 255, tBuff);
    sendBuff[strlen(sendBuff) - 1] = 0;
  } else if (sockets[index].sendSubType == SEND_SECONDS) {
    time_t timer;
    time(&timer);
    _itoa_s((int)timer, sendBuff, 255, 10);
  }

  bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
  if (SOCKET_ERROR == bytesSent) {
    printf("Server: Error at send(): %d\n", WSAGetLastError());
    closesocket(msgSocket);
    removeSocket(index);
    return;
  }

  printf("Time Server: Sent: %d\\%zu bytes of \"%s\" message.\n", bytesSent,
         strlen(sendBuff), sendBuff);

  sockets[index].send = IDLE;
}

WebApplication _web_application_builder_build(void) {
  WebApplication app = {.run = _web_application_run};
  return app;
}

WebApplicationBuilder CreateWebApplicationBuilder() {
  WebApplicationBuilder builder = {.build = _web_application_builder_build};

  return builder;
}
