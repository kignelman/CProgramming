//
// Created by balbe on 28/05/2017.
//

#ifndef GAME_SOCKET_UTIL_H
#define GAME_SOCKET_UTIL_H
#define MAX_CLIENTS 20
#define MAP_WIDTH 20
#define MAP_HEIGHT 16
#define SERVER_PORT 4242
#define MAP_PATH 1
#define GP_UPDATE 10
#define GP_MOVE 2
#define GP_BOMB 3
#define GP_GAME_OVER 4
#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4
#define MAP_FIRE 3
#define MAP_BRICK 4
#define MAP_STEEL 5
#define MAP_BOMB 6

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(linux)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
#endif

typedef struct server_request_s {
    int alive;
    int protocol;
    int position[2];
    int map[MAP_HEIGHT][MAP_WIDTH][2];
    int clients[MAP_HEIGHT][MAP_WIDTH][2];
} server_request_t;

typedef struct client_request_s {
    int protocol;
    int value;
} client_request_t;

struct sockaddr_in create_sockaddr_in(char * hostname, int port);

/**
 * Creation du client.
 *
 * @param hostname
 * @param port
 * @return
 */
SOCKET create_client(char *hostname, int port);

/**
 *
 * reception d'une requete serveur
 *
 * @param socket
 * @param request
 * @return
 */
int recv_server_request(SOCKET socket, server_request_t *request);


/**
 *
 * reception d'une requete client
 *
 * @param socket
 * @param request
 * @return
 */
int recv_client_request(SOCKET socket, client_request_t *request);

/**
 *
 * envoie d'une requete serveur
 *
 * @param socket
 * @param request
 * @return
 */
int send_server_request(SOCKET socket, server_request_t *request);


/**
 *
 * envoie d'une requete client
 *
 * @param socket
 * @param request
 * @return
 */
int send_client_request(SOCKET socket, client_request_t *request);

/**
 * Serialize une requete serveur dans une chaine.
 *
 * @param request
 * @param data
 */
void serialize_server_request(server_request_t *request, char data[1024]);

/**
 *
 * deserialize une chaine en requete serveur.
 *
 * @param request
 * @param data
 */
void deserialize_server_request(char data[1024], server_request_t *request);

/**
 * Serialize une requete client dans une chaine.
 *
 * @param request
 * @param data
 */
void serialize_client_request(client_request_t *request, char data[4]);

/**
 *
 * deserialize une chaine en requete client.
 *
 * @param request
 * @param data
 */
void deserialize_client_request(char data[4], client_request_t *request);

/**
 * Creation d'un serveur.
 *
 * @param hostname
 * @param port
 * @return
 */
SOCKET create_server(char *hostname, int port);

/**
 *
 */
void start_socket();

/**
 *
 */
void cleanup_socket();

#endif //GAME_SOCKET_UTIL_H
