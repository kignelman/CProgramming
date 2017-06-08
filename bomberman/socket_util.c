//
// Created by balbe on 28/05/2017.
//

#include "socket_util.h"


struct sockaddr_in create_sockaddr_in(char *hostname, int port) {
    struct hostent *host_info = NULL;
    struct sockaddr_in sin = {0};
    host_info = gethostbyname(hostname);

    if (host_info == NULL) {
        return sin;
    }
    sin.sin_family = AF_INET;
    sin.sin_addr = *(IN_ADDR *) host_info->h_addr;
    sin.sin_port = htons(port);

    return sin;
}

SOCKET create_client(char *hostname, int port) {
    SOCKET sock;
    struct sockaddr_in sin;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    sin = create_sockaddr_in(hostname, port);

    if (connect(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        return INVALID_SOCKET;
    }

    return sock;
}


SOCKET create_server(char *hostname, int port) {
    SOCKET sock;
    struct sockaddr_in sin;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    sin = create_sockaddr_in(hostname, port);

    if (bind(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        return INVALID_SOCKET;
    }

    listen(sock, 0);

    return sock;
}

int recv_server_request(SOCKET socket, server_request_t *request) {

    char message[2048];
    int reading;

    reading = recv(socket, message, sizeof(message), 0);
    if (reading > 0) {
        deserialize_server_request(message, request);
    }
    return reading;
}

int recv_client_request(SOCKET socket, client_request_t *request) {

    char message[4];
    int reading;

    reading = recv(socket, message, sizeof(message), 0);
    if (reading > 0) {
        deserialize_client_request(message, request);
    }
    return reading;
}

int send_server_request(SOCKET socket, server_request_t *request) {
    char message[2048];
    serialize_server_request(request, message);
    return send(socket, message , sizeof(message), 0);
}

int send_client_request(SOCKET socket, client_request_t *request) {
    char message[4];
    serialize_client_request(request, message);
    return send(socket, message , sizeof(message), 0);
}

void serialize_server_request(server_request_t *request, char data[2048]) {

    int i, j, k, t = 0;

    for (i = 0; i < 2048; i++)
        data[i] = '\0';

    data[0] = (char)((request->protocol % 26) + 'a');
    data[1] = (char)((request->alive % 26) + 'a');
    data[2] = (char)((request->position[0] % 26) + 'a');
    data[3] = (char)((request->position[1] % 26) + 'a');

    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            for (k = 0; k < 2; k++) {
                data[4 + t + k] = (char)((request->map[i][j][k] % 26) + 'a');
                data[4 + t + k + MAP_WIDTH * MAP_HEIGHT * 2] = (char)((request->clients[i][j][k] % 26) + 'a');
            }
            t += 2;
        }
    }
}

void deserialize_server_request(char data[2048], server_request_t *request) {
    int i, j, k, t = 0;

    request->protocol = ((int) data[0]) - (int) 'a';
    request->alive = ((int) data[1]) - (int) 'a';
    request->position[0] = ((int) data[2]) - (int) 'a';
    request->position[1] = ((int) data[3]) - (int) 'a';

    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            for (k = 0; k < 2; k++) {
                request->map[i][j][k] = ((int) data[4 + t + k]) - (int) 'a';
                request->clients[i][j][k] = ((int) data[4 + t + k + MAP_WIDTH * MAP_HEIGHT * 2]) - (int) 'a';
            }

            t += 2;
        }
    }
}

void serialize_client_request(client_request_t *request, char data[4]) {
    int i;
    for (i = 0; i < 4; i++)
        data[i] = '\0';

    data[0] = (char)((request->protocol % 26) + 'a');
    data[1] = (char)((request->value % 26) + 'a');
}

void deserialize_client_request(char data[4], client_request_t *request) {
    request->protocol = ((int) data[0]) - (int) 'a';
    request->value = ((int) data[1]) - (int) 'a';
}

void start_socket() {
#if defined(WIN32)
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

void cleanup_socket() {
#if defined(WIN32)
    WSACleanup();
#endif
}