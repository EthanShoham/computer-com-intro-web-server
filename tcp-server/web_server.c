#include <stdio.h>
#include <assert.h>
#pragma comment(lib, "Ws2_32.lib")
#include <WS2tcpip.h>
#include <WinSock2.h>
#include "web_server.h"
#include "http_request.h"
#include "http_response.h"

typedef struct WSAData WSAData;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

#define SERVER_PORT 8080
#define MAX_CONNECTIONS 60

enum SocketRecvState {
  SOCKET_RECV_ACTIVE,
  SOCKET_RECV_IDLE,
  SOCKET_RECV_FINISHED
};

enum SocketSendState {
  SOCKET_SEND_ACTIVE,
  SOCKET_SEND_IDLE,
  SOCKET_SEND_FINISHED
};

#define RECV_BUFFER_SIZE 256
#define SEND_BUFFER_SIZE 256

typedef struct SocketState {
  SOCKET socket;
  unsigned long long timeout_time;

  enum SocketRecvState recv_state;
  char recv_buffer[RECV_BUFFER_SIZE];
  size_t recv_buffer_start;
  size_t recv_buffer_end;

  enum SocketSendState send_state;
  char send_buffer[SEND_BUFFER_SIZE];
  size_t send_buffer_start;
  size_t send_buffer_end;

  HttpRequsest http_request;
  HttpResponse http_response;
} SocketState;

enum HttpRequestState {
  HTTP_REQUEST_READING_HEADER,
  HTTP_REQUEST_READING_BODY,
  HTTP_REQUSET_DONE
};

typedef struct HttpRequest {
  enum HttpRequstState state;

  enum HttpMethod method;
  HttpQuery *query;
  HttpHeaders *headers;

  SocketState *socket_state;
} HttpRequest;

enum HttpMethod req_get_method(const HttpRequest* req) {
  assert(req && "req_get_method: req is null.");
  assert(req->state != HTTP_REQUEST_READING_HEADER && "req_get_method: cannot accesse method bofore finished reading headers.");
  return req->method;
}
const HttpQuery* req_get_query(const HttpRequest* req) {
  assert(req && "req_get_query: req is null.");
  assert(req->state != HTTP_REQUEST_READING_HEADER && "req_get_query: cannot accesse query bofore finished reading headers.");
  return req->query;
}
const HttpHeaders* req_get_headers(const HttpRequest* req) {
  assert(req && "req_get_headers: req is null.");
  assert(req->state != HTTP_REQUEST_READING_HEADER && "req_get_headers: cannot accesse headers bofore finished reading headers.");
  return req->headers;
}

bool req_can_read(const HttpRequest* req) {
  assert(req && "req_can_read: req is null.");
  assert(req->state != HTTP_REQUEST_READING_HEADER && "req_can_read: cannot accesse body bofore finished reading headers.");
  assert(req->socket_state && "req_can_read: req has a null socket_state, something went wrong!");

  const SocketState* socket_state = req->socket_state;

  return socket_state->recv_buffer_start != socket_state->recv_buffer_end;
}
size_t req_read(HttpRequest* req, char* buffer, size_t length) {
  assert(req && "req_read: req is null.");
  assert(req->state != HTTP_REQUEST_READING_HEADER && "req_read: cannot access body before finished reading headers.");
  assert(req->socket_state && "req_read: req has a null socket_state, something went wrong!");

  if(length == 0) {
    return 0;
  }

  SocketState* state = req->socket_state;

  size_t chars_read_count = 0;
  while(state->recv_buffer_start != state->recv_buffer_end) {
    if(chars_read_count == length) {
      break;
    }

    buffer[chars_read_count++] = state->recv_buffer[state->recv_buffer_start++];
  }

  return chars_read_count;
}


enum HttpResponseState {
  HTTP_RESPONSE_WRITING_STATUS_CODE,
  HTTP_RESPONSE_WRITING_HEADERS,
  HTTP_RESPONSE_WRITING_BODY,
  HTTP_RESPONSE_DONE
};

struct HttpResponse {
  enum HttpResponseState state;

  enum HttpStatusCode http_status_code;
  HttpHeaders* headers;
  void *context;

  SocketState* socket_state;
};

void res_set_status_code(HttpResponse *res,enum HttpStatusCode code) {
  assert(res && "res_set_status_code: res is null");
  assert(res->state == HTTP_RESPONSE_WRITING_STATUS_CODE && "res_set_status_code: response all ready set status code.");

  res->http_status_code = code;
  res->state = HTTP_RESPONSE_WRITING_HEADERS;
}
HttpHeaders* res_get_headers(const HttpResponse* res) {
  assert(res && "res_get_headers: res is null");
  assert(res->state & (HTTP_RESPONSE_WRITING_STATUS_CODE | HTTP_RESPONSE_WRITING_HEADERS) && "res_get_headers: response all ready finished writing headers.");
  assert(res->headers && "res_get_headers: response headers is null");

  return res->headers;
}
void res_finish_headers(HttpResponse* res) {
  assert(res && "res_finish_headers: res is null");
  assert(res->state & HTTP_RESPONSE_WRITING_HEADERS && "res_finish_headers: response status doesn't allow to finish headers. Did you forgot to set status code? or did you all ready finished headers?");

  res->state = HTTP_RESPONSE_WRITING_BODY;
}
void res_finish_write(HttpResponse* res) {
  assert(res && "res_finish_write: res is null");
  assert(res->state & HTTP_RESPONSE_WRITING_BODY && "res_finish_headers request");

  res->state = HTTP_RESPONSE_DONE;
}
void res_set_context(HttpResponse* res, const void* context) {
  assert(res && "res_set_context: res is null");

  res->context = context;
}
void* res_get_context(const HttpResponse* res) {
  assert(res && "res_get_context: res is null");

  return res->context;
}

bool res_can_write(const HttpResponse* res) {
  assert(res && "res_can_write: res is null.");
  assert(res->state == HTTP_RESPONSE_WRITING_BODY && "res_can_write: cannot write body bofore finishing writing headers.");
  assert(res->socket_state && "res_can_write: res has a null socket_state, something went wrong!");

  const SocketState* socket_state = req->socket_state;

  return socket_state->recv_buffer_start != socket_state->recv_buffer_end;
}
size_t res_wirte(HttpResponse* res, const char* buffer, size_t length) {
  assert(res && "res_write: res is null.");
  assert(res->state == HTTP_RESPONSE_WRITING_BODY && "res_write: cannot write body before finishing writing headers.");
  assert(res->socket_state && "res_write: res has a null socket_state, something went wrong!");

  if(length == 0) {
    return 0;
  }

  SocketState* state = req->socket_state;

  size_t chars_wrote_count = 0;
  while(state->recv_buffer_start != state->recv_buffer_end) {
    if(chars_wrote_count == length) {
      break;
    }

    state->recv_buffer[state->send_buffer_end++] = buffer[chars_wrote_count++];
  }

  return chars_wrote_count;
}

typedef struct WebServer {
  SocketState socket_states[MAX_CONNECTIONS];
} WebServer;

int web_server_run(WebServer *server) {
  WSAData wsa_data;

  if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
    printf("Server: Error at WSAStartup()\n");
    return 1;
  }

  SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (INVALID_SOCKET == listen_socket) {
    printf("Server: Error at socket(): %d\n", WSAGetLastError());
    WSACleanup();
    return 1;
  }

  sockaddr_in server_service;
  server_service.sin_family = AF_INET;
  server_service.sin_addr.s_addr = "127.0.0.1";
  // server_service.sin_addr.s_addr = INADDR_ANY;
  server_service.sin_port = htons(SERVER_PORT);

  if (SOCKET_ERROR ==
      bind(listen_socket, (SOCKADDR *)&server_service, sizeof(server_service))) {
    printf("Server: Error at bind(): %d\n", WSAGetLastError());
    closesocket(listen_socket);
    WSACleanup();
    return 1;
  }

  if (SOCKET_ERROR == listen(listen_socket, 5)) {
    printf("Server: Error at listen(): %d\n", WSAGetLastError());
    closesocket(listen_socket);
    WSACleanup();
    return 1;
  }

  printf("Server: listening on %s:%d\n", "127.0.0.1", SERVER_PORT);
  fd_set wait_recv;
  fd_set wait_send;

  while (true) {
    FD_ZERO(&wait_recv);
    FD_ZERO(&wait_send);



    for (int i = 0; i < MAX_SOCKETS; i++) {
      if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
        FD_SET(sockets[i].id, &waitRecv);
      if (sockets[i].send == SEND)
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
          receiveRequest(i);
          break;
        }
      }
      if (FD_ISSET(sockets[i].id, &waitSend)) {
        nfd--;
        switch (sockets[i].send) {
        case SEND:
          sendResponse(i);
          break;
        }
      }
    }
  }

  printf("Server: Closing Connection.\n");
  closesocket(listenSocket);
  WSACleanup();

  return 0;
  return 0;
}
