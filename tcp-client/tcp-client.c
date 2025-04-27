#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <server> <port>\n", argv[0]);
        return 1;
    }

    const char *server = argv[1];
    const char *port   = argv[2];
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo hints, *result = NULL, *ptr = NULL;
    int ret;

    // 1. Initialize Winsock
    ret = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (ret != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", ret);
        return 1;
    }

    // 2. Resolve the server address and port
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
    hints.ai_protocol = IPPROTO_TCP;    // TCP protocol

    ret = getaddrinfo(server, port, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "getaddrinfo failed: %d\n", ret);
        WSACleanup();
        return 1;
    }

    // 3. Attempt to connect to the first address returned by getaddrinfo
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET) {
            fprintf(stderr, "socket failed: %ld\n", WSAGetLastError());
            continue;
        }

        // Connect to server.
        ret = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (ret == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // 4. Send and receive data
    const char *sendbuf = "Hello, server!";
    char recvbuf[512];
    int  recvlen;

    ret = send(sock, sendbuf, (int)strlen(sendbuf), 0);
    if (ret == SOCKET_ERROR) {
        fprintf(stderr, "send failed: %d\n", WSAGetLastError());
    } else {
        printf("Sent %d bytes to server: %s\n", ret, sendbuf);

        recvlen = recv(sock, recvbuf, sizeof(recvbuf) - 1, 0);
        if (recvlen > 0) {
            recvbuf[recvlen] = '\0';  // Null-terminate
            printf("Received %d bytes from server: %s\n", recvlen, recvbuf);
        } else if (recvlen == 0) {
            printf("Connection closed by server.\n");
        } else {
            fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
        }
    }

    // 5. Cleanup
    closesocket(sock);
    WSACleanup();

    return 0;
}
